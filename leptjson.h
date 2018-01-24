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
typedef struct {
    double n;
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

#endif  /* LEPTJSON_H__ */