#ifndef CGIFLASH_H
#define CGIFLASH_H

#define MAX_BIN_BUFFER 1024

#include "httpd.h"

int cgiReadFlash(HttpdConnData *connData);
int cgiUploadRaw(HttpdConnData *connData);
int cgiUpdateWeb(HttpdConnData *connData);
int cgiUpgradeRaw(HttpdConnData *connData);
int cgiGetAppVer(HttpdConnData *connData);

#endif