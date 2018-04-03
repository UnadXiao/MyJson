#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include <stddef.h> /* size_t */

// 数据结构体

typedef enum { 
    LEPT_NULL,
    LEPT_FALSE,
    LEPT_TRUE,
    LEPT_NUMBER,
    LEPT_STRING,
    LEPT_ARRAY,
    LEPT_OBJECT
} lept_type;

typedef struct lept_value lept_value;       // lept_value使用了自身类型的指针，所以需要向前声明
// json是树状结构，lept_value表示各个节点
// lept_value事实上是一种变体类型(variant type)：通过type类型决定那些成员是有效的
typedef struct lept_value {
    union {
        struct { lept_value *e; size_t size; } a;     // array size元素个数
        struct { char *s; size_t len; } s;       // string
        double n;
    } u;
    lept_type type;
} lept_value;

// 返回值
enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG,
    LEPT_PARSE_MISS_QUOTATION_MARK,
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_STRING_CHAR,
    LEPT_PARSE_INVALID_UNICODE_HEX,
    LEPT_PARSE_INVALID_UNICODE_SURROGATE,
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};

// API函数

#define lept_init(v) do {(v)->type = LEPT_NULL;} while(0)
int lept_parse(lept_value *v, const char *json);
void lept_free(lept_value *v);
lept_type lept_get_type(const lept_value *v);

#define lept_set_null(v) lept_free(v)

int lept_get_boolean(const lept_value *v);
void lept_set_boolean(lept_value *v, int b);
double lept_get_number(const lept_value *v);
void lept_set_number(lept_value *v, double n);
size_t lept_get_array_size(const lept_value *v);
lept_value *lept_get_array_element(const lept_value *v, size_t index);

const char* lept_get_string(const lept_value *v);
size_t lept_get_string_length(const lept_value *v);
void lept_set_string(lept_value *v, const char *s, size_t len);

// JSON语法子集

// JSON-text = ws value ws
// ws = *(%x20 / %x09 / %x0A / %x0D)
// value = null / false / true
// null = "null"
// false = "false"
// true = "true"

// JSON-number = [ "-" ] int [ frac ] [ exp ]
// int = "0" / digit1-9 * digit
// frac = "." 1*digit
// exp = ("e" / "E") ["-" / "+"] 1*digit

// JSON-string = quotation-mark *char quotation-mark
// char = unescaped /
//    escape (
//        %x22 /          ; "    quotation mark  U+0022
//        %x5C /          ; \    reverse solidus U+005C
//        %x2F /          ; /    solidus         U+002F
//        %x62 /          ; b    backspace       U+0008
//        %x66 /          ; f    form feed       U+000C
//        %x6E /          ; n    line feed       U+000A
//        %x72 /          ; r    carriage return U+000D
//        %x74 /          ; t    tab             U+0009
//        %x75 4HEXDIG )  ; uXXXX                U+XXXX
// escape = %x5C          ; \
// quotation-mark = %x22  ; "
// unescaped = %x20-21 / %x23-5B / %x5D-10FFFF

// JSON-array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D
// %x5B ; [
// %x5D ; ]
// %x2C ; ,
// ws = *(%x20 / %x09 / %x0A / %x0D)
// value = JSON-text / JSON-number / JSON-string / JSON-value
// 

#endif  /* LEPTJSON_H__ */