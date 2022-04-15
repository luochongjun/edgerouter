#include <stdio.h>
#include <string.h>
#include <gl/gjson.h>
#include <gl/shell.h>
#include <gl/router.h>
#include <gl/cmm.h>
#include <gl/debug.h>
#include <sys/types.h>
#include <dirent.h>
#include <gl/err_code.h>
#include <gl/guci2.h>

#define APPNMAE "SideRouter"

char *get_package_name(void)
{
	return APPNMAE;
}

/***
 * @api {post} /siderouter/set_config /siderouter/set_config
 * @apiGroup siderouter
 * @apiVersion 1.0.0
 * @apiDescription Remove the software.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int set_config(json_object* input,json_object* output)
{
	bool enable = gjson_get_boolean(input, "enabled");

	struct uci_context* ctx = guci2_init();
	if(enable){
		guci2_set(ctx, "siderouter.global.enabled", "1");
		guci2_commit(ctx, "siderouter");
		guci2_free(ctx);
		execCommand("/etc/init.d/siderouter start");
	}
	else
	{
		guci2_set(ctx, "siderouter.global.enabled", "0");
		guci2_commit(ctx, "siderouter");
		guci2_free(ctx);
		execCommand("/etc/init.d/siderouter stop");
	}
	
	
	return 0;

}

/***
 * @api {get} /siderouter/get_config /siderouter/get_config
 * @apiGroup siderouter
 * @apiVersion 1.0.0
 * @apiDescription Get the list of user installed software.
 * @apiHeader {string} Authorization Users unique token.
 * @apiSuccess {integer} code return code.
 * @apiSuccess (Code) {integer} 0 success.
 * @apiSuccess (Code) {integer} -1 Invalid user, permission denied or not logged in!
 */
int get_config(json_object* input,json_object* output)
{
	char enable[3]={0};

	struct uci_context* ctx = guci2_init();	
	guci2_get(ctx, "siderouter.global.enabled", enable );
	guci2_free(ctx);

	if(strcmp(enable,"1")==0){
		gjson_add_boolean(output, "enabled",1);
	}
	else
	{
		gjson_add_boolean(output, "enabled",0);
	}

	return 0;
}

/** The implementation of the GetAPIFunctions function **/
#include <gl/glapibase.h>

static api_info_t gl_lstCgiApiFuctionInfo[]={
		map("/siderouter/set_config","post",set_config),
		map("/siderouter/get_config","get",get_config),
};

api_info_t* get_api_entity(int* pLen)
{
	(*pLen)=sizeof(gl_lstCgiApiFuctionInfo)/sizeof(gl_lstCgiApiFuctionInfo[0]);
	return gl_lstCgiApiFuctionInfo;
}
