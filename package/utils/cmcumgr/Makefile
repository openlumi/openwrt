include $(TOPDIR)/rules.mk

PKG_NAME:=cmcumgr
PKG_VERSION:=0.2.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/LeBlue/cmcumgr-client.git
PKG_SOURCE_DATE:=2025-11-11
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_MIRROR_HASH:=03d0bfa032f32d0ba302e39bcb8aec4c5569160fed55549f6796d71cce0b808f

PKG_BUILD_DEPENDS:=meson/host ninja/host
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/meson.mk

define Package/cmcumgr
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=MCUmgr client for Zephyr and Mynewt
  URL:=https://github.com/LeBlue/cmcumgr-client
endef

define Package/cmcumgr/description
  A command-line client for communicating with devices using the Simple
  Management Protocol (SMP) over serial, compatible with Zephyr and
  Apache Mynewt bootloaders.
endef

TARGET_CFLAGS += -U_FORTIFY_SOURCE

define Package/cmcumgr/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cmcumgr $(1)/usr/bin/cmcumgr
	$(INSTALL_BIN) ./files/smpflash $(1)/usr/bin/smpflash
endef

$(eval $(call BuildPackage,cmcumgr))
