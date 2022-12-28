#ifndef WAF_DSO_KCHALLENGE_SESSION_H
#define WAF_DSO_KCHALLENGE_SESSION_H
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
class KChallengeSession
{
public:
	KChallengeSession(char *url)
	{
		unsigned r = rand();
		//È¡4Î»Êý
		number = (r % 10000);
		this->url = url;
	}
	KChallengeSession()
	{
		memset(this, 0, sizeof(KChallengeSession));
	}
	~KChallengeSession()
	{
		if (url) {
			free(url);
		}
	}
	int key;
	int number;
	char *url;
	time_t expire_time;
	KChallengeSession *next;
	KChallengeSession *prev;
};
int get_challenge_session_count();
int create_session_number(char *url);
int anticc_session_number(int key);
char *anticc_session_verify(int key, int &number);
void anticc_session_flush(time_t now_time);
void init_anticc_session();
#endif

