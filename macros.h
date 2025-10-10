#define MAX2(A, B) ({ \
    __typeof__ (A) _a = (A); \
    __typeof__ (B) _b = (B); \
    _a > _b ? _a : _b; \
})

#define MIN2(A, B) ({ \
    __typeof__ (A) _a = (A); \
    __typeof__ (B) _b = (B); \
    _a < _b ? _a : _b; \
}

#define $(EXPR) ({ \
    __typeof__ (EXPR) _eval_result = (EXPR); \
    if (*error != NULL) return result; \
    _eval_result; \
})

