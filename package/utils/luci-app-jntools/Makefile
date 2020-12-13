include $(TOPDIR)/rules.mk

LUCI_TITLE:=JN516X zigbee tools LuCI interface
LUCI_DEPENDS:=+jntools +libustream-mbedtls
LUCI_PKGARCH:=all
LUCI_DESCRIPTION:=Provides a webadmin for jntools.

PKG_MAINTAINER:= Ivan Belokobylskiy <belokobylskij@gmail.com>
PKG_LICENSE:=Apache-2.0

include ../../../feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
