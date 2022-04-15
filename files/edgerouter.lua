module("luci.controller.admin.edgerouter", package.seeall) 
 function index()
     entry({"admin", "network", "edgerouter"}, cbi("edgerouter/cbi_tab"), translate("一键旁路由"), 1)  
  end
