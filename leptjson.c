#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, strtod() */

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

#define EXPECT(_c, _ch) \
do { \
assert(*_c->json == _ch); \
_c->json++; \
} while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define PUTC(_c, _ch) do { *(char *)lept_context_push(_c, size_of(char)) = (_ch); } while(0)

#define lept_init(v) do {(v)->type = LEPT_NULL;} while(0)
#define lept_set_null(v) lept_free(v)

// 由于stack会扩展，top不可以使用指针形式，因为扩展后指针地址存储数据可能不正确
typedef struct {
    const char* json;
    char *stack;        // 动态堆栈
    size_t size, top;
} lept_context;

static void lept_parse_whitespace(lept_context *c) {
    const char *p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
    ++p;
    c->json = p;
}

static int lept_parse_literal(lept_context *c, lept_value *v, const char *literal, lept_type type) {
    size_t i;
    EXPECT(c, literal[0]);
    // for(i = 0; literal[i + 1] != '\0'; i++)        // 使用literal本身判断
    for (i = 0; literal[i + 1]; i++)
        if (c->json[i] != literal[i + 1])
            return LEPT_PARSE_INVALID_VALUE;
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context *c, lept_value *v) {
    const char *p = c->json;
    /* 负号 */
    if(*p == '-') ++p;
    /* 整数 */
    if(*p == '0') ++p;
    else {
        if(!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        // ++p
        // while(ISDIGIT(*p))
        // ++p
        for(++p; ISDIGIT(*p); ++p);     // 相比while形式，可以合并成一行
    }
    /* 小数 */
    if(*p == '.') {
        ++p;
        if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for(++p; ISDIGIT(*p); ++p);
    }
    /* 指数 */
    if(*p == 'e' || *p == 'E') {
        ++p;
        if(*p == '-' || *p == '+') ++p;
        if(!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for(++p; ISDIGIT(*p); ++p);
    }
    errno = 0;
    v->u.n = strtod(c->json, NULL);       // 前面部分只是验证是否符合JSON语法，最后还是用strtod解析
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch(*c->json) {
        case 'n':
            return lept_parse_literal(c, v, "null", LEPT_NULL);
        case 'f':
            return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 't':
            return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;
        default:
            return lept_parse_number(c, v);
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    lept_init(v);
    lept_parse_whitespace(&c);
    if((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
        	ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
	}
    assert(c.top == 0);     // 确保所有数据都被弹出
    free(c.stack);
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}

void lept_free(lept_value *v) {
    assert(v != NULL);
    if(v->type == LEPT_STRING)
        free(v->u.s.s);
    v->type = LEPT_NULL;
}

void letp_set_string(lept_value *v, const char *s, size_t len) {
    assert(v != NULL && (s != NULL || len == 0));
    lept_free(v);
    v->u.s.s = (char *) malloc (len + 1);
    memccpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = LEPT_STRING;
}

static void* lept_context_push(lept_context *c, size_t size) {
    void *ret;
    assert(size > 0);
    if(c->top + size >= c->size) {
        if(c->size == 0)        // 初次c->stack == NULL
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        while(c->top + size >= c->size)
            c->size += c->size >> 1;        // c->size = c->size/2 + c->size
        c->stack = (char *)realloc(c->stack, c->size);      // realloc(NULL, size)等价malloc(size)
    }
    ret = c->stack + c->top;        // 数据存储起始位置
    c->top += size;     // 栈顶位置
    return ret;
}

static void* lept_context_pop(lept_context *c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}