#include <stdio.h>
#include <assert.h>
#include "leptjson.h"

#define EXPECT(_c, _ch) \
do { \
assert(*_c->json == _ch); \
_c->json++; \
} while(0)

// 减少解析函数之间传递多个参数
typedef struct {
    const char* json;
} lept_context;

static void lept_parse_whitespace(lept_context *c) {
    const char *p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\t')
    ++p;
    c->json = p;
}

static int lept_parse_null(lept_context* c, lept_value *v) {
    EXPECT(c, 'n');
    if(c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value *v) {
    EXPECT(c, 'f');
    if(c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_context* c, lept_value *v) {
    EXPECT(c, 't');
    if(c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch(*c->json) {
        case 'n':
            return lept_parse_null(c, v);
        case 'f':
            return lept_parse_false(c, v);
        case 't':
            return lept_parse_true(c, v);
        default:
            return LEPT_PARSE_INVALID_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    int ret = lept_parse_value(&c, v);
    const char *p = c.json;
    lept_parse_whitespace(&c);
    if(p != c.json)
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    else
        return ret;
}

lept_type lept_get_type(const lept_value* v) {
    return v->type;
}