MOD_DIR := $(USERMOD_DIR)
PYDISPLAY_CMODS_DIR = $(MOD_DIR)/../pydisplay_cmods

SRC_USERMOD_C += $(PYDISPLAY_CMODS_DIR)/src/byteswap/byteswap.c
# CFLAGS_USERMOD += -I$(CMOD_DIR)