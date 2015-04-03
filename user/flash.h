#ifndef FLASH_H
#define FLASH_H

#ifdef OTA
#define PARTITION_0 0x00000 // BOOTLOADER
#define PARTITION_SIZE_0 0x01000 //4KB
#define PARTITION_1 0x01000 //USER APP 1
#define PARTITION_SIZE_1 0x3B000 // 236KB
#define PARTITION_2 0x3C000 //USER PARAMS
#define PARTITION_SIZE_2 0x04000 // 16KB
#define PARTITION_3 0x40000 //RESERVED
#define PARTITION_SIZE_3 0x01000 //4KB
#define PARTITION_4 0x41000 //USER APP2
#define PARTITION_SIZE_4 0x3B000 // 236KB
#define PARTITION_5 0x7C000 //SYSTEM PARAMS
#define PARTITION_SIZE_5 0x04000 // 16KB
#else
#define PARTITION_0 0x00000 // ICACHE - ROM part 1
#define PARTITION_SIZE_0 0x12000 //72KB
#define PARTITION_1 0x12000 // Website storage
#define PARTITION_SIZE_1 0x20000 // 128KB - Hangs if more than that
#define PARTITION_2 0x40000 // IROM
#define PARTITION_SIZE_2 0x3C000 // 240K 
#define PARTITION_3 0x7C000 //SYSTEM PARAMS AND CONFIGS
#define PARTITION_SIZE_3 0x04000 // 16KB
#endif

#include <c_types.h>
#include <ip_addr.h>
#include <espconn.h>
#include <spi_flash.h>

typedef struct Partition Partition;
struct Partition {
	int offset;
	int size;
};

extern Partition partition[];


void ICACHE_FLASH_ATTR erase_partition(int part);
void ICACHE_FLASH_ATTR erase_block(int address);
int ICACHE_FLASH_ATTR flash_binary(char *data, int size, int part);
int ICACHE_FLASH_ATTR get_web_partition();
void reset_flash();

#endif