#=====================================
# OpenWrt Makefile
#=====================================

include $(TOPDIR)/rules.mk

PKG_NAME:=jnflasher
PKG_VERSION:=0.11
PKG_RELEASE:=1

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

CC = gcc
CFLAGS = -Wall
LDFLAGS = 

include $(INCLUDE_DIR)/package.mk


define Package/jnflasher
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=jnflasher
endef

define Package/jnflasher/description
	jnflasher - a tool to flash jn5169 zigbee chip
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

define Package/jnflasher/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/jennicProgrammer/iot_jp $(1)/usr/bin/
	$(LN) iot_jp $(1)/usr/bin/jnflasher
endef

$(eval $(call BuildPackage,jnflasher))