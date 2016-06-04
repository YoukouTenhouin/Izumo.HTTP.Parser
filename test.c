#include <stdarg.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

#include "parser.h"

int tests;
int failures;

int
bufis(const char *buf, size_t buf_size, const char *str)
{
	return buf_size == strlen(str) && strncmp(buf, str, buf_size) == 0;	
}

void
ok(int cond)
{
	++tests;
	if (cond) {
		printf("succeed\n");		
	} else {
		failures++;
		printf("failed\n");
	}
}

void
rlp_test()
{
	struct izm_http_request_line rl;
	struct izm_http_request_line_parser rlp;

	int ret;
	size_t bytes_parsed;

#define PARSE(s, init, name)						\
	do {								\
		if (init) {						\
			rlp.state = 0;					\
			bzero(&rl, sizeof(rl));				\
		}							\
		if (name) printf("\tRunning test: %s......", name);	\
		ret = izm_http_parse_request_line(&rlp, &rl, &bytes_parsed, s, sizeof(s) - 1); \
	} while(0)

	PARSE("GET / HTTP/1.0\r\n", 1, "simple");	
	ok((ret == IZM_PARSER_OK)
	   && bufis(rl.method, rl.method_size, "GET")
	   && bufis(rl.target, rl.target_size, "/")
	   && (rl.httpver_major == 1)
	   && (rl.httpver_minor == 0));

	PARSE("GET / HTTP/1.1\r", 1, "partial");
	ok((ret == IZM_PARSER_CONTINUE));

	PARSE("GET /doge HTTP/1.1\r\n", 1, "doggy target");
	ok((ret == IZM_PARSER_OK)
	   && bufis(rl.method, rl.method_size, "GET")
	   && bufis(rl.target, rl.target_size, "/doge")
	   && (rl.httpver_major == 1)
	   && (rl.httpver_minor == 1));

	PARSE("GET", 1, "incomplete 1");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (rl.method == NULL)
	   && (rl.method_size == 0));	

	PARSE("GET ", 1, "incomplete 2");
	ok((ret == IZM_PARSER_CONTINUE)
	   && bufis(rl.method, rl.method_size, "GET"));

	PARSE("GET /", 1, "incomplete 3");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (rl.target == NULL));	

	PARSE("GET / ", 1, "incomplete 4");
	ok((ret == IZM_PARSER_CONTINUE)
	   && bufis(rl.target, rl.target_size, "/"));

	PARSE("GET / H", 1, "incomplete 5");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (rl.httpver_major == 0));
	
	PARSE("GET / HTTP/1.", 1, "incomplete 6");
	ok((ret == IZM_PARSER_CONTINUE)
	    && (rl.httpver_major == 1));

	PARSE("GET / HTTP/1.1", 1, "incomplete 7");
	ok((ret == IZM_PARSER_CONTINUE)
	    && (rl.httpver_minor == 1));

	PARSE("G\0T / HTTP/1.1\r\n", 1, "NUL in method");
	ok(ret == IZM_PARSER_BAD_REQUEST);

	PARSE("G\tT / HTTP/1.1\r\n", 1, "tab in method");
	ok(ret == IZM_PARSER_BAD_REQUEST);

	PARSE("GET /\x7f""doge HTTP/1.1\r\n", 1, "DEL in target");
	ok(ret == IZM_PARSER_BAD_REQUEST);

	PARSE("GET /\xa0 HTTP/1.1\r\n", 1, "accept MSB chars");
	ok((ret == IZM_PARSER_OK)
	   && bufis(rl.method, rl.method_size, "GET")
	   && bufis(rl.target, rl.target_size, "/\xa0")
	   && (rl.httpver_major == 1)
	   && (rl.httpver_minor == 1));
	
#undef PARSE
}

void
slp_test()
{
	struct izm_http_status_line sl;
	struct izm_http_status_line_parser slp;

	int ret;
	size_t bytes_parsed;

#define PARSE(s, init, name)						\
	do {								\
		if (init) {						\
			slp.state = 0;					\
			bzero(&sl, sizeof(sl));				\
		}							\
		if (name) printf("\tRunning test: %s......", name);	\
		ret = izm_http_parse_status_line(&slp, &sl, &bytes_parsed, s, sizeof(s) - 1); \
	} while(0)

	PARSE("HTTP/1.1 200 OK\r\n", 1, "simple");
	ok((ret == IZM_PARSER_OK)
	   && (sl.status_code == 200)
	   && bufis(sl.reason, sl.reason_size, "OK")
	   && (sl.httpver_major == 1)
	   && (sl.httpver_minor == 1));

	PARSE("HTTP/1.1 200 OK\r", 1, "partial");
	ok((ret == IZM_PARSER_CONTINUE));

	PARSE("HTTP/1.1 200 DOGE\r\n", 1, "doggy reason");
	ok((ret == IZM_PARSER_OK)
	   && bufis(sl.reason, sl.reason_size, "DOGE"));

	PARSE("HTTP/1.1 418 I'm a teapot\r\n", 1, "whitespace in reason");
	ok((ret == IZM_PARSER_OK)
	   && bufis(sl.reason, sl.reason_size, "I'm a teapot"));

	PARSE("HTT", 1, "incomplete 1");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (sl.httpver_major == 0));

	PARSE("HTTP/1.", 1, "incomplete 2");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (sl.httpver_major == 1));

	PARSE("HTTP/1.1 ", 1, "incomplete 3");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (sl.httpver_minor == 1));

	PARSE("HTTP/1.1 20", 1, "incomplete 4");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (sl.status_code == 0));

	PARSE("HTTP/1.1 200 ", 1, "incomplete 5");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (sl.status_code == 200));

	PARSE("HTTP/1.1 200 OK", 1, "incomplete 6");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (sl.reason == NULL));

	PARSE("HTTP/1.1 200 OK\r", 1, "incomplete 7");
	ok((ret == IZM_PARSER_CONTINUE)
	   && bufis(sl.reason, sl.reason_size, "OK"));

	PARSE("HTTP@1.1 200 OK\r\n", 1, "invalid version string");
	ok(ret == IZM_PARSER_BAD_REQUEST);

	PARSE("HTTP/1.1 2A0 OK\r\n", 1, "hex status code");
	ok(ret == IZM_PARSER_BAD_REQUEST);

	PARSE("HTTP/1.1 2000 OK\r\n", 1, "Status Code ME(R)");
	ok(ret == IZM_PARSER_BAD_REQUEST);

	PARSE("HTTP/1.1 200 O\x7fK\r\n", 1, "DEL in reason");
	ok(ret == IZM_PARSER_BAD_REQUEST);

#undef PARSE
}

void
hp_test() {
	struct izm_http_header h[32];
	struct izm_http_headers_parser hp;

	int hc = sizeof(h) / sizeof(h[0]), hf;
	
	int ret;
	size_t bytes_parsed;

#define PARSE(s, init, name)						\
	do {								\
		if (init) {						\
			hp.state = 0;					\
			bzero(h, sizeof(h));				\
		}							\
		if (name) printf("\tRunning test: %s......", name);	\
		ret = izm_http_parse_headers(&hp, h, hc, &hf, &bytes_parsed, s, sizeof(s) - 1); \
	} while(0)

	PARSE("\r\n", 1, "eof");
	ok((ret == IZM_PARSER_OK)
	   && (hf == 0));

	PARSE("Host: remotehost\r\n\r\n", 1, "simple");
	ok((ret == IZM_PARSER_OK)
	   && (hf == 1)
	   && bufis(h[0].name, h[0].name_size, "Host")
	   && bufis(h[0].value, h[0].value_size, "remotehost"));

	PARSE("Host: remotehost\r\n"
	      "User-Agent: \xe5\xae\x8c\xe7\xbe\x8e\xe7\x9a\x84\xe7\x80\x8f\xe8\xa6\xbd\xe5\x99\xa8 1.0\r\n\r\n", 1, "multibyte included");
	ok((ret == IZM_PARSER_OK)
	   && (hf == 2)
	   && bufis(h[0].name, h[0].name_size, "Host")
	   && bufis(h[0].value, h[0].value_size, "remotehost")
	   && bufis(h[1].name, h[1].name_size, "User-Agent")
	   && bufis(h[1].value, h[1].value_size, "\xe5\xae\x8c\xe7\xbe\x8e\xe7\x9a\x84\xe7\x80\x8f\xe8\xa6\xbd\xe5\x99\xa8 1.0"));

	PARSE("Host: remotehost\r\n"
	      "fool: b\r\n \tc\r\n\r\n", 1, "parse multiline");
	ok((ret == IZM_PARSER_OK)
	   && (hf == 2)
	   && bufis(h[1].name, h[1].name_size, "fool")
	   && bufis(h[1].value, h[1].value_size, "b\r\n \tc"));

	PARSE("Instruc", 1, "incomplete 1");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (hf == 0)
	   && (h[0].name == NULL));

	PARSE("Instruction: ", 1, "incomplete 2");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (hf == 0));

	PARSE("Instruction: HCF", 1, "incomplete 3");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (hf == 0));

	PARSE("Instruction: HCF\r\nLP0", 1, "incomplete 4");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (hf == 1)
	   && bufis(h[0].name, h[0].name_size, "Instruction")
	   && bufis(h[0].value, h[0].value_size, "HCF")
	   && (h[1].name == NULL));

	PARSE("Instruction: HCF\r\nLP0: ", 1, "incomplete 5");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (hf == 1)
	   && (h[1].name == NULL));

	PARSE("Instruction: HCF\r\nLP0: On Fire\r\n", 1, "incomplete 6");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (hf == 1));

	PARSE("Instruction: HCF\r\nLP0: On Fire\r\n\r", 1, "incomplete 7");
	ok((ret == IZM_PARSER_CONTINUE)
	   && (hf == 2)
	   && bufis(h[1].name, h[1].name_size, "LP0")
	   && bufis(h[1].value, h[1].value_size, "On Fire"));
	

	PARSE("d\0ge: doge\r\n\r\n", 1, "NUL in field name");
	ok((ret == IZM_PARSER_BAD_REQUEST)
	   && (hf == 0));

	PARSE("doge: d\0ge\r\n\r\n", 1, "NUL in field value");
	ok((ret == IZM_PARSER_BAD_REQUEST)
	   && (hf == 0)
	   && (h[0].name == NULL)
	   && (h[0].value == NULL));

	PARSE("do\033ge: doge\r\n\r\n", 1, "CTL in field name");
	ok(ret == IZM_PARSER_BAD_REQUEST);

	PARSE("doge: do\033ge\r\n\r\n", 1, "CTL in field value");
	ok(ret == IZM_PARSER_BAD_REQUEST);

	PARSE("fool : doge\r\n\r\n", 1, "header with trailing space");
	ok(ret == IZM_PARSER_BAD_REQUEST);

	PARSE("mouse: \xe3\x81\xab\xe3\x82\x83\r\n\r\n", 1, "accept MSB chars");
	ok((ret == IZM_PARSER_OK)
	   && (hf == 1)
	   && bufis(h[0].name, h[0].name_size, "mouse")
	   && bufis(h[0].value, h[0].value_size, "\xe3\x81\xab\xe3\x82\x83"));

#undef PARSE
}

int
main()
{
	printf("Running RLP tests:\n");
	rlp_test();
	
	printf("Running SLP tests:\n");
	slp_test();
	
	printf("Running HP tests:\n");
	hp_test();

	printf("%d/%d tests failed\n", failures, tests);	
}
