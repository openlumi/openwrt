#=====================================
# OpenWrt Makefile
#=====================================

include $(TOPDIR)/rules.mk

PKG_NAME:=rtk_hciattach
PKG_VERSION:=20201020
PKG_RELEASE:=1

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

CC = gcc
CFLAGS = -Wall
LDFLAGS = 

include $(INCLUDE_DIR)/package.mk


define Package/rtk_hciattach
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=rtl8723bs_bt
endef

define Package/rtk_hciattach/description
	rtl8723bs_bt
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

# Package build instructions; invoke the target-specific compiler to first compile the source file, and then to link the file into the final executable
define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/hciattach_rtk.o -c $(PKG_BUILD_DIR)/hciattach_rtk.c 
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/hciattach.o -c $(PKG_BUILD_DIR)/hciattach.c 
	$(TARGET_CC) $(TARGET_LDFLAGS) -o $(PKG_BUILD_DIR)/$1 $(PKG_BUILD_DIR)/hciattach_rtk.o $(PKG_BUILD_DIR)/hciattach.o
endef

define Package/rtk_hciattach/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rtk_hciattach $(1)/bin/
	$(INSTALL_DIR) $(1)/lib/firmware/rtl_bt/
	$(CP) $(PKG_BUILD_DIR)/rtlbt_fw $(1)/lib/firmware/rtl_bt/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/rtk_hciattach.init $(1)/etc/init.d/rtk_hciattach
endef

$(eval $(call BuildPackage,rtk_hciattach))