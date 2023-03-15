#include "ksapi.h"
#include "KWaf.h"
#include "KWafSession.h"
static void *create_ctx()
{
	return new KWaf;
}
static void free_ctx(void *ctx)
{
	KWaf *waf = (KWaf *)ctx;
	delete waf;
}
static KGL_RESULT build(kgl_access_build *build_ctx, uint32_t build_type)
{
	KWaf *lua = (KWaf *)build_ctx->module;
	return lua->build(build_ctx, build_type);
}
static KGL_RESULT parse(kgl_access_parse_config *parse_ctx)
{
	KWaf *lua = (KWaf *)parse_ctx->module;
	return lua->parse(parse_ctx);
}
static uint32_t process(KREQUEST rq, kgl_access_context *ctx, DWORD notify)
{
	KWaf *lua = (KWaf *)ctx->module;
	return lua->process(rq, ctx, notify);
}
static void *create_ctx_session()
{
	return new KWafSession;
}
static void free_ctx_session(void *ctx)
{
	KWafSession *waf = (KWafSession *)ctx;
	delete waf;
}
static KGL_RESULT build_session(kgl_access_build *build_ctx, uint32_t build_type)
{
	KWafSession *lua = (KWafSession *)build_ctx->module;
	return lua->build(build_ctx, build_type);
}
static KGL_RESULT parse_session(kgl_access_parse_config *parse_ctx)
{
	KWafSession *lua = (KWafSession *)parse_ctx->module;
	return lua->parse(parse_ctx);
}
static uint32_t process_session(KREQUEST rq, kgl_access_context *ctx, DWORD notify)
{
	KWafSession *lua = (KWafSession *)ctx->module;
	return lua->process(rq, ctx, notify);
}
static kgl_access anti_cc_model = {
	sizeof(kgl_access),
	KF_NOTIFY_REQUEST_MARK|KF_NOTIFY_REPLACE,
	"anti_cc",
	create_ctx,
	free_ctx,
	build,
	parse,
	NULL,
	process,
	NULL
};

static kgl_access anti_session_model = {
	sizeof(kgl_access),
	KF_NOTIFY_REQUEST_MARK | KF_NOTIFY_REPLACE,
	"anti_session",
	create_ctx_session,
	free_ctx_session,
	build_session,
	parse_session,
	NULL,
	process_session,
	NULL
};
void register_access(kgl_dso_version *ver)
{
	KGL_REGISTER_ACCESS(ver, &anti_cc_model);
	KGL_REGISTER_ACCESS(ver, &anti_session_model);
}