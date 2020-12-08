#=====================================
# OpenWrt Makefile
#=====================================

include $(TOPDIR)/rules.mk

PKG_NAME:=memtool
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

CC = gcc
CFLAGS = -Wall
LDFLAGS = 

include $(INCLUDE_DIR)/package.mk


define Package/memtool
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Memtool utility to flash zigbee chip
endef

define Package/memtool/description
	Utility to flash zigbee chip on Xioami Lumi gateway
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

# Package build instructions; invoke the target-specific compiler to first compile the source file, and then to link the file into the final executable
define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/memtool.o -c $(PKG_BUILD_DIR)/memtool.c 
	$(TARGET_CC) $(TARGET_LDFLAGS) -o $(PKG_BUILD_DIR)/$1 $(PKG_BUILD_DIR)/memtool.o
endef

define Package/memtool/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/memtool $(1)/usr/bin/
endef

$(eval $(call BuildPackage,memtool))