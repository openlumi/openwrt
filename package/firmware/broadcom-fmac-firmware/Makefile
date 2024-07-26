# SPDX-License-Identifier: GPL-2.0-or-later

include $(TOPDIR)/rules.mk

PKG_NAME:=broadcom-fmac-firmware

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)
PKG_VERSION:=20170825
PKG_RELEASE:=1

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

PKG_MAINTAINER:=Ivan Belokobylskiy <belokobylskij@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/broadcom-fmac-default
  SECTION:=firmware
  CATEGORY:=Firmware
endef

define Build/Compile
	true
endef

# BCM43455
define Package/broadcom-43455-fmac
  $(Package/broadcom-fmac-default)
  TITLE:=BCM43455 Firmware
endef

define Package/broadcom-43455-fmac/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		./files/brcmfmac43455-sdio.geniatech,gtw360.bin \
		./files/brcmfmac43455-sdio.geniatech,gtw360.clm_blob \
		./files/brcmfmac43455-sdio.txt \
		$(1)/lib/firmware/brcm/
endef

$(eval $(call BuildPackage,broadcom-43455-fmac))
