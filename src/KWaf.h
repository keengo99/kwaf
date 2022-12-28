#ifndef KGL_WAF_H
#define KGL_WAF_H
#include "ksapi.h"
#include "kmalloc.h"
#include <string>
#include "KWafConfig.h"
#include "KChallenge.h"

class KWaf: public KChallengeConfig
{
public:
	KWaf();
	~KWaf();
	KGL_RESULT build(kgl_access_build *build_ctx, KF_ACCESS_BUILD_TYPE build_type);
	KGL_RESULT parse(kgl_access_parse *parse_ctx, KF_ACCESS_PARSE_TYPE parse_type);
	uint32_t process(KREQUEST r, kgl_access_context *ctx, DWORD notify);
private:
	std::string msg;
	KChallenge *challenge;
};
extern kgl_dso_version *server_support;
#endif
