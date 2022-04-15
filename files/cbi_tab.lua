m = Map("edgerouter", translate("旁路由模式"), translate("使用一键旁路由模式请将设备的WAN口连接到上游路由器的LAN口，勾选启用并保存应用")) -- cbi_file is the config file in /etc/config
d = m:section(TypedSection, "global", translate("全局设置"))  -- info is the section called info in cbi_file
a = d:option(Flag, "enabled", translate("启用")); a.optional=false; a.rmempty = false;  -- name is the option in the cbi_file
return m
