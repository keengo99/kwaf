#ifndef WAF_DSO_ANTI_SESSION_H
#define WAF_DSO_ANTI_SESSION_H
#include "ksapi.h"
#include "KChallengeSession.h"
#include <sstream>
class KWafSession {
public:
	KGL_RESULT build(kgl_access_build *build_ctx, uint32_t build_type)
	{
		std::stringstream s;
		s << get_challenge_session_count();
		build_ctx->write_string(build_ctx->cn, s.str().c_str(), (int)s.str().size(),0);
		return KGL_OK;
	}
	KGL_RESULT parse(kgl_access_parse_config *parse_ctx)
	{
		return KGL_OK;
	}
	uint32_t process(KREQUEST r, kgl_access_context *ctx, DWORD notify);
};
#endif