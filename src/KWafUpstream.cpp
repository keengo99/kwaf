#include "KWafUpstream.h"
#include "kstring.h"
#include "KHttpField.h"
#include "KWafConfig.h"
#include "ksocket.h"
#include "KStringBuf.h"
#include "kmd5.h"
#include "KTargetRate.h"
#include "KSequence.h"
#include "KDsoAutoBuffer.h"
#include <sstream>
static KTargetRate urlTargetRate;
#define KGL_WAF_COOKIE "kgl_cookie"
KWafUpstream::KWafUpstream(KChallenge *challenge)
{
	this->challenge = challenge;
	challenge->AddRef();
}


KWafUpstream::~KWafUpstream()
{
	challenge->Release();
}

static void *create_ctx()
{
	//never goto here
	assert(false);
	return NULL;
}
static void free_ctx(void *ctx)
{
	KWafUpstream *waf_ctx = (KWafUpstream *)ctx;
	delete waf_ctx;
}
static KGL_RESULT check(KREQUEST rq, kgl_async_context *ctx)
{
	KWafUpstream *waf_ctx = (KWafUpstream *)ctx->module;
	return waf_ctx->check(rq, ctx);
}
KGL_RESULT KWafUpstream::check(KREQUEST r,kgl_async_context *ctx)
{
	char md5buf[33];
	char ips[MAXIPLEN];
	DWORD iplen = sizeof(ips) - 1;
	ctx->f->get_variable(r, KGL_VAR_REMOTE_ADDR, NULL, ips, &iplen);
	char path[MAX_PATH];
	DWORD path_len = sizeof(path) - 1;
	ctx->f->get_variable(r, KGL_VAR_PATH_INFO, NULL, path, &path_len);
	char param[255];
	param[0] = '\0';
	DWORD param_len = sizeof(param) - 1;
	KGL_RESULT ret = ctx->f->get_variable(r, KGL_VAR_QUERY_STRING, NULL, param, &param_len);
	char *cc_key_buf = NULL;
	if (ret == KGL_OK) {
		char *split_url = param;
		char *split2_url = NULL;
		char *cc_key = NULL;
		if (strncmp(split_url, waf_config.first_cc_str, waf_config.cc_key_len + 1) == 0) {
			*split_url = '\0';
			split2_url = split_url;
			cc_key = split_url + waf_config.cc_key_len + 1;
			split_url = NULL;
		} else {
			split2_url = strstr(split_url, waf_config.not_first_cc_str);
			if (split2_url) {
				*split2_url = 0;
				cc_key = split2_url + waf_config.cc_key_len + 2;
			}
		}
		if (cc_key) {
			char *hot = cc_key;
			while (*hot) {
				if (*hot == '?' || *hot == '&') {
					if (*hot == '&' && split_url) {
						*split2_url = '&';
						split2_url++;
					}
					*hot = '\0';
					hot++;
					if (cc_key_buf == NULL) {
						cc_key_buf = strdup(cc_key);
					}
					while (*hot) {
						*split2_url = *hot;
						hot++;
						split2_url++;
					}
					*split2_url = '\0';
					break;
				}
				hot++;
			}
			if (cc_key_buf == NULL) {
				cc_key_buf = strdup(cc_key);
			}

			ctx->f->support_function(r, ctx->cn, KD_REQ_REWRITE_PARAM, param, NULL);
			int cc_flag = 0;
			if (CheckCCKey(r, ctx, ips, cc_key_buf, cc_flag)) {
				xfree(cc_key_buf);
				if (!KBIT_TEST(cc_flag, ANTICC_FIX_URL)) {
					return KGL_NEXT;
				}
				//再重定向一次修复url
				ctx->out->f->write_status(ctx->out->ctx, 302);
				std::stringstream  fix_url;
				
				fix_url << path;
				if (*param) {
					fix_url << "?" << param;
				}
				ctx->out->f->write_unknow_header(ctx->out->ctx, kgl_expand_string("Location"), fix_url.str().c_str(), (hlen_t)fix_url.str().size());
				kgl_response_body body;
				auto result = ctx->out->f->write_header_finish(ctx->out->ctx, 0, &body);
				if (result != KGL_OK) {
					return result;
				}
				return body.f->close(body.ctx, KGL_OK);
			}
		}
	}

	if (challenge->wl && ctx->f->support_function(r, ctx->cn, KD_REQ_CHECK_WHITE_LIST, (void *)ips,(void **)&challenge->flush)==KGL_OK) {
		//白名单
		return KGL_NEXT;
	}
	KStringBuf r_url;
	KStringBuf id;
	unsigned seq = sequence.getNext();
	time_t now_time = time(NULL);
	KDsoAutoBuffer new_request(r);
	int cc_flag = 0;
	if (challenge->wl) {
		KBIT_SET(cc_flag, ANTICC_WHITE_LIST);
	}
	if (challenge->fix_url) {
		KBIT_SET(cc_flag, ANTICC_FIX_URL);
	}
	id.add(cc_flag, "%d");
	r_url.add(cc_flag, "%d");
	r_url << ips;
	r_url.write_all(waf_config.cc_salt, waf_config.cc_salt_len);
	r_url << (INT64)now_time << "_" << seq;
	KMD5(r_url.c_str(), r_url.size(), md5buf);
	id.write_all(md5buf, 32);
	id << (INT64)now_time << "_" << seq;
	ctx->out->f->write_status(ctx->out->ctx,  challenge->status_code);
	
	KChallengeHeader *header = challenge->header;

	KChallengeBuffer *buffer = challenge->body;
	KAntiRefreshParserContext cc_ctx;
	KStringBuf new_url;
	new_url << path;
	if (*param) {
		new_url << "?" << param << "&";
	} else {
		new_url << "?";
	}
	new_url << waf_config.cc_key << "=" << id.c_str();
	while (header) {
		if (header->val->next == NULL && header->val->type == CC_BUFFER_STRING) {
			assert(header->val->buf);
			ctx->out->f->write_unknow_header(ctx->out->ctx, header->attr, header->attr_len, header->val->buf, header->val->len);
		} else {
			KDsoAutoBuffer tmp_buffer(NULL);
			header->process(tmp_buffer, new_url, &cc_ctx);
			kbuf *buf = tmp_buffer.getHead();
			if (buf->next == NULL) {
				ctx->out->f->write_unknow_header(ctx->out->ctx, header->attr, header->attr_len, buf->data, buf->used);
			} else {
				KStringBuf val;
				while (buf) {
					val.write_all(buf->data, buf->used);
					buf = buf->next;
				}
				ctx->out->f->write_unknow_header(ctx->out->ctx, header->attr, header->attr_len, val.buf(), val.size());
			}
		}
		header = header->next;
	}
	char method[8];
	method[0] = '\0';
	DWORD method_len = sizeof(method) - 1;
	ctx->f->get_variable(r, KGL_VAR_REQUEST_METHOD, NULL, method, &method_len);
	if (strcmp(method, "HEAD") == 0) {
		return KGL_EDENIED;
	}
	while (buffer) {
		buffer->process(new_request, new_url, &cc_ctx);
		buffer = buffer->next;
	}
	int bc = new_request.getLen();
	auto buf = new_request.getHead();
	kgl_response_body body;
	auto result = ctx->out->f->write_header_finish(ctx->out->ctx, (int64_t)new_request.getLen(), &body);
	if (result != KGL_OK) {
		return result;
	}
	return body.f->close(body.ctx, body.f->writev(body.ctx, buf,bc));
}
bool KWafUpstream::CheckCCKey(KREQUEST r, kgl_async_context *ctx, const char *ips, const char *cc_key, int &cc_flag)
{
	if (!cc_key) {
		return false;
	}

	char md5buf[33];
	//printf("cc_key=%s\n",cc_key);
	if (strlen(cc_key) < 34) {
		return false;
	}
	KStringBuf s;

	int go_time = (int)(time(NULL) - atoi(cc_key + 33));
	if (go_time < MAX_ATTACK_ALIVE_TIME && go_time >= 0) {//时间有效
		s << *cc_key << ips;
		s.write_all(waf_config.cc_salt, waf_config.cc_salt_len);
		s << cc_key + 33;
		KMD5(s.c_str(), s.size(), md5buf);
		//		printf("*********s=%s,md5=%s\n",s.str().c_str(),md5buf);
		if (memcmp(cc_key + 1, md5buf, 32) == 0) {//是正常用户
			//printf("这是正常用户访问\n");
			char cc_flags[2] = { *cc_key, 0 };
			cc_flag = atoi(cc_flags);
			if (KBIT_TEST(cc_flag, ANTICC_WHITE_LIST)) {
				ctx->f->support_function(r, ctx->cn, KD_REQ_ADD_WHITE_LIST, (void *)ips, NULL);
			}
			return true;
		}
	}
	return false;
}
static kgl_upstream waf_upstream = {
	sizeof(kgl_upstream),
	KGL_UPSTREAM_BEFORE_CACHE,
	"waf",
	create_ctx,
	free_ctx,
	check,
	NULL
};
void create_waf_upstream(KREQUEST r, kgl_access_context *ctx, KChallenge *c)
{
	KWafUpstream *us = new KWafUpstream(c);
	ctx->f->support_function(r, ctx->cn, KF_REQ_UPSTREAM, &waf_upstream, (void **)&us);
}
