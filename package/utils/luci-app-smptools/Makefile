include $(TOPDIR)/rules.mk

LUCI_TITLE:=NRF52840 zigbee tools LuCI interface
LUCI_DEPENDS:=+cmcumgr +libustream-mbedtls
LUCI_PKGARCH:=all
LUCI_DESCRIPTION:=Provides a webadmin for cmcumgr.

PKG_MAINTAINER:= Ivan Belokobylskiy <belokobylskij@gmail.com>
PKG_LICENSE:=Apache-2.0

include ../../../feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
