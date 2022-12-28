#include "KWafSession.h"
#include "KChallengeSession.h"
uint32_t KWafSession::process(KREQUEST r, kgl_access_context *ctx, DWORD notify)
{
	char param[512];
	DWORD param_len = sizeof(param) - 1;
	if (KGL_OK != ctx->f->get_variable(r, KGL_VAR_QUERY_STRING, NULL, param, &param_len)) {
		return KF_STATUS_REQ_FALSE;
	}
	char *op = strstr(param, "k=");
	if (op == NULL) {
		return KF_STATUS_REQ_FALSE;
	}
	int key = atoi(op + 2);
	char *v = strstr(param, "v=");
	if (v == NULL) {
		//get security number
		int number = anticc_session_number(key);
		if (number == -1) {
			return KF_STATUS_REQ_FALSE;
		}
		memset(param, 0, sizeof(param));
		snprintf(param, 16, "v=%d", number);
		ctx->f->support_function(r, ctx->cn, KD_REQ_REWRITE_PARAM, param, NULL);
		return KF_STATUS_REQ_TRUE;
	}
	int val = atoi(v + 2);
	int secury_num = -1;
	char *url = anticc_session_verify(key, secury_num);
	if (url == NULL) {
		return KF_STATUS_REQ_FALSE;
	}
	char *p = strchr(url, '?');
	if (p) {
		if (secury_num == val) {
			ctx->f->support_function(r, ctx->cn, KD_REQ_REWRITE_URL, url, NULL);
		}
	}
	free(url);
	return KF_STATUS_REQ_FALSE;
}
