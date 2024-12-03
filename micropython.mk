CMOD_DIR := $(USERMOD_DIR)/../pydisplay_cmods

SRC_USERMOD_C += $(CMOD_DIR)/src/byteswap/byteswap.c
CFLAGS_USERMOD += -I$(CMOD_DIR)