# tnx to mamalala
# Changelog
# Changed the variables to include the header file directory
# Added global var for the XTENSA tool root
#
# This make file still needs some work.
#
#
# Output directors to store intermediate compiled files
# relative to the project directory
BUILD_BASE	= build
FW_BASE		= firmware

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= /opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin

#Extra Tensilica includes from the ESS VM
SDK_EXTRA_INCLUDES ?= /opt/Espressif/include
SDK_EXTRA_LIBS ?= /opt/Espressif/arch/lib

# base directory of the ESP8266 SDK package, absolute
SDK_ROOT	?= /opt/Espressif/ESP8266_SDK/
SDK_VERSION	?= 0.9.3
SDK_BASE	?= $(SDK_ROOT)esp_iot_sdk_v$(SDK_VERSION)/

# Hardware info
ESP_FLASH_SIZE	?= 512
ESP_BOOT_VER	?= old

#Esptool.py path and port
ESPTOOL		?= esptool
FW_TOOL		?= esptool
JOIN_TOOL	?= tools/gen_flashbin.py
ESPTOOLPY	?= esptool/esptool.py
ESPPORT		?= /dev/ttyS1
#ESPDELAY indicates seconds to wait between flashing the two binary images
ESPDELAY	?= 3
ESPBAUD		?= 115200
# name for the target project
TARGET		= httpd
BLANK		= blank.bin
BOOTLOADER	= boot_v1.1.bin
ifeq ("$(SDK_VERSION)","0.9.3")
ESP_BOOT_VER	= old
endif
ifeq ("$(ESP_BOOT_VER)","new")
ifeq ("$(SDK_VERSION)","0.9.5")
BOOTLOADER      = boot_v1.2.bin
endif
ifeq ("$(SDK_VERSION)","0.9.6")
BOOTLOADER      = boot_v1.3(b3).bin
endif
endif


# which modules (subdirectories) of the project to include in compiling
#MODULES		= driver user lwip/api lwip/app lwip/core lwip/core/ipv4 lwip/netif
MODULES		= driver user
EXTRA_INCDIR	= include \
		. \
		lib/heatshrink/ \
		$(SDK_EXTRA_INCLUDES)

# libraries used in this project, mainly provided by the SDK
LIBS		= c gcc hal phy pp net80211 wpa main lwip upgrade

# compiler flags using during compilation of source files
CFLAGS		= -Os -ggdb -std=c99 -Wpointer-arith -Wundef -Wall -Wl,-EL -fno-inline-functions \
		-nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH \
		-Wno-address

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static -L$(SDK_EXTRA_LIBS)

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld
ifeq ("$(SDK_VERSION)","0.9.3")
LD_SCRIPT1      = eagle.app.v6.app1.ld
LD_SCRIPT2      = eagle.app.v6.app2.ld
else
LD_SCRIPT1      = eagle.app.v6.$(ESP_BOOT_VER).$(ESP_FLASH_SIZE).app1.ld
LD_SCRIPT2      = eagle.app.v6.$(ESP_BOOT_VER).$(ESP_FLASH_SIZE).app2.ld
BOOTLOADER	= 
endif
# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json
SDK_BIN		= bin

# we create two different files for uploading into the flash
# these are the names and options to generate them
FW_FILE_1	= 0x00000
FW_FILE_1_ARGS	= -bo $@ -bs .text -bs .data -bs .rodata -bc -ec
FW_FILE_2	= 0x40000
FW_FILE_2_ARGS	= -es .irom0.text $@ -ec
FW_FILE_3	= user1
FW_FILE_4	= user2

#Intermediate files for User1.bin and User2.bin
FW_INTF_1	= 0x01000
FW_INTF_1_ARGS	= -bo $@ -bs .text -bs .data -bs .rodata -bc -ec

FW_INTF_2	= 0x11000
FW_INTF_2_ARGS	= -es .irom0.text $@ -ec

FW_INTF_3	= 0x41000
FW_INTF_3_ARGS	= -bo $@ -bs .text -bs .data -bs .rodata -bc -ec

FW_INTF_4	= 0x51000
FW_INTF_4_ARGS	= -es .irom0.text $@ -ec


# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc



####
#### no user configurable options below here
####
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))
SDK_BIN		:= $(addprefix $(SDK_BASE)/,$(SDK_BIN))
SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)
BOOTLOADER	:= $(addprefix $(SDK_BIN)/,$(BOOTLOADER))
BLANK		:= $(addprefix $(SDK_BIN)/,$(BLANK))

JOIN_TOOL	:= $(addprefix $(SDK_BASE)/,$(JOIN_TOOL))
ESPTOOLPY	:= $(addprefix $(SDK_ROOT)/,$(ESPTOOLPY))

LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))
LD_SCRIPT1	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT1))
LD_SCRIPT2	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT2))

INCDIR		:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

FW_FILE_1	:= $(addprefix $(FW_BASE)/,$(FW_FILE_1).bin)
FW_FILE_2	:= $(addprefix $(FW_BASE)/,$(FW_FILE_2).bin)
FW_FILE_3	:= $(addprefix $(FW_BASE)/,$(FW_FILE_3).bin)
FW_FILE_4	:= $(addprefix $(FW_BASE)/,$(FW_FILE_4).bin)

FW_INTF_1	:= $(addprefix $(BUILD_BASE)/,$(FW_INTF_1).bin)
FW_INTF_2	:= $(addprefix $(BUILD_BASE)/,$(FW_INTF_2).bin)
FW_INTF_3	:= $(addprefix $(BUILD_BASE)/,$(FW_INTF_3).bin)
FW_INTF_4	:= $(addprefix $(BUILD_BASE)/,$(FW_INTF_4).bin)


V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)

ifdef DEBUG
  CFLAGS := $(CFLAGS) -DDEBUG_VERSION=$(DEBUG)
endif

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(vecho) "$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS) -c $$< -o $$@"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2) $(FW_FILE_3) $(FW_FILE_4)

website: webpages.espfs htmlflash 

$(FW_FILE_1): $(TARGET_OUT)
	$(vecho) "FW $@"
	$(Q) $(ESPTOOL) -eo $(TARGET_OUT) $(FW_FILE_1_ARGS)

$(FW_FILE_2): $(TARGET_OUT)
	$(vecho) "FW $@"
	$(Q) $(ESPTOOL) -eo $(TARGET_OUT) $(FW_FILE_2_ARGS)

$(FW_FILE_3): $(FW_INTF_1) $(FW_INTF_2)
	@echo "FW $@"
	@$(JOIN_TOOL) $(FW_INTF_1) $(FW_INTF_2)
	@mv eagle.app.flash.bin $@

$(FW_FILE_4): $(FW_INTF_3) $(FW_INTF_4)
	@echo "FW $@"
	@$(JOIN_TOOL) $(FW_INTF_3) $(FW_INTF_4)
	@mv eagle.app.flash.bin $@



#Intermediate bin files for User1 and User2

$(FW_INTF_1): $(TARGET_OUT)
	@echo "FW $@"
	@$(FW_TOOL) -eo $(TARGET_OUT)1 $(FW_INTF_1_ARGS)

$(FW_INTF_2): $(TARGET_OUT)
	@echo "FW $@"
	@$(FW_TOOL) -eo $(TARGET_OUT)1 $(FW_INTF_2_ARGS)

$(FW_INTF_3): $(TARGET_OUT)
	@echo "FW $@"
	@$(FW_TOOL) -eo $(TARGET_OUT)2 $(FW_INTF_3_ARGS)

$(FW_INTF_4): $(TARGET_OUT)
	@echo "FW $@"
	@$(FW_TOOL) -eo $(TARGET_OUT)2 $(FW_INTF_4_ARGS)



$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT1) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@1
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT2) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@2


$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

firmware:
	$(Q) mkdir -p $@

flash: $(FW_FILE_1) $(FW_FILE_2)
	echo "Running..."
	$(ESPTOOLPY) --port $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin 0x40000 firmware/0x40000.bin
	#$(Q) $(ESPTOOL) -cp $(ESPPORT) -cb $(ESPBAUD) -ca 0x00000 -cf firmware/0x00000.bin -v
	#$(Q) [ $(ESPDELAY) -ne 0 ] && echo "Please put the ESP in bootloader mode..." || true
	#$(Q) sleep $(ESPDELAY) || true
	#$(Q) $(ESPTOOL) -cp $(ESPPORT) -cb $(ESPBAUD) -ca 0x40000 -cf firmware/0x40000.bin -v

cloud: $(FW_FILE_3) $(FW_FILE_4) webpages.espfs
	echo "Running..."
	$(ESPTOOLPY) --port $(ESPPORT) write_flash 0x00000 $(BOOTLOADER) 0x01000 firmware/user1.bin 0x41000 webpages.espfs 

webpages.espfs: html/ html/wifi/ mkespfsimage/mkespfsimage
	cd html; find | ../mkespfsimage/mkespfsimage > ../webpages.espfs; cd ..

mkespfsimage/mkespfsimage: mkespfsimage/
	make -C mkespfsimage

htmlflash: webpages.espfs
	$(Q) if [ $$(stat -c '%s' webpages.espfs) -gt $$(( 0x2E000 )) ]; then echo "webpages.espfs too big!"; false; fi
	$(ESPTOOLPY) --port $(ESPPORT) write_flash 0x12000 webpages.espfs
	#$(ESPTOOL) -cp $(ESPPORT) -cb $(ESPBAUD) -ca 0x12000 -cf webpages.espfs -v

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) find $(BUILD_BASE) -type f | xargs rm -f


	$(Q) rm -f $(FW_FILE_1)
	$(Q) rm -f $(FW_FILE_2)
	$(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
