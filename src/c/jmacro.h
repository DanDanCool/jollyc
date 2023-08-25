#pragma once

const int MAGIC = 'j' << 24 | 'o' << 16 | 11 << 8 | 'y'

#define MM256 u32 __attribute__((aligned(256)))

#define PAREN ()

// recursively expand macros up to 324 times https://www.scs.stanford.edu/~dm/blog/va-opt.html
#define EXPAND(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) __VA_ARGS__

// number of arguments in variadic macro
// arguments required, use __VA_OPT__ if unsure if there are macros
#define NARG(...) NARG_(__VA_ARGS__, NARG_RSEQ_N())
#define NARG_(...) NARG_N(__VA_ARGS__)
#define NARG_N( \
_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
_61,_62,_63,N,...) N
#define NARG_RSEQ_N() \
63,62,61,60, \
59,58,57,56,55,54,53,52,51,50, \
49,48,47,46,45,44,43,42,41,40, \
39,38,37,36,35,34,33,32,31,30, \
29,28,27,26,25,24,23,22,21,20, \
19,18,17,16,15,14,13,12,11,10, \
9,8,7,6,5,4,3,2,1,0

#define STR(x) #x
#define CAT(a, b) a##b

#define elif else if

// note: assumes two's complement
#define ABS(x) ((x) & (1 << 31)) ? 1 + ~(x) : (x)

#define INTERFACE(NAME) \
typedef struct interface_##NAME interface_##NAME; \
typedef interface_##NAME* interface(NAME); \
struct interface_##NAME

#define INTERFACE_DECLARE(name, impl) \
extern const interface(name) interface_impl(name, impl)

#define INTERFACE_IMPL(name, impl) \
const interface(name) interface_impl(name, impl) =

#define interface(NAME) v##NAME
#define interface_impl(name, impl) interface(NAME)##_##impl

#define vdispatch(name) vtable->##name

#define DECLARE_FN__() DECLARE_FN_

#define DECLARE_FN_(STRUCT, TYPE, arg, ...) \
STRUCT##_##arg(TYPE); \
__VA_OPT__(DECLARE_FN__ PAREN (NAME, TYPE, __VA_ARGS__))

#define DECLARE_FN(STRUCT, TYPE, ...) \
__VA_OPT__(EXPAND(DECLARE_FN_(STRUCT, TYPE, __VA_ARGS__)))
