# SPDX-License-Identifier: GPL-2.0-or-later

include $(TOPDIR)/rules.mk

PKG_NAME:=broadcom-hci-firmware

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)
PKG_VERSION:=20240723
PKG_RELEASE:=1

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

PKG_MAINTAINER:=Ivan Belokobylskiy <belokobylskij@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/broadcom-hci-default
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Build/Compile
	true
endef

define Package/broadcom-4345C0-hci
  $(Package/broadcom-hci-default)
  TITLE:=BCM HCI Firmware
endef

define Package/broadcom-4345C0-hci/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		./files/BCM4345C0.hcd \
		$(1)/lib/firmware/brcm/
endef

$(eval $(call BuildPackage,broadcom-4345C0-hci))
