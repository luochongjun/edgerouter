#
#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=edgerouter
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/edgerouter
  SECTION:=base
  CATEGORY:=gl-inet
  TITLE:=GL iNet edgerouter mode
  DEPENDS:=+luci-compat
endef

define Package/edgerouter/description
Openvpn server endpoint api for gl-inet.
endef

ifdef CONFIG_TARGET_ipq806x
ifeq ($(CONFIG_GCC_VERSION_4_8),y)
  TARGET_CFLAGS += -std=gnu99
endif
MAKE_FLAGS += \
		CFLAGS+="$(TARGET_CFLAGS) -Wall -DIPQ40XX"
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(call Build/Compile/Default)
endef


define Package/edgerouter/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lancover  $(1)/usr/bin

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/edgerouter.init  $(1)/etc/init.d/edgerouter
	$(INSTALL_BIN) ./files/edgerouter.fw  $(1)/etc/

	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/18-edgerouter  $(1)/etc/hotplug.d/iface

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/edgerouter.config  $(1)/etc/config/edgerouter

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/edgerouter
	$(INSTALL_DATA) ./files/cbi_tab.lua  $(1)/usr/lib/lua/luci/model/cbi/edgerouter

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller/admin
	$(INSTALL_DATA) ./files/edgerouter.lua  $(1)/usr/lib/lua/luci/controller/admin

	#$(INSTALL_DIR) $(1)/usr/lib/gl
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/libedgerouterapi.so $(1)/usr/lib/gl
	#$(LN) /usr/lib/gl/libedgerouterapi.so $(1)/usr/lib/
endef

define Package/edgerouter/postinst
        #!/bin/sh
	uci add ucitrack edgerouter
	uci set ucitrack.@edgerouter[0]=edgerouter
	uci set ucitrack.@edgerouter[0].exec="/bin/sh /etc/rc.common /etc/init.d/edgerouter restart"
	uci commit ucitrack
	/etc/init.d/ucitrack restart

        exit 0
endef


$(eval $(call BuildPackage,edgerouter))
