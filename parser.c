#include "parser.h"

#define USE_STREQU5
#define USE_BRANCH_PREDICT
#define USE_CHAR_MAPS

#if defined USE_STREQU5
#define STREQU5(A, B) ((A[0] == B[0]) && (A[1] == B[1])		\
		       && (A[2] == B[2]) && (A[3] == B[3])	\
		       && (A[4] == B[4]))
#else
#include <string.h>
#define STREQU5(A, B) (strncmp(A, B, 5) == 0)
#endif	/* USE_STREQU5 */

#if defined __GNUC__ && defined USE_BRANCH_PREDICT
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif	/* __GNUC__ && USE_BRANCH_PREDICT */

#ifdef USE_CHAR_MAPS

static const char DIGIT_MAP[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

static const char DELIMITERS_MAP[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0, 0, 0,
	1, 1, 0, 0, 1, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 1, 0, 0
};

static const char VCHAR_MAP[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 0
};

static const char TCHAR_MAP[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 1, 1, 1, 1, 1,
	0, 0, 1, 1, 0, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 0, 0, 0, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 0, 1, 0, 1, 0
};

#endif	/* USE_CHAR_MAPS */

/* RFC5234 core rules refered by RFC7230 */
inline static int
is_digit(char c)
{
#ifndef USE_CHAR_MAPS
	return c >= '0' && c <= '9';
#else
	return DIGIT_MAP[(int)c];
#endif
}

inline static int
is_vchar(char c)
{
#ifndef USE_CHAR_MAPS
	return c >= '\x21' && c <= '\x7e';
#else
	return VCHAR_MAP[(int)c];
#endif
}

inline static int
is_whitespace(char c)
{
	return c == ' ' || c == '\t';
}

/* Common syntax components */
inline static int
is_delimiters(char c)
{
#ifndef USE_CHAR_MAPS
	return c == '(' || c == ')' || c == ',' || c == '/'
		|| c == ':'  || c == ';' || c == '<' || c == '='
		|| c == '>'  || c == '?' || c == '@' || c == '['
		|| c == '\\' || c == ']' || c == '{' || c == '}'
		|| c == '"';
#else
	return DELIMITERS_MAP[(int)c];
#endif
}

inline static int
is_tchar(char c)
{
#ifndef USE_CHAR_MAPS
	return is_vchar(c) && !is_whitespace(c) && !is_delimiters(c);
#else
	return TCHAR_MAP[(int)c];
#endif
}

inline static int
is_obs_text(char c)
{
	return c < 0;
}

int
izm_http_parse_request_line(struct izm_http_request_line_parser *parser,
			    struct izm_http_request_line *l,
			    size_t *bytes_scaned,
			    const char *input, size_t input_size)
{	
	enum {
		S_INIT = 0,
		S_BAD_REQ,
		S_METHOD,
		S_BEFORE_TARGET,
		S_TARGET,
		S_BEFORE_HTTPVER,
		S_BEFORE_MAJOR_VER,
		S_MAJOR_VER,
		S_DOT,
		S_MINOR_VER,
		S_CR,
		S_LF,
		S_FINISHED,
	};

	const char *end = input + input_size;

	if (parser->state == S_INIT) { /* initialize parser */
		parser->p = parser->m = input;
		parser->state = S_METHOD;
	}

	const char *p = parser->p;
	const char *m = parser->m;
	
	while (p < end) {		
		switch (parser->state) {
		case S_BAD_REQ:
			goto BAD_REQUEST;			
		case S_FINISHED:
			goto FINISHED;			
		case S_METHOD:
			if (!is_tchar(*p)) {
				if (unlikely(*p != ' '))
					goto BAD_REQUEST;
				
				size_t size = p - m;
				if (unlikely(size == 0))
					goto BAD_REQUEST;
				
				l->method = m;
				l->method_size = size;
				parser->state = S_BEFORE_TARGET;
			}
			break;
		case S_BEFORE_TARGET:
			m = p;
			parser->state = S_TARGET;
			/* FALL THROUGH */
		case S_TARGET:
			if (!is_vchar(*p) && !is_obs_text(*p)) {
				if (unlikely(*p != ' '))
					goto BAD_REQUEST;

				size_t size = p - m;
				if (unlikely(size == 0))
					goto BAD_REQUEST;

				l->target = m;
				l->target_size = size;
				parser->state = S_BEFORE_HTTPVER;
			}
			break;
		case S_BEFORE_HTTPVER:
			m = p;
			parser->state = S_BEFORE_MAJOR_VER;
			break;
		case S_BEFORE_MAJOR_VER:
			if ((p - m) < 5) break;
			if (unlikely(!STREQU5(m, "HTTP/")))
				goto BAD_REQUEST;
			
			parser->state = S_MAJOR_VER;
			/* FALL THROUGH */
		case S_MAJOR_VER:
			if (unlikely(!is_digit(*p)))
				goto BAD_REQUEST;

			l->httpver_major = *p - '0';
			parser->state = S_DOT;
			break;
		case S_DOT:
			if (unlikely(*p != '.'))
				goto BAD_REQUEST;
			
			parser->state = S_MINOR_VER;
			break;
		case S_MINOR_VER:
			if (unlikely(!is_digit(*p)))
				goto BAD_REQUEST;
			
			l->httpver_minor = *p - '0';
			parser->state = S_CR;
			break;
		case S_CR:
			if (unlikely(*p != '\r'))
				goto BAD_REQUEST;
			parser->state = S_LF;
			break;
		case S_LF:
			if (unlikely(*p != '\n'))
				goto BAD_REQUEST;
			
			goto FINISHED;
		default:
			return IZM_PARSER_INVALID_PARSER;
		}
		++p;
	}
	parser->m = m;
	parser->p = p;
	*bytes_scaned = p - input;
	return IZM_PARSER_CONTINUE;

BAD_REQUEST:
	parser->state = S_BAD_REQ;
	*bytes_scaned = p - input + 1;
	return IZM_PARSER_BAD_REQUEST;

FINISHED:
	parser->state = S_FINISHED;
	*bytes_scaned = p - input + 1;
	return IZM_PARSER_OK;
}

int
izm_http_parse_status_line(struct izm_http_status_line_parser *parser,
			   struct izm_http_status_line *l,
			   size_t *bytes_scaned,
			   const char *input, size_t input_size)
{
	const char *end = input + input_size;

	enum {
		S_INIT = 0,
		S_BAD_REQ,
		S_BEFORE_MAJOR_VER,
		S_MAJOR_VER,
		S_DOT,
		S_MINOR_VER,
		S_BEFORE_STATUS_CODE,
		S_STATUS_CODE,
		S_BEFORE_REASON,
		S_REASON,
		S_CRLF,
		S_FINISHED,
	};

	if (parser->state == S_INIT) {
		parser->state = S_BEFORE_MAJOR_VER;
		parser->p = parser->m = input;
	}

	const char *p = parser->p, *m = parser->m;

	while (p < end) {
		switch (parser->state) {
		case S_BAD_REQ:
			goto BAD_REQUEST;
		case S_FINISHED:
			goto FINISHED;
		case S_BEFORE_MAJOR_VER:
			if (p - m < 5) break;
			if (unlikely(!STREQU5(m, "HTTP/")))
				goto BAD_REQUEST;
			
			parser->state = S_MAJOR_VER;
			/* FALL THROUGH */
		case S_MAJOR_VER:
			if (unlikely(!is_digit(*p)))
				goto BAD_REQUEST;
			
			l->httpver_major = *p - '0';
			parser->state = S_DOT;
			break;
		case S_DOT:
			if (unlikely(*p != '.'))
				goto BAD_REQUEST;
			
			parser->state = S_MINOR_VER;
			break;
		case S_MINOR_VER:
			if (unlikely(!is_digit(*p)))
				goto BAD_REQUEST;
			
			l->httpver_minor = *p - '0';
			parser->state = S_BEFORE_STATUS_CODE;	
			break;
		case S_BEFORE_STATUS_CODE:
			if (unlikely(*p != ' '))
				goto BAD_REQUEST;

			m = p + 1;
			parser->state = S_STATUS_CODE;
			break;
		case S_STATUS_CODE:
			if (p - m < 3) {
				if (unlikely(!is_digit(*p)))
					goto BAD_REQUEST;

				break;
			}
			
			l->status_code = (m[0] - '0') * 100
				+ (m[1] - '0') * 10
				+ (m[2] - '0');
			parser->state = S_BEFORE_REASON;
			/* FALL THROUGH */
		case S_BEFORE_REASON:
			if (unlikely(*p != ' '))
				goto BAD_REQUEST;

			parser->state = S_REASON;
			m = p + 1;
			break;
		case S_REASON:
			if (!is_vchar(*p) && !is_whitespace(*p) && !is_obs_text(*p)) {
				if (likely(*p == '\r')) {
					l->reason = m;
					l->reason_size = p - m;
					parser->state = S_CRLF;
				} else {
					goto BAD_REQUEST;
				}
			}			
			break;
		case S_CRLF:
			if (unlikely(*p != '\n'))
				goto BAD_REQUEST;

			goto FINISHED;
		}
		++p;
	}

	parser->m = m;
	parser->p = p;
	*bytes_scaned = p - input;
	return IZM_PARSER_CONTINUE;

BAD_REQUEST:
	parser->state = S_BAD_REQ;
	*bytes_scaned = p - input + 1;
	return IZM_PARSER_BAD_REQUEST;

FINISHED:
	parser->state = S_FINISHED;
	*bytes_scaned = p - input + 1;
	return IZM_PARSER_OK;
}

int
izm_http_parse_headers(struct izm_http_headers_parser *parser,
		       struct izm_http_header headers[], uint32_t headers_count,
		       uint32_t *headers_filled, size_t *bytes_scaned,
		       const char *input, size_t input_size)
{
	enum {
		S_INIT = 0,
		S_BAD_REQ,
		S_BEFORE_NAME,
		S_NAME,
		S_BEFORE_VALUE,
		S_VALUE,
		S_CR,
		S_LF,
		S_AFTER_FIELD,
		S_EOF,
		S_FINISHED,
	};

	if (headers_count <= 0)
		return IZM_PARSER_OK;

	const char *end = input + input_size;
	uint32_t header_idx = 0;

	if (parser->state == S_INIT) {
		parser->p = input;
		parser->state = S_BEFORE_NAME;
	}

	const char *p = parser->p, *nm = NULL, *vm = NULL;
	size_t ns = 0;

	while (p < end) {
		switch (parser->state) {
		case S_BAD_REQ:
			goto BAD_REQUEST;
		case S_FINISHED:
			goto FINISHED;
		case S_BEFORE_NAME:
			if (*p == '\r') {
				parser->state = S_EOF;
				break;
			}
			
			if (header_idx >= headers_count)
				goto CONTINUE;
			
			nm = p;
			parser->state = S_NAME;
			/* FALL THROUGH */
		case S_NAME:
			if (!is_tchar(*p)) {
				if (unlikely(*p != ':'))
					goto BAD_REQUEST;

				ns = p - nm;
				if (unlikely(ns == 0))
					goto BAD_REQUEST;

				parser->state = S_BEFORE_VALUE;
			}
			break;
		case S_BEFORE_VALUE:
			if (is_whitespace(*p)) break;
			vm = p;
			parser->state = S_VALUE;
			/* FALL THROUGH */
		case S_VALUE:
			if (!is_vchar(*p) && !is_obs_text(*p) && !is_whitespace(*p)) {			
				if (unlikely(*p != '\r'))
					goto BAD_REQUEST;
				
				parser->state = S_CR;
			}
			break;
		case S_CR:
			if (unlikely(*p != '\n'))
				goto BAD_REQUEST;

			parser->state = S_LF;
			break;
		case S_LF:
			if (is_whitespace(*p)) {
				parser->state = S_VALUE;
				break;
			} else {
				parser->state = S_AFTER_FIELD;
				/* FALL THROUGH */
			}
		case S_AFTER_FIELD:
			headers[header_idx].name = nm;
			headers[header_idx].name_size = ns;
			headers[header_idx].value = vm;
			headers[header_idx].value_size = p - vm - 2;
			++header_idx;			

			parser->state = S_BEFORE_NAME;
			
			continue;
		case S_EOF:
			if (unlikely(*p != '\n'))
				goto BAD_REQUEST;

			goto FINISHED;
		default:
			return IZM_PARSER_INVALID_PARSER;
		}
		++p;
	}
	
CONTINUE:
	*headers_filled = header_idx;
	parser->p = p;
	parser->nm = nm;
	parser->ns = ns;
	parser->vm = vm;
	*bytes_scaned = p - input;
	return IZM_PARSER_CONTINUE;

BAD_REQUEST:
	parser->state = S_BAD_REQ;
	*headers_filled = header_idx;
	*bytes_scaned = p - input + 1;
	return IZM_PARSER_BAD_REQUEST;

FINISHED:
	parser->state = S_FINISHED;
	*headers_filled = header_idx;
	*bytes_scaned = p - input + 1;
	return IZM_PARSER_OK;	
}
