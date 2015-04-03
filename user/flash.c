#include "flash.h"
#include "osapi.h"

uint8_t flashing_flag = 0;

#ifdef OTA
Partition partition[6]={
	{PARTITION_0, PARTITION_SIZE_0},
	{PARTITION_1, PARTITION_SIZE_1},
	{PARTITION_2, PARTITION_SIZE_2},
	{PARTITION_3, PARTITION_SIZE_3},
	{PARTITION_4, PARTITION_SIZE_4},
	{PARTITION_5, PARTITION_SIZE_5}
};
#else
Partition partition[4]={
	{PARTITION_0, PARTITION_SIZE_0},
	{PARTITION_1, PARTITION_SIZE_1},
	{PARTITION_2, PARTITION_SIZE_2},
	{PARTITION_3, PARTITION_SIZE_3}
};
#endif

void ICACHE_FLASH_ATTR erase_partition(int part){
	int flashOff = partition[part].offset;
	int flashSize = partition[part].size;
	int x;
	os_printf("Erasing partition %d at 0x%x...\n", part, flashOff);
	// Which segment are we flashing?	
	for (x=0; x<flashSize; x+=SPI_FLASH_SEC_SIZE){
		spi_flash_erase_sector((flashOff+x)/0x1000);
	}
	os_printf("Done erasing.\n");
}

void ICACHE_FLASH_ATTR erase_block(int address){
	if(address % SPI_FLASH_SEC_SIZE == 0){
		// We need to erase this block
		os_printf("Erasing flash at 0x%x\n", address/SPI_FLASH_SEC_SIZE);
		spi_flash_erase_sector(address/SPI_FLASH_SEC_SIZE);
	}
}

int flashSize=0;

void reset_flash(){
	flashSize=0;
}

int ICACHE_FLASH_ATTR flash_binary(char *data, int size, int part){
	SpiFlashOpResult ret;
	
	//If this is the first time, erase the flash sector
	/*if (flashSize == 0){
		erase_partition(part);
	}*/
	erase_block(partition[part].offset + flashSize);
	
	// The source should be 4byte aligned, so go ahead an flash whatever we have
	ret=spi_flash_write((partition[part].offset + flashSize), (uint32 *)data, size);
	if (ret==SPI_FLASH_RESULT_OK){
		flashSize+=size;
		return size;
	}else{
		os_printf("\nSPI_RET_CODE: %d\n", ret);
		return 0;
	}
}

// Return the updatable partition.  If non OTA, the only updatable partition is 1 (webroot) if OTA, it depends on which partition we are currently booted into.
int get_updatable_partition()
{
#ifndef OTA
	return 1;
#endif
	if (system_upgrade_userbin_check() == 0){
		return 4;
	}else{
		return 1;
	}
}
	