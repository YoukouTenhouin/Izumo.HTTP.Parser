# Izumo.HTTP.Parser
Zero-copy no-memory-alloc HTTP Parser used by [Izumo](https://github.com/AbrahamHokuto/Izumo).

## Features
+ Zero-copy
+ No memory allocation
+ No dependencies
+ Guaranteed to be stateless (i. e. free of local static objects)

## Example (Pseudo-code)
Parsing request line:
```C
struct izm_http_request_line_parser rlp;
struct izm_http_request_line rl;

bzero(&rlp, sizeof(rlp));
bzero(&rl, sizeof(rl));

while (1) {
	bytes_read += recv(client, buf + bytes_read, buf_size - bytes_read);
	int ret = izm_http_parse_request_line(&rlp, &rl, &bytes_parsed, buf, bytes_read);
	if (ret == IZM_PARSER_OK) {
		handle_request_line(rl);
		break ;
	} else if (ret == IZM_PARSER_CONTINUE) {
		continue;
	} else if (ret == IZM_PARSER_BAD_REQUEST) {
		bad_request();
	}
}
```

Parsing status line:
```C
struct izm_http_status_line_parser slp;
struct izm_http_status_line sl;

bzero(&hp, sizeof(slp));
bzero(h, sizeof(sl));

while (1) {
	bytes_read += recv(client, buf + bytes_read, buf_size - bytes_read);
	int ret = izm_http_parse_status_line(&slp, sl, &bytes_parsed, buf, bytes_read);
	headers_count += headers_filled;
	if (ret == IZM_PARSER_OK) {
		handle_status_line(sl);
		break;
	} else if (ret == IZM_PARSER_CONTINUE) {
		continue;
	} else if (ret == IZM_PARSER_BAD_REQUEST) {
		bad_request();
	}
}
```

Parsing headers:
```C
struct izm_http_headers_parser hp;
struct izm_http_header h[32];

bzero(&hp, sizeof(rlp));
bzero(h, sizeof(rl));

while (1) {
	bytes_read += recv(client, buf + bytes_read, buf_size - bytes_read);
	int ret = izm_http_parse_headers(&hp, h + headers_count, 32 - headers_count, &headers_filled, &bytes_parsed, buf + start_line_size, bytes_read - start_line_size);
	headers_count += headers_filled;
	if (ret == IZM_PARSER_OK) {
		handle_headers(h, headers_count);
		break;
	} else if (ret == IZM_PARSER_CONTINUE) {
		continue;
	} else if (ret == IZM_PARSER_BAD_REQUEST) {
		bad_request();
	}
}
```

Detailed information can be found in wiki.