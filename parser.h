#ifndef __IZM_HTTP_PARSER_H__
#define __IZM_HTTP_PARSER_H__

#include <stddef.h>
#include <stdint.h>

struct izm_http_request_line {
	const char *method;
	size_t method_size;

	const char *target;
	size_t target_size;
	
	int httpver_major;
	int httpver_minor;
};

struct izm_http_status_line {
	int status_code;

	const char *reason;
	size_t reason_size;

	int httpver_major;
	int httpver_minor;
};

struct izm_http_header {
	const char *name;
	size_t name_size;

	const char *value;
	size_t value_size;	
};

struct izm_http_request_line_parser {
	int state;
	const char *p, *m;
};

struct izm_http_status_line_parser {
	int state;
	const char *p, *m;
};

struct izm_http_headers_parser {
	int state;
	const char *p;
	const char *nm, *vm;
	size_t ns;	
};

enum {
	IZM_PARSER_OK = 0,
	IZM_PARSER_CONTINUE = -1,
	IZM_PARSER_BAD_REQUEST = -2,
	IZM_PARSER_INVALID_PARSER = -3,
};

int izm_http_parse_request_line(struct izm_http_request_line_parser *parser,
				struct izm_http_request_line *l,
				size_t *bytes_scanned,
				const char *input, size_t input_size);

int izm_http_parse_status_line(struct izm_http_status_line_parser *parser,
			       struct izm_http_status_line *l,
			       size_t *bytes_scanned,
			       const char *input, size_t input_size);

int izm_http_parse_headers(struct izm_http_headers_parser *parser,
			   struct izm_http_header headers[], uint32_t headers_count,
			   uint32_t *headers_filled, size_t *bytes_scanned,
			   const char *input, size_t input_size);

#endif	/* __IZM_HTTP_PARSER_H__ */
