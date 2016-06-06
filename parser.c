#include "parser.h"

#define USE_STREQU5
#define USE_BRANCH_PREDICT
#define USE_CHAR_MAPS

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wattributes" /* SHUT UP GCC */
#define FORCEINLINE __attribute__((always_inline))
#else
#define FORCEINLINE
#endif	/* __GNUC__ */

#if defined USE_AVX2 && defined __AVX2__
#define USE_AVX2_
#endif	/* USE_AVX2 && __AVX2__ */

#if defined USE_AVX2_
#include <immintrin.h>

FORCEINLINE static unsigned long
_tzcnt(uint64_t in)
{
	unsigned long res;
	asm ("tzcnt %1, %0\n\t" :"=r"(res) : "r"(in));
	return res;
}

FORCEINLINE static uint32_t
_find_method_avx2_32(__m256i b0)
{
	__m256i _rr0 = _mm256_set1_epi8(0x41);
	__m256i _rr1 = _mm256_set1_epi8(0x5A);

	__m256i _ir0 = _mm256_or_si256(_mm256_cmpgt_epi8(_rr0, b0), _mm256_cmpgt_epi8(b0, _rr1));

	return _mm256_movemask_epi8(_ir0);
}

FORCEINLINE static uint32_t
_find_target_avx2_32(__m256i b0)
{
	__m256i _rr0 = _mm256_set1_epi8(0x00 -1);
	__m256i _rr1 = _mm256_set1_epi8(0x33);
	__m256i _rr2 = _mm256_set1_epi8(0x7F);

	__m256i _ir0 = _mm256_cmpgt_epi8(b0, _rr0);
	__m256i _ir1 = _mm256_cmpgt_epi8(_rr1, b0);
	__m256i _ir2 = _mm256_cmpeq_epi8(b0, _rr2);
	__m256i _ir3 = _mm256_and_si256(_ir0, _ir1);
	__m256i _ir4 = _mm256_andnot_si256(_ir2, _ir3);

	return _mm256_movemask_epi8(_ir4);	
}

FORCEINLINE static uint64_t
_find_target_avx2_64(__m256i b0, __m256i b1)
{
	__m256i _rr0 = _mm256_set1_epi8(0x00 - 1);
	__m256i _rr1 = _mm256_set1_epi8(0x21);
	__m256i _rr2 = _mm256_set1_epi8(0x7F);

	__m256i _ir0 = _mm256_cmpgt_epi8(b0, _rr0);
	__m256i _ir1 = _mm256_cmpgt_epi8(_rr1, b0);
	__m256i _ir2 = _mm256_cmpeq_epi8(_rr2, b0);
	__m256i _ir3 = _mm256_and_si256(_ir0, _ir1);
	__m256i _ir4 = _mm256_andnot_si256(_ir2, _ir3);

	uint64_t _r0 = _mm256_movemask_epi8(_ir4);

	_ir0 = _mm256_cmpgt_epi8(b1, _rr0);
	_ir1 = _mm256_cmpgt_epi8(_rr1, b1);
	_ir2 = _mm256_cmpeq_epi8(_rr2, b1);
	_ir3 = _mm256_and_si256(_ir0, _ir1);
	_ir4 = _mm256_andnot_si256(_ir2, _ir3);

	uint64_t _r1 = _mm256_movemask_epi8(_ir4);

	return (_r1 << 32) | _r0;	
}

FORCEINLINE static uint32_t
_match_httpver_avx2(__m256i b0, __m256i b1)
{
	__m256i _ir0 = _mm256_cmpeq_epi8(b0, b1);
	uint32_t bitmap = _mm256_movemask_epi8(_ir0);
	return bitmap & 0xFF;
}

FORCEINLINE static uint32_t
_find_field_name_avx2_32(__m256i b0)
{
	__m256i _rr0 = _mm256_set1_epi8(0x00 - 1);
	__m256i _rr1 = _mm256_set1_epi8(0x30);
	__m256i _rr2 = _mm256_set1_epi8(0x39);
	__m256i _rr3 = _mm256_set1_epi8(0x41);
	__m256i _rr4 = _mm256_set1_epi8(0x5A);
	__m256i _rr5 = _mm256_set1_epi8(0x61);
	__m256i _rr6 = _mm256_set1_epi8(0x7A);
	__m256i _rr7 = _mm256_set1_epi8(0x2D);

	__m256i _ir0 = _mm256_and_si256(_mm256_cmpgt_epi8(b0, _rr0), _mm256_cmpgt_epi8(_rr1, b0));
	__m256i _ir1 = _mm256_and_si256(_mm256_cmpgt_epi8(b0, _rr2), _mm256_cmpgt_epi8(_rr3, b0));
	__m256i _ir2 = _mm256_and_si256(_mm256_cmpgt_epi8(b0, _rr4), _mm256_cmpgt_epi8(_rr5, b0));
	__m256i _ir3 = _mm256_cmpeq_epi8(b0, _rr6);
	__m256i _ir4 = _mm256_cmpeq_epi8(b0, _rr7); 
	__m256i _ir5 = _mm256_or_si256(_ir0, _ir1);
	__m256i _ir6 = _mm256_or_si256(_ir2, _ir3);
	__m256i _ir7 = _mm256_or_si256(_ir5, _ir6);
	__m256i _ir8 = _mm256_andnot_si256(_ir4, _ir7);
	
	return _mm256_movemask_epi8(_ir8);	
}

FORCEINLINE static uint32_t
_find_field_val_avx2_32(__m256i b0)
{
	__m256i _rr0 = _mm256_set1_epi8(0x00 - 1);
	__m256i _rr1 = _mm256_set1_epi8(0x20);
	__m256i _rr2 = _mm256_set1_epi8(0x09);
	__m256i _rr3 = _mm256_set1_epi8(0x7F);
	
	__m256i _ir0 = _mm256_and_si256(_mm256_cmpgt_epi8(b0, _rr0), _mm256_cmpgt_epi8(_rr1, b0));
	__m256i _ir1 = _mm256_cmpeq_epi8(b0, _rr2);
	__m256i _ir2 = _mm256_cmpeq_epi8(b0, _rr3);
	__m256i _ir3 = _mm256_or_si256(_ir0, _ir2);
	__m256i _ir4 = _mm256_andnot_si256(_ir1, _ir3);
	return _mm256_movemask_epi8(_ir4);
}

FORCEINLINE static uint64_t
_find_field_val_avx2_64(__m256i b0, __m256i b1)
{
	__m256i _rr0 = _mm256_set1_epi8(0x00 - 1);
	__m256i _rr1 = _mm256_set1_epi8(0x20);
	__m256i _rr2 = _mm256_set1_epi8(0x09);
	__m256i _rr3 = _mm256_set1_epi8(0x7F);
	
	__m256i _ir0 = _mm256_and_si256(_mm256_cmpgt_epi8(b0, _rr0), _mm256_cmpgt_epi8(_rr1, b0));
	__m256i _ir1 = _mm256_cmpeq_epi8(b0, _rr2);
	__m256i _ir2 = _mm256_cmpeq_epi8(b0, _rr3);
	__m256i _ir3 = _mm256_or_si256(_ir0, _ir2);
	__m256i _ir4 = _mm256_andnot_si256(_ir1, _ir3);
	
	uint64_t r1 =  _mm256_movemask_epi8(_ir4);
	
	_ir0 = _mm256_and_si256(_mm256_cmpgt_epi8(b1, _rr0), _mm256_cmpgt_epi8(_rr1, b1));
	_ir1 = _mm256_cmpeq_epi8(b1, _rr2);
	_ir2 = _mm256_cmpeq_epi8(b1, _rr3);
	_ir3 = _mm256_or_si256(_ir0, _ir2);
	_ir4 = _mm256_andnot_si256(_ir1, _ir3);
	
	uint64_t r2 =  _mm256_movemask_epi8(_ir4);
	
	return (r2 << 32) | r1;	
}

FORCEINLINE static void
_find_field_val_avx2_128(__m256i b0, __m256i b1, __m256i b2, __m256i b3, uint64_t *r0, uint64_t *r1)
{
	__m256i _rr0 = _mm256_set1_epi8(0x00 - 1);
	__m256i _rr1 = _mm256_set1_epi8(0x20);
	__m256i _rr2 = _mm256_set1_epi8(0x09);
	__m256i _rr3 = _mm256_set1_epi8(0x7F);
	
	__m256i _ir0 = _mm256_and_si256(_mm256_cmpgt_epi8(b0, _rr0), _mm256_cmpgt_epi8(_rr1, b0));
	__m256i _ir1 = _mm256_cmpeq_epi8(b0, _rr2);
	__m256i _ir2 = _mm256_cmpeq_epi8(b0, _rr3);
	__m256i _ir3 = _mm256_or_si256(_ir0, _ir2);
	__m256i _ir4 = _mm256_andnot_si256(_ir1, _ir3);
	
	uint64_t _r0 =  _mm256_movemask_epi8(_ir4);
	
	_ir0 = _mm256_and_si256(_mm256_cmpgt_epi8(b1, _rr0), _mm256_cmpgt_epi8(_rr1, b1));
	_ir1 = _mm256_cmpeq_epi8(b1, _rr2);
	_ir2 = _mm256_cmpeq_epi8(b1, _rr3);
	_ir3 = _mm256_or_si256(_ir0, _ir2);
	_ir4 = _mm256_andnot_si256(_ir1, _ir3);
	
	uint64_t _r1 =  _mm256_movemask_epi8(_ir4);

	_ir0 = _mm256_and_si256(_mm256_cmpgt_epi8(b2, _rr0), _mm256_cmpgt_epi8(_rr1, b2));
	_ir1 = _mm256_cmpeq_epi8(b2, _rr2);
	_ir2 = _mm256_cmpeq_epi8(b2, _rr3);
	_ir3 = _mm256_or_si256(_ir0, _ir2);
	_ir4 = _mm256_andnot_si256(_ir1, _ir3);
	
	uint64_t _r2 =  _mm256_movemask_epi8(_ir4);

	_ir0 = _mm256_and_si256(_mm256_cmpgt_epi8(b3, _rr0), _mm256_cmpgt_epi8(_rr1, b3));
	_ir1 = _mm256_cmpeq_epi8(b3, _rr2);
	_ir2 = _mm256_cmpeq_epi8(b3, _rr3);
	_ir3 = _mm256_or_si256(_ir0, _ir2);
	_ir4 = _mm256_andnot_si256(_ir1, _ir3);
	
	uint64_t _r3 =  _mm256_movemask_epi8(_ir4);

	*r0 = (_r1 << 32) | _r0;
	*r1 = (_r3 << 32) | _r2;
}

FORCEINLINE static size_t
_method_index_avx2_32(const char *buf)
{
	__m256i b0 = _mm256_loadu_si256((void*)buf);
	uint32_t bitmap = _find_method_avx2_32(b0);
	return _tzcnt(bitmap);
}

FORCEINLINE static size_t
_target_index_avx2_32(const char *buf)
{
	__m256i b0 = _mm256_loadu_si256((void*)buf);
	uint32_t bitmap = _find_target_avx2_32(b0);
	return _tzcnt(bitmap);	
}

FORCEINLINE static size_t
_target_index_avx2_64(const char *buf)
{
	__m256i b0 = _mm256_loadu_si256((void*)buf);
	__m256i b1 = _mm256_loadu_si256((void*)(buf + 32));	
	uint64_t bitmap = _find_target_avx2_64(b0, b1);
	return _tzcnt(bitmap);
}

FORCEINLINE static size_t
_match_httpver_10_avx2(const char *buf)
{
	__m256i _b0 = _mm256_loadu_si256((void*)buf);
	__m256i _b1 = _mm256_set_epi8(0, 0, 0, 0, 0, 0, 0, 0,
				      0, 0, 0, 0, 0, 0, 0, 0,
				      0, 0, 0, 0, 0, 0, 0, 0,
				      '0', '.', '1', '/', 'P', 'T', 'T', 'H');
	return _match_httpver_avx2(_b0, _b1);
}

FORCEINLINE static size_t
_match_httpver_11_avx2(const char *buf)
{
	__m256i _b0 = _mm256_loadu_si256((void*)buf);
	__m256i _b1 = _mm256_set_epi8(0, 0, 0, 0, 0, 0, 0, 0,
				      0, 0, 0, 0, 0, 0, 0, 0,
				      0, 0, 0, 0, 0, 0, 0, 0,
				      '1', '.', '1', '/', 'P', 'T', 'T', 'H');
	return _match_httpver_avx2(_b0, _b1);
}

FORCEINLINE static size_t
_field_name_index_avx2_32(const char *buf)
{
	__m256i b0 = _mm256_loadu_si256((void*)buf);
	uint32_t bitmap = _find_field_name_avx2_32(b0);
	return _tzcnt(bitmap);
}

FORCEINLINE static size_t
_field_value_index_avx2_32(const char *buf)
{
	__m256i b0 = _mm256_loadu_si256((void*)buf);
	uint32_t bitmap = _find_field_val_avx2_32(b0);
	return _tzcnt(bitmap);
}

FORCEINLINE static size_t
_field_value_index_avx2_64(const char *buf)
{
	__m256i b0 = _mm256_loadu_si256((void*)buf);
	__m256i b1 = _mm256_loadu_si256((void*)(buf + 32));
	uint64_t bitmap = _find_field_val_avx2_64(b0, b1);
	return _tzcnt(bitmap);
}

FORCEINLINE static size_t
_field_value_index_avx2_128(const char *buf)
{
	__m256i b0 = _mm256_loadu_si256((void*)buf);
	__m256i b1 = _mm256_loadu_si256((void*)(buf + 32));
	__m256i b2 = _mm256_loadu_si256((void*)(buf + 64));
	__m256i b3 = _mm256_loadu_si256((void*)(buf + 96));
	uint64_t r0, r1;
	_find_field_val_avx2_128(b0, b1, b2, b3, &r0, &r1);

	uint64_t s0 = _tzcnt(r0), s1 = _tzcnt(r1);
	return s0 == 64 ? s1 + 64 : s0;
}

#endif	/* USE_AVX2_ */

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
#ifdef USE_AVX2_
			if (input_size >= 32) {
				size_t idx = _method_index_avx2_32(p);
				if (unlikely(idx >= 32)) {
					goto BAD_REQUEST;
				}
				p += idx;

				if (unlikely(*p != ' '))
					goto BAD_REQUEST;

				l->method = m;
				l->method_size = idx;
				parser->state = S_BEFORE_TARGET;
				break;
			} else
#endif	/* USE_AVX2_ */				
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
#ifdef USE_AVX2_
			if (end - p >= 64) {
				size_t idx = _target_index_avx2_64(p);
				if (unlikely(idx >= 64)) {
					p += 63;
					break;
				}
				
				p += idx;
				if (unlikely(*p != ' '))
					goto BAD_REQUEST;

				l->target = m;
				l->target_size = p - m;
				parser->state = S_BEFORE_HTTPVER;
				break;
			} else if (end - p >= 32) {
				size_t idx = _target_index_avx2_32(p);
				if (unlikely(idx >= 32)) {
					p += 31;
					break;
				}

				p += idx;
				if (unlikely(*p != ' '))
					goto BAD_REQUEST;

				l->target = m;
				l->target_size = p - m;
				parser->state = S_BEFORE_HTTPVER;
				break;
			} else
#endif	/* USE_AVX2_ */
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
#ifdef USE_AVX2_
			if (end - p > 32) {
				if (_match_httpver_10_avx2(p)) {
					l->httpver_major = 1;
					l->httpver_minor = 0;
					p += 7;
					parser->state = S_CR;
					break;
				} else if (_match_httpver_11_avx2(p)) {
					l->httpver_major = 1;
					l->httpver_minor = 1;
					p += 7;
					parser->state = S_CR;
					break;
				} else
					goto BAD_REQUEST;
			} else
#endif	/* USE_AVX2_ */
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
#ifdef USE_AVX2_
			if (end - p > 32) {
				size_t idx = _field_name_index_avx2_32(p);
				if (unlikely(idx >= 32)) {
					p += 31;
					break;
				}

				p += idx;
				if (unlikely(*p != ':'))
					goto BAD_REQUEST;

				ns = p - nm;
				if (unlikely(ns == 0))
					goto BAD_REQUEST;

				parser->state = S_BEFORE_VALUE;
			} else
#endif	/* USE_AVX2_ */
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
#ifdef USE_AVX2_
			if (end - p > 128) {
				size_t idx = _field_value_index_avx2_128(p);
				if (unlikely(idx >= 128)) {
					p += 127;
					break;
				}

				p += idx;
				
				if (unlikely(*p != '\r'))
					goto BAD_REQUEST;
				parser->state = S_CR;
				break;
			} else if (end - p > 64) {
			/* if (end - p > 64) { */
				size_t idx = _field_value_index_avx2_64(p);
				if (unlikely(idx >= 64)) {
					p += 63;
					break;
				}

				p += idx;
				
				if (unlikely(*p != '\r'))
					goto BAD_REQUEST;

				parser->state = S_CR;
				break;
			} else if (end - p > 32) {
				size_t idx = _field_value_index_avx2_32(p);
				if (idx >= 32) {
					p += 63;
					break;
				}

				p += idx;

				if (unlikely(*p != '\r'))
					goto BAD_REQUEST;

				parser->state = S_CR;
				break;
			} else
#endif	/* USE_AVX2_ */
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
