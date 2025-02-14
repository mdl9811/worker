// This file is designed to be included by C/C++ files which need the contents
// of the HPACK static table. It may be included more than once if necessary.
// See http://httpwg.org/specs/rfc7541.html#static.table.definition

STATIC_TABLE_ENTRY(":authority", "", 1);
STATIC_TABLE_ENTRY(":method", "GET", 2);
STATIC_TABLE_ENTRY(":method", "POST", 3);
STATIC_TABLE_ENTRY(":path", "/", 4);
STATIC_TABLE_ENTRY(":path", "/index.html", 5);
STATIC_TABLE_ENTRY(":scheme", "http", 6);
STATIC_TABLE_ENTRY(":scheme", "https", 7);
STATIC_TABLE_ENTRY(":status", "200", 8);
STATIC_TABLE_ENTRY(":status", "204", 9);
STATIC_TABLE_ENTRY(":status", "206", 10);
STATIC_TABLE_ENTRY(":status", "304", 11);
STATIC_TABLE_ENTRY(":status", "400", 12);
STATIC_TABLE_ENTRY(":status", "404", 13);
STATIC_TABLE_ENTRY(":status", "500", 14);
STATIC_TABLE_ENTRY("accept-charset", "", 15);
STATIC_TABLE_ENTRY("accept-encoding", "gzip, deflate", 16);
STATIC_TABLE_ENTRY("accept-language", "", 17);
STATIC_TABLE_ENTRY("accept-ranges", "", 18);
STATIC_TABLE_ENTRY("accept", "", 19);
STATIC_TABLE_ENTRY("access-control-allow-origin", "", 20);
STATIC_TABLE_ENTRY("age", "", 21);
STATIC_TABLE_ENTRY("allow", "", 22);
STATIC_TABLE_ENTRY("authorization", "", 23);
STATIC_TABLE_ENTRY("cache-control", "", 24);
STATIC_TABLE_ENTRY("content-disposition", "", 25);
STATIC_TABLE_ENTRY("content-encoding", "", 26);
STATIC_TABLE_ENTRY("content-language", "", 27);
STATIC_TABLE_ENTRY("content-length", "", 28);
STATIC_TABLE_ENTRY("content-location", "", 29);
STATIC_TABLE_ENTRY("content-range", "", 30);
STATIC_TABLE_ENTRY("content-type", "", 31);
STATIC_TABLE_ENTRY("cookie", "", 32);
STATIC_TABLE_ENTRY("date", "", 33);
STATIC_TABLE_ENTRY("etag", "", 34);
STATIC_TABLE_ENTRY("expect", "", 35);
STATIC_TABLE_ENTRY("expires", "", 36);
STATIC_TABLE_ENTRY("from", "", 37);
STATIC_TABLE_ENTRY("host", "", 38);
STATIC_TABLE_ENTRY("if-match", "", 39);
STATIC_TABLE_ENTRY("if-modified-since", "", 40);
STATIC_TABLE_ENTRY("if-none-match", "", 41);
STATIC_TABLE_ENTRY("if-range", "", 42);
STATIC_TABLE_ENTRY("if-unmodified-since", "", 43);
STATIC_TABLE_ENTRY("last-modified", "", 44);
STATIC_TABLE_ENTRY("link", "", 45);
STATIC_TABLE_ENTRY("location", "", 46);
STATIC_TABLE_ENTRY("max-forwards", "", 47);
STATIC_TABLE_ENTRY("proxy-authenticate", "", 48);
STATIC_TABLE_ENTRY("proxy-authorization", "", 49);
STATIC_TABLE_ENTRY("range", "", 50);
STATIC_TABLE_ENTRY("referer", "", 51);
STATIC_TABLE_ENTRY("refresh", "", 52);
STATIC_TABLE_ENTRY("retry-after", "", 53);
STATIC_TABLE_ENTRY("server", "", 54);
STATIC_TABLE_ENTRY("set-cookie", "", 55);
STATIC_TABLE_ENTRY("strict-transport-security", "", 56);
STATIC_TABLE_ENTRY("transfer-encoding", "", 57);
STATIC_TABLE_ENTRY("user-agent", "", 58);
STATIC_TABLE_ENTRY("vary", "", 59);
STATIC_TABLE_ENTRY("via", "", 60);
STATIC_TABLE_ENTRY("www-authenticate", "", 61);
