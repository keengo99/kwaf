#include "kforwin32.h"
#include "ksapi.h"
#include <sstream>
#include "KWafConfig.h"
#include "KChallengeSession.h"

kgl_dso_version *server_support = NULL;
void register_access(kgl_dso_version *ver);
int waf_flush_fiber(void* arg, int got)
{
	for (;;) {
		server_support->fiber->msleep(5000);
		anticc_session_flush(time(NULL));
	}
	return 0;
}
static char *waf_rand_password(int len)
{
	std::stringstream s;
	const char *base_password = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	int base_len = (int)strlen(base_password);
	if (len < 8) {
		len = 8;
	}
	for (int i = 0;i < len;i++) {
		s << base_password[rand() % base_len];
	}
	return strdup(s.str().c_str());
}
DLL_PUBLIC BOOL  kgl_dso_init(kgl_dso_version *ver)
{
	if (!IS_KSAPI_VERSION_COMPATIBLE(ver->api_version)) {
		return FALSE;
	}
	server_support = ver;
	init_anticc_session();
	char buf[255];
	DWORD buf_size = sizeof(buf);
	waf_config.cc_salt_len = sizeof(waf_config.cc_salt) - 1;
	if (ver->f->get_variable(ver->cn, KGL_GCONFIG, "salt", buf, &buf_size) == KGL_OK) {
		waf_config.set_cc_salt(buf);
	} else {
		char *rand_salt = waf_rand_password(15);
		waf_config.set_cc_salt(rand_salt);
		free(rand_salt);
	}
	buf_size = sizeof(buf);
	waf_config.cc_key_len = sizeof(waf_config.cc_key) - 1;
	if (ver->f->get_variable(ver->cn, KGL_GCONFIG, "key", buf, &buf_size) == KGL_OK) {
		waf_config.set_cc_key(buf);
	} else {
		waf_config.set_cc_key("__WAF");
	}
	ver->api_version = KSAPI_VERSION;
	ver->module_version = MAKELONG(0, 1);	
	ver->fiber->create2(ver->f->get_perfect_selector(), waf_flush_fiber, NULL, 0, 0, NULL);
	register_access(ver);
	return TRUE;
}
DLL_PUBLIC BOOL  kgl_dso_finit(DWORD flag)
{
	return TRUE;
}
