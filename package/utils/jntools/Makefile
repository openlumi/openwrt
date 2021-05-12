#=====================================
# OpenWrt Makefile
#=====================================

include $(TOPDIR)/rules.mk

PKG_NAME:=jntools
PKG_VERSION:=0.11
PKG_RELEASE:=2

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

CC = gcc
CFLAGS = -Wall
LDFLAGS =

include $(INCLUDE_DIR)/package.mk


define Package/jntools
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=jntools
	DEPENDS:=+memtool
endef

define Package/jntools/description
	jntools - a set of tools to flash and manage jn5169 zigbee chip
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

# Package build instructions; invoke the target-specific compiler to first compile the source file, and then to link the file into the final executable
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	        CC="$(TARGET_CC)" \
	        CFLAGS="$(TARGET_CFLAGS) -Wall" \
	        LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/jntools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/jennicProgrammer/iot_jp $(1)/usr/bin/
	$(LN) iot_jp $(1)/usr/bin/jnprogrammer
	$(INSTALL_BIN) ./files/jnflash $(1)/usr/bin/
	$(INSTALL_BIN) ./files/jntool $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/iot/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/jennicProgrammer/FlashProgrammerExtension_*.bin $(1)/usr/share/iot/
endef

$(eval $(call BuildPackage,jntools))
