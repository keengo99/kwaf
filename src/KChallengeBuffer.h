#ifndef WAF_CHALLENGE_BUFFER_H
#define WAF_CHALLENGE_BUFFER_H
#include "kmalloc.h"
#include <string.h>
#include "KDsoAutoBuffer.h"
#include "KStringBuf.h"
class KAntiRefreshParserContext
{
public:
	KAntiRefreshParserContext()
	{
		img_key = -1;
	}
	~KAntiRefreshParserContext() {

	}
	int img_key;
};
class KChallengeBuffer
{
public:
	KChallengeBuffer()
	{
		memset(this, 0, sizeof(KChallengeBuffer));
	}
	~KChallengeBuffer()
	{
		if (buf) {
			free(buf);
		}
	}
	void process(KDsoAutoBuffer &new_request, KStringBuf &new_url, KAntiRefreshParserContext *ctx);
	u_short type;
	u_short len;
	char *buf;
	KChallengeBuffer *next;
};
#endif
