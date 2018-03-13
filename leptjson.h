#ifndef LEPTJSON_H__
#define LEPTJSON_H__

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

// json是树状结构，lept_value表示各个节点
// lept_value事实上是一种变体类型(variant type)：通过type类型决定那些成员是有效的
typedef struct {
    union {
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
    LEPT_PARSE_NUMBER_TOO_BIG
};

// API函数

int lept_parse(lept_value *v, const char *json);
lept_type lept_get_type(const lept_value *v);
double lept_get_number(const lept_value *v);
double lept_set_number(lept_value *v, double n);
int lept_get_boolean(const lept_value *v);
void lept_set_boolean(lept_value *v, int b);
const char* lept_get_string(const lept_value *v);
void lept_set_string(lept_value *v, const char *s, size_t len);
size_t lept_get_string_length(const lept_value *v);

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

#endif  /* LEPTJSON_H__ */