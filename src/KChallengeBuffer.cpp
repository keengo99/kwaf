#include "KChallengeBuffer.h"
#include "KChallenge.h"
#include "KChallengeSession.h"

inline static void rand_space(KDsoAutoBuffer &s)
{
	int num = rand() & 3;
	s.write_all("                                    ", num);
}
void KChallengeBuffer::process(KDsoAutoBuffer &new_request, KStringBuf &new_url, KAntiRefreshParserContext *ctx)
{
	switch (type) {
	case CC_BUFFER_URL:
		new_request.write_all(new_url.c_str(), new_url.size());
		break;
	case CC_BUFFER_MURL:
	{
		const char *tbuf = new_url.c_str();
		int len = new_url.size();
		while (len > 0) {
			int rand_len = ((rand() & 7) + 1);
			int this_len = KGL_MIN(len, rand_len);
			new_request.write_all(tbuf, this_len);
			tbuf += this_len;
			len -= this_len;
			if (len > 0) {
				new_request.write_all("\"", 1);
				rand_space(new_request);
				new_request.write_all("+", 1);
				rand_space(new_request);
				new_request.write_all("\"", 1);
			}
		}
		break;
	}
	case CC_BUFFER_REVERT:
	{
		if (buf == NULL || *buf == '\0') {
			break;
		}
		new_request << "var " << buf << "='';";
		const char *tbuf = new_url.c_str();
		int len = new_url.size();
		while (len > 0) {
			new_request << buf << "=";
			rand_space(new_request);
			new_request << "'";
			int rand_len = ((rand() & 7) + 1);
			int this_len = KGL_MIN(len, rand_len);
			int start = len - this_len;
			new_request.write_all(tbuf + start, this_len);
			len -= this_len;
			new_request << "'";
			rand_space(new_request);
			new_request << "+";
			rand_space(new_request);
			new_request << buf;
			new_request << ";";
		}
		break;
	}
	case CC_BUFFER_SC_KEY:
	{
		if (ctx->img_key == -1) {
			char *u = strdup(new_url.c_str());
			ctx->img_key = create_session_number(u);
		}
		new_request << ctx->img_key;
		break;
	}
	case CC_BUFFER_SB:
		break;
	default:
		if (buf) {
			new_request.write_all(buf, len);
		}
		break;
	}
}