#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, malloc(), realloc(), free(), strtod() */
#include <string.h>  /* memcpy() */

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
#define PUTC(_c, _ch) do { *(char *)lept_context_push(_c, sizeof(char)) = (_ch); } while(0)


// 由于stack会扩展，top不可以使用指针形式，因为扩展后指针地址存储数据可能不正确
typedef struct {
    const char* json;
    char *stack;        // 动态堆栈
    size_t size, top;
} lept_context;


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

static int lept_parse_string(lept_context *c, lept_value *v) {
    size_t head = c->top, len;      // 备份栈顶，用于计算字符串长度
    const char *p;
    EXPECT(c, '\"');
    p = c->json;
    for(;;) {
        char ch = *p++;
        switch(ch) {
            case '\"':
                len = c->top - head;
                lept_set_string(v, (const char*)lept_context_pop(c, len), len);
                c->json = p;        // 这里不明白
            return LEPT_PARSE_OK;
			case '\\':      // 转义字符
				switch (*p++) {
					case '\"': PUTC(c, '\"'); break;
					case '\\': PUTC(c, '\\'); break;
					case '/': PUTC(c, '/'); break;
					case 'b': PUTC(c, '\b'); break;
					case 'f': PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                    default:
                        c->top = head;
                        return LEPT_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            case '\0':
                c->top = head;
                return LEPT_PARSE_MISS_QUOTATION_MARK;
            default:
                if((unsigned char) ch < 0x20) {     // 无效字符
                    c->top = head;
                    return LEPT_PARSE_INVALID_STRING_CHAR;
                }
                PUTC(c, ch);
        }
    }
}


static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
		case 't':
            return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':
            return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':
            return lept_parse_literal(c, v, "null", LEPT_NULL);
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;
        case '"':
            return lept_parse_string(c, v);
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

void lept_free(lept_value *v) {
    assert(v != NULL);
    if(v->type == LEPT_STRING)
        free(v->u.s.s);
    v->type = LEPT_NULL;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

int lept_get_boolean(const lept_value *v) {
    assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
    return v->type == LEPT_TRUE;        // v->type类型是布尔类型，返回0是False，1是True，和我想的有点出路。
}

void lept_set_boolean(lept_value *v, int b) {
    lept_free(v);
    v->type = b ? LEPT_TRUE : LEPT_FALSE;
}

double lept_get_number(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}

void lept_set_number(lept_value *v, double n) {
    lept_free(v);
    v->u.n = n;
    v->type = LEPT_NUMBER;
}

const char* lept_get_string(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.s;
}

size_t lept_get_string_length(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.len;
}

void lept_set_string(lept_value* v, const char* s, size_t len) {
    assert(v != NULL && (s != NULL || len == 0));
    lept_free(v);
    v->u.s.s = (char *) malloc (len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = LEPT_STRING;
}