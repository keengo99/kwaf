#include "KChallenge.h"
anti_cc_model anti_cc_models[] =
{
	{"http redirect","HTTP/1.1 302 FOUND\\r\\nConnection: keep-alive\\r\\nLocation: {{url}}\\r\\n\\r\\n<html><body><a href=\"{{url}}\">continue</a></body></html>"},
	{"html redirect","HTTP/1.1 200 OK\\r\\nContent-Type: text/html; charset=utf-8\\r\\nConnection: keep-alive\\r\\nCache-Control: no-cache,no-store\\r\\n\\r\\n<html><head><meta http-equiv=\"refresh\" content=\"0;url={{url}}\"></head><body><a href=\"{{url}}\">continue</a></body></html>"},
	{"js plain","HTTP/1.1 200 OK\\r\\nContent-Type: text/html; charset=utf-8\\r\\nConnection: keep-alive\\r\\nCache-Control: no-cache,no-store\\r\\n\\r\\n<html><body><sc'+'ript language=\"javascript\">window.location=\"{{url}}\";</'+'script><a href=\"{{url}}\">continue</a></body></html>"},
	{"js concat","HTTP/1.1 200 OK\\r\\nContent-Type: text/html; charset=utf-8\\r\\nConnection: keep-alive\\r\\nCache-Control: no-cache,no-store\\r\\n\\r\\n<html><body><sc'+'ript language=\"javascript\">window.location=\"{{murl}}\";</'+'script></body></html>"},
	{"js revert","HTTP/1.1 200 OK\\r\\nContent-Type: text/html; charset=utf-8\\r\\nConnection: keep-alive\\r\\nCache-Control: no-cache,no-store\\r\\n\\r\\n<html><body><sc'+'ript language=\"javascript\">{{revert:url}};window.location=url;</'+'script></body></html>"},
	{"html manual","HTTP/1.1 200 OK\\r\\nContent-Type: text/html; charset=utf-8\\r\\nConnection: keep-alive\\r\\nCache-Control: no-cache,no-store\\r\\n\\r\\n<html><body><a href=\"{{url}}\">continue</a></body></html>"},
	{"deny","HTTP/1.1 200 OK\\r\\nContent-Type: text/html; charset=utf-8\\r\\nConnection: keep-alive\\r\\nCache-Control: no-cache,no-store\\r\\n\\r\\n<html><body>try again later</body></html>"}
};
