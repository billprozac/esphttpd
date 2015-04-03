/*
Some flash handling cgi routines. Used for reading the existing flash and updating the ESPFS image.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd.h"
#include "cgiflash.h"
#include "io.h"
#include <ip_addr.h>
#include "espmissingincludes.h"
#include "../include/httpdconfig.h"
#include "upgrade.h"


LOCAL os_timer_t flash_reboot_timer;

//Cgi that reads the SPI flash. Assumes 512KByte flash.
int ICACHE_FLASH_ATTR cgiReadFlash(HttpdConnData *connData) {
	int *pos=(int *)&connData->cgiData;
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	if (*pos==0) {
		os_printf("Start flash download.\n");
		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", "application/bin");
		httpdEndHeaders(connData);
		*pos=0x40200000;
		return HTTPD_CGI_MORE;
	}
	//Send 1K of flash per call. We will get called again if we haven't sent 512K yet.
	espconn_sent(connData->conn, (uint8 *)(*pos), 1024);
	*pos+=1024;
	if (*pos>=0x40200000+(512*1024)) return HTTPD_CGI_DONE; else return HTTPD_CGI_MORE;
}

//Cgi that allows the ESPFS image to be replaced via http POST
int ICACHE_FLASH_ATTR cgiUploadRaw(HttpdConnData *connData) {
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	int part = get_updatable_partition();
	if (connData->post->processed == 0 && connData->getArgs!=0){
		// First run, init the partition and prep if necessary
		part = connData->getArgs[0] - '0'; // Turn for arg (should be partition ID) into partition index
		os_printf("\nFlashing partition: %d\n", part);
	}
	
	SpiFlashOpResult ret;
	// The source should be 4byte aligned, so go ahead an flash whatever we have
	ret=flash_binary(connData->post->buff, connData->post->buffLen, part);
	if (ret != connData->post->buffLen){
		os_printf("Error writing to the flash\n");
		return HTTPD_CGI_DONE;
	}	
	// Count bytes for data
	connData->post->processed += ret;

	os_printf("Wrote %d bytes (%dB of %d)\n", connData->post->buffSize, connData->post->processed, connData->post->len);//&connData->postBuff));

	if (connData->post->processed == connData->post->len){
		httpdSend(connData, "Finished uploading", -1);
		reset_flash();
		return HTTPD_CGI_DONE;
	} else {
		return HTTPD_CGI_MORE;
	}
}

int ICACHE_FLASH_ATTR cgiUpgradeRaw(HttpdConnData *connData) {
	int ret = cgiUploadRaw(connData);
	if (ret == HTTPD_CGI_DONE){
		system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
		// Schedule a reboot
		os_timer_disarm(&flash_reboot_timer);
		os_timer_setfn(&flash_reboot_timer, (os_timer_func_t *)system_upgrade_reboot, NULL);
		os_timer_arm(&flash_reboot_timer, 2000, 1);
		
	}
	return ret;
}

char uploadForm[2048] = "\
<html>\
  <body>\
  <h1>Upload binary file</h1>\
    <form method=\"POST\" enctype=\"multipart/form-data\">\
      File&nbsp;&nbsp;<t><input type=\"file\" name=\"app\"><br>\
      <input type=\"checkbox\" name=\"reset\" value=\"reset\"> Reset Configuration<br>\
      <input type=\"submit\">\
    </form>\
  </body>\
</html>";

#define LOOKING_FOR_SECTION 1
#define CHECKING_HEADERS 2
#define IN_DATA 3
 
// Because flash must be 4-byte aligned, we need to buffer the data....unfortunately
char binBuffer[MAX_BIN_BUFFER]; 
 
typedef struct updateState_t {
	char delimiter[61];
	int  step;
} updateState_t;

char* bin_strstr(char *haystack, char *needle, int haystackLen, int needleLen){
	if(needleLen < 0){
		needleLen = strlen(needle);
	}
	char * end = haystack + haystackLen;
	for(; haystack < end; haystack++){
		if(*haystack == *needle){
			int match = true;
			for(int i = 1; i< needleLen; i++){
				if(*(needle + i) != *(haystack + i)){
					match = false;
					break;
				}
			}
			if(match){
				return haystack;
			}
		}
	}
	return NULL;
}


//Cgi to allow user to upload a new espfs image -- Mostly from bjpirt -- work in progress
int ICACHE_FLASH_ATTR cgiUpdateWeb(HttpdConnData *connData) {
	os_printf("data size   : %d\r\n", connData->post->buffLen);
	updateState_t *state;
	int part;
	part=get_updatable_partition();

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	if(connData->requestType == HTTPD_METHOD_GET){
		// Check to see if we have a prettier version of this form... vain and I know it!
		int ret = cgiEspFsHook(connData);
		if (ret == HTTPD_CGI_NOTFOUND){
			char var_len[10];
			os_sprintf(var_len, "%d", strlen(uploadForm));
			httpdStartResponse(connData, 200);
			httpdHeader(connData, "Content-Length", var_len);
			httpdEndHeaders(connData);
			httpdSend(connData, uploadForm, strlen(uploadForm));
		} else {
			return ret;
		}
	}else if(connData->requestType == HTTPD_METHOD_POST){
		if(connData->post->received == connData->post->buffLen){
			//it's the first time this handler has been called for this request
			connData->cgiPrivData = (int*)os_malloc(sizeof(updateState_t));
			state = connData->cgiPrivData;
			state->step = LOOKING_FOR_SECTION;
		}else{
			state = connData->cgiPrivData;
		}

		char *b;
		char *p;
		int x = 0;
		int eof=0;
		char * end = connData->post->buff + connData->post->buffLen;
		for(b = connData->post->buff; b < end; b++){
			if(state->step == LOOKING_FOR_SECTION){
				if((p = bin_strstr(b, connData->post->multipartBoundary, connData->post->buffLen - (b - connData->post->buff), -1)) != NULL){
					os_printf("Found section\r\n");
					// We've found one of the sections, now make sure it's the file
					b = p + strlen(connData->post->multipartBoundary) + 2;
					state->step = CHECKING_HEADERS;
				}else{
					os_printf("Not Found section\r\n");
					break;
				}
			}

			if(state->step == CHECKING_HEADERS){
				//os_printf("next data: %s\n", b);
				if(!os_strncmp(b, "Content-Disposition: form-data", 30)){
					os_printf("Correct header\r\n");
					// It's the Content-Disposition header
					// Find the end of the line
					p = os_strstr(b, "\r\n");
					// turn the end of the line into a string end so we can search within the line
					*p = 0;
					if(os_strstr(b, "name=\"file\"") != NULL){
						os_printf("Correct file\r\n");
						b = p + 2;
						// it's the correct section, skip to the data
						if((p = os_strstr(b, "\r\n\r\n")) != NULL){
							os_printf("Skipping to data\r\n");
							b = p + 4;
							state->step = IN_DATA;
						}else{
							os_printf("Couldn't find line endings\r\n");
							os_printf("data: %s\n", b);
							return HTTPD_CGI_DONE;
						}
					}else{
						// it's the wrong section, skip to the next boundary
						b = p + 1;
						state->step = LOOKING_FOR_SECTION;
					}
				}else{
					// Skip to the next header
					p = os_strstr(b, "\r\n");
					b = p + 2;
				}
			}

			if(state->step == IN_DATA){
				//os_printf("In data\r\n");
				// Make sure it doesn't contain the boundary, but we can't rely on there being no zeroes so can't use strstr
				if((p = bin_strstr(b, connData->post->multipartBoundary, connData->post->buffLen - (b - connData->post->buff), -1)) == NULL){
					// all of the data is file data
					binBuffer[x++]=*b;
				}else{
					if(b < (p - 2)){ // -2 bytes for the \r\n
						// This is a byte that's part of the file
						binBuffer[x++]=*b;
					}else{
						eof = 1;
						os_printf("\r\nComplete\r\n");
					}
				}

				if (x >= MAX_BIN_BUFFER || eof) {
					//Send the response.
					flash_binary(binBuffer, x, part);
					x = 0;
				}
			}
		}
		/*
		os_printf("postReceived: %d\r\n", connData->postReceived);
		os_printf("postLen     : %d\r\n", connData->postLen);
		os_printf("data size   : %d\r\n", connData->postBuffLen);
		*/
		if(connData->post->received >= connData->post->len){
			httpdStartResponse(connData, 204);
			if (connData->cgiPrivData!=NULL) os_free(connData->cgiPrivData);
			return HTTPD_CGI_DONE;
		}else{
			return HTTPD_CGI_MORE;
		}
	}
	return HTTPD_CGI_DONE;
}

//Cgi to allow user to upload a new espfs image
int ICACHE_FLASH_ATTR cgiGetAppVer(HttpdConnData *connData) {
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	char var[1];
	os_sprintf(var, "%d", system_upgrade_userbin_check() + 1);
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Length", "1");
	httpdEndHeaders(connData);
	httpdSend(connData, var, 1);
	return HTTPD_CGI_DONE;
}