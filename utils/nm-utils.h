//
// Created by wenyiyu on 2024/10/18.
//

#ifndef RASPBERRY_PI_OLED_PANEL_NM_UTILS_H
#define RASPBERRY_PI_OLED_PANEL_NM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif
#include <glib.h>
#include <libnm/NetworkManager.h>
#include "config.h"

#define NM_FLAGS_ANY(a, b) (a & b)

char *
nm_utils_ssid_to_utf8(const guint8 *ssid, gsize len);

static inline int
nm_streq0(const char *s1, const char *s2) {
    return (s1 == s2) || (s1 && s2 && strcmp(s1, s2) == 0);
}

static inline int
nm_streq(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0;
}

static inline int
_NM_IN_STRSET_EVAL_op_streq(const char *x1, const char *x) {
    return x && nm_streq(x1, x);
}

#define NM_PASTE_ARGS(identifier1, identifier2) identifier1##identifier2

#define NM_PASTE(identifier1, identifier2)      NM_PASTE_ARGS(identifier1, identifier2)

#define _NM_MACRO_CALL(macro, ...)  macro(__VA_ARGS__)
#define _NM_MACRO_CALL2(macro, ...) macro(__VA_ARGS__)

#define _NM_IN_STRSET_EVAL_OP_NULL(x, idx, op_arg)  ((int) (((const char *) NULL) == (x)))

#define _NM_IN_STRSET_EVAL_OP_STREQ(x, idx, op_arg) _NM_IN_STRSET_EVAL_op_streq(_x1, x)

/* clang-format off */
#define _NM_VA_ARGS_FOREACH_0(prefix, postfix, sep, op, op_arg)
#define _NM_VA_ARGS_FOREACH_1(prefix, postfix, sep, op, op_arg, x)         prefix _NM_MACRO_CALL2(op, x, 0,   op_arg) postfix
#define _NM_VA_ARGS_FOREACH_2(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 1,   op_arg) postfix sep _NM_VA_ARGS_FOREACH_1(  prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_3(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 2,   op_arg) postfix sep _NM_VA_ARGS_FOREACH_2(  prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_4(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 3,   op_arg) postfix sep _NM_VA_ARGS_FOREACH_3(  prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_5(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 4,   op_arg) postfix sep _NM_VA_ARGS_FOREACH_4(  prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_6(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 5,   op_arg) postfix sep _NM_VA_ARGS_FOREACH_5(  prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_7(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 6,   op_arg) postfix sep _NM_VA_ARGS_FOREACH_6(  prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_8(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 7,   op_arg) postfix sep _NM_VA_ARGS_FOREACH_7(  prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_9(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 8,   op_arg) postfix sep _NM_VA_ARGS_FOREACH_8(  prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_10(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 9,   op_arg) postfix sep _NM_VA_ARGS_FOREACH_9(  prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_11(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 10,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_10( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_12(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 11,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_11( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_13(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 12,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_12( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_14(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 13,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_13( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_15(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 14,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_14( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_16(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 15,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_15( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_17(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 16,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_16( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_18(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 17,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_17( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_19(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 18,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_18( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_20(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 19,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_19( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_21(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 20,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_20( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_22(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 21,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_21( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_23(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 22,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_22( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_24(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 23,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_23( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_25(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 24,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_24( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_26(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 25,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_25( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_27(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 26,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_26( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_28(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 27,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_27( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_29(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 28,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_28( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_30(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 29,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_29( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_31(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 30,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_30( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_32(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 31,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_31( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_33(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 32,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_32( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_34(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 33,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_33( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_35(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 34,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_34( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_36(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 35,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_35( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_37(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 36,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_36( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_38(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 37,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_37( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_39(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 38,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_38( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_40(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 39,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_39( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_41(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 40,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_40( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_42(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 41,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_41( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_43(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 42,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_42( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_44(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 43,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_43( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_45(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 44,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_44( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_46(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 45,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_45( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_47(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 46,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_46( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_48(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 47,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_47( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_49(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 48,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_48( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_50(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 49,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_49( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_51(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 50,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_50( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_52(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 51,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_51( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_53(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 52,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_52( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_54(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 53,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_53( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_55(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 54,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_54( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_56(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 55,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_55( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_57(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 56,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_56( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_58(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 57,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_57( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_59(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 58,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_58( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_60(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 59,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_59( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_61(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 10,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_60( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_62(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 61,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_61( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_63(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 62,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_62( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_64(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 63,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_63( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_65(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 64,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_64( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_66(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 65,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_65( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_67(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 66,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_66( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_68(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 67,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_67( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_69(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 68,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_68( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_70(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 69,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_69( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_71(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 70,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_70( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_72(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 71,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_71( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_73(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 72,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_72( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_74(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 73,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_73( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_75(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 74,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_74( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_76(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 75,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_75( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_77(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 76,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_76( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_78(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 77,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_77( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_79(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 78,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_78( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_80(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 79,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_79( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_81(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 80,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_80( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_82(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 81,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_81( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_83(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 82,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_82( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_84(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 83,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_83( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_85(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 84,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_84( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_86(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 85,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_85( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_87(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 86,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_86( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_88(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 87,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_87( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_89(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 88,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_88( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_90(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 89,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_89( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_91(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 90,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_90( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_92(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 91,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_91( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_93(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 92,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_92( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_94(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 93,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_93( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_95(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 94,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_94( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_96(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 95,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_95( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_97(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 96,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_96( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_98(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 97,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_97( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_99(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 98,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_98( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_100(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 99,  op_arg) postfix sep _NM_VA_ARGS_FOREACH_99( prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_101(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 100, op_arg) postfix sep _NM_VA_ARGS_FOREACH_100(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_102(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 101, op_arg) postfix sep _NM_VA_ARGS_FOREACH_101(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_103(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 102, op_arg) postfix sep _NM_VA_ARGS_FOREACH_102(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_104(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 103, op_arg) postfix sep _NM_VA_ARGS_FOREACH_103(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_105(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 104, op_arg) postfix sep _NM_VA_ARGS_FOREACH_104(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_106(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 105, op_arg) postfix sep _NM_VA_ARGS_FOREACH_105(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_107(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 106, op_arg) postfix sep _NM_VA_ARGS_FOREACH_106(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_108(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 107, op_arg) postfix sep _NM_VA_ARGS_FOREACH_107(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_109(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 108, op_arg) postfix sep _NM_VA_ARGS_FOREACH_108(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_110(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 109, op_arg) postfix sep _NM_VA_ARGS_FOREACH_109(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_111(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 100, op_arg) postfix sep _NM_VA_ARGS_FOREACH_110(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_112(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 111, op_arg) postfix sep _NM_VA_ARGS_FOREACH_111(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_113(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 112, op_arg) postfix sep _NM_VA_ARGS_FOREACH_112(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_114(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 113, op_arg) postfix sep _NM_VA_ARGS_FOREACH_113(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_115(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 114, op_arg) postfix sep _NM_VA_ARGS_FOREACH_114(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_116(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 115, op_arg) postfix sep _NM_VA_ARGS_FOREACH_115(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_117(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 116, op_arg) postfix sep _NM_VA_ARGS_FOREACH_116(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_118(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 117, op_arg) postfix sep _NM_VA_ARGS_FOREACH_117(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_119(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 118, op_arg) postfix sep _NM_VA_ARGS_FOREACH_118(prefix, postfix, sep, op, op_arg, __VA_ARGS__)
#define _NM_VA_ARGS_FOREACH_120(prefix, postfix, sep, op, op_arg, x, ...)    prefix _NM_MACRO_CALL2(op, x, 119, op_arg) postfix sep _NM_VA_ARGS_FOREACH_119(prefix, postfix, sep, op, op_arg, __VA_ARGS__)

/* clang-format off */
#define _NM_MACRO_SELECT_ARG_120(_empty, \
                                 _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
                                 _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
                                 _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
                                 _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
                                 _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
                                 _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
                                 _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, \
                                 _71, _72, _73, _74, _75, _76, _77, _78, _79, _80, \
                                 _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, \
                                 _91, _92, _93, _94, _95, _96, _97, _98, _99, _100, \
                                 _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, \
                                 _111, _112, _113, _114, _115, _116, _117, _118, _119, _120, \
                                 N, \
                                 ...)    \
    N

#define NM_NARG(...)                        \
    _NM_MACRO_SELECT_ARG_120(, ##__VA_ARGS__, \
                             120, \
                             119, 118, 117, 116, 115, 114, 113, 112, 111, 110, \
                             109, 108, 107, 106, 105, 104, 103, 102, 101, 100, \
                             99,  98,  97,  96,  95,  94,  93,  92,  91,  90,  \
                             89,  88,  87,  86,  85,  84,  83,  82,  81,  80,  \
                             79,  78,  77,  76,  75,  74,  73,  72,  71,  70,  \
                             69,  68,  67,  66,  65,  64,  63,  62,  61,  60,  \
                             59,  58,  57,  56,  55,  54,  53,  52,  51,  50,  \
                             49,  48,  47,  46,  45,  44,  43,  42,  41,  40,  \
                             39,  38,  37,  36,  35,  34,  33,  32,  31,  30,  \
                             29,  28,  27,  26,  25,  24,  23,  22,  21,  20,  \
                             19,  18,  17,  16,  15,  14,  13,  12,  11,  10,  \
                             9,   8,   7,   6,   5,   4,   3,   2,   1,   0)

/* clang-format on */
#define NM_VA_ARGS_FOREACH(prefix, postfix, sep, op, op_arg, ...)        \
    _NM_MACRO_CALL(NM_PASTE(_NM_VA_ARGS_FOREACH_, NM_NARG(__VA_ARGS__)), \
                   prefix,                                               \
                   postfix,                                              \
                   sep,                                                  \
                   op,                                                   \
                   op_arg,                                               \
                   ##__VA_ARGS__)

#define _NM_IN_STRSET_EVAL(op, eval_op, x1, ...)                                           \
    ({                                                                                     \
        const char *const _x1 = (x1);                                                      \
                                                                                           \
        !!(_x1 ? (NM_VA_ARGS_FOREACH(, , op, eval_op, , __VA_ARGS__))                      \
               : (NM_VA_ARGS_FOREACH(, , op, _NM_IN_STRSET_EVAL_OP_NULL, , __VA_ARGS__))); \
    })

/* Beware that this does short-circuit evaluation (use "||" instead of "|")
 * which has a possibly unexpected non-function-like behavior.
 * Use NM_IN_STRSET_SE if you need all arguments to be evaluated. */
#define NM_IN_STRSET(x1, ...) _NM_IN_STRSET_EVAL(||, _NM_IN_STRSET_EVAL_OP_STREQ, x1, __VA_ARGS__)

#define NM_UTILS_LOOKUP_DEFAULT(v)      return (v)

#define NM_UTILS_LOOKUP_ITEM(v, n) \
    case v:                        \
        return (n);

#define NM_UTILS_LOOKUP_ITEM_IGNORE_OTHER() \
    default:                                \
        break;

const char *nmc_device_reason_to_string(NMDeviceStateReason reason);

const char *nmc_device_state_to_string(NMDeviceState state);

#define _(str) str
#define N_(str) str

#define NM_FLAGS_HAS(a, b) (a&b)

#define nm_assert(a)

char *
nmc_unique_connection_name(const GPtrArray *connections, const char *try_name);

/* NM_CACHED_QUARK() returns the GQuark for @string, but caches
 * it in a static variable to speed up future lookups.
 *
 * @string must be a string literal.
 */
#define NM_CACHED_QUARK(string)                                                \
    ({                                                                         \
        static GQuark _nm_cached_quark = 0;                                    \
                                                                               \
        (G_LIKELY(_nm_cached_quark != 0)                                       \
             ? _nm_cached_quark                                                \
             : (_nm_cached_quark = g_quark_from_static_string("" string ""))); \
    })

/* NM_CACHED_QUARK_FCN() is essentially the same as G_DEFINE_QUARK
 * with two differences:
 * - @string must be a quoted string-literal
 * - @fcn must be the full function name, while G_DEFINE_QUARK() appends
 *   "_quark" to the function name.
 * Both properties of G_DEFINE_QUARK() are non favorable, because you can no
 * longer grep for string/fcn -- unless you are aware that you are searching
 * for G_DEFINE_QUARK() and omit quotes / append _quark(). With NM_CACHED_QUARK_FCN(),
 * ctags/cscope can locate the use of @fcn (though it doesn't recognize that
 * NM_CACHED_QUARK_FCN() defines it).
 */
#define NM_CACHED_QUARK_FCN(string, fcn) \
    GQuark fcn(void)                     \
    {                                    \
        return NM_CACHED_QUARK(string);  \
    }


/* Error quark for GError domain */
#define NMCLI_ERROR (nmcli_error_quark())

GQuark nmcli_error_quark(void);

guint64
nm_random_u64_range_full(guint64 begin, guint64 end, gboolean crypto_bytes);

#if defined(__clang__)
/* Clang can emit -Wunused-but-set-variable warning for cleanup variables
 * that are only assigned (never used otherwise). Hack around */
#define _nm_auto_extra _nm_unused
#else
#define _nm_auto_extra
#endif

/**
 * nm_close_with_error:
 *
 * Wrapper around close().
 *
 * This fails an nm_assert() for EBADF with a non-negative file descriptor. Trying
 * to close an invalid file descriptor is always a serious bug. Never use close()
 * directly, because we want to catch such bugs.
 *
 * This also suppresses any EINTR and pretends success. That is appropriate
 * on Linux (but not necessarily on other POSIX systems).
 *
 * In no case is it appropriate to use @fd afterwards (or retry).
 *
 * This function returns 0 on success, or a negative errno value.
 * On success, errno is undefined afterwards. On failure, errno is
 * the same as the (negative) return value.
 *
 * In the common case, when you don't intend to handle the error from close(),
 * prefer nm_close() over nm_close_with_error(). Never use close() directly.
 *
 * The function is also async-signal-safe (unless an assertion fails).
 *
 * Returns: 0 on success or the negative errno from close().
 */
static inline int
nm_close_with_error(int fd) {
    int r;

    r = close(fd);

    if (r != 0) {
        int errsv = errno;

        nm_assert(r == -1);

        /* EBADF indicates a bug.
         *
         * - if fd is non-negative, this means the tracking of the descriptor
         *   got messed up. That's very bad, somebody closed a wrong FD or we
         *   might do so. On a multi threaded application, messing up the tracking
         *   of the file descriptor means we race against closing an unrelated FD.
         * - if fd is negative, it may not be a bug but intentional. However, our callers
         *   are not supposed to call close() on a negative FD either. Assert
         *   against that too. */
        nm_assert(errsv != EBADF);

        if (errsv == EINTR) {
            /* There isn't really much we can do about EINTR. On Linux, always this means
             * the FD was closed. On some POSIX systems that may be different and retry
             * would be appropriate.
             *
             * Whether there was any IO error is unknown. Assume not and signal success. */
            return 0;
        }

        return -errsv;
    }

    return 0;
}

static inline void
_nm_auto_protect_errno(const int *p_saved_errno) {
    errno = *p_saved_errno;
}

#define _nm_unused             __attribute__((__unused__))

#define NM_AUTO_PROTECT_ERRNO(errsv_saved) \
    nm_auto(_nm_auto_protect_errno) _nm_unused const int errsv_saved = (errno)


/**
 * nm_close:
 *
 * Wrapper around nm_close_with_error(), which ignores any error and preserves the
 * caller's errno.
 *
 * We usually don't care about errors from close, so this is usually preferable over
 * nm_close_with_error(). Never use close() directly.
 *
 * Everything from nm_close_with_error() applies.
 */
static inline void nm_close(int fd);

static inline void _nm_auto_close(int *pfd);

#define nm_auto_close nm_auto(_nm_auto_close)

#define nm_auto(fcn) _nm_auto_extra __attribute__((__cleanup__(fcn)))

#define nm_auto_close nm_auto(_nm_auto_close)

int
nm_utils_fd_wait_for_event(int fd, int event, gint64 timeout_nsec);

#define NM_UTILS_NSEC_PER_SEC  ((gint64) 1000000000)

#define NM_ERRNO_NATIVE(errsv)        \
    ({                                \
        const int _errsv_x = (errsv); \
                                      \
        nm_assert(_errsv_x > 0);      \
        _errsv_x;                     \
    })

#define NM_AUTO_DEFINE_FCN0(Type, name, func) \
    static inline void name(Type *v)          \
    {                                         \
        if (*v)                               \
            func(*v);                         \
    }

#define NM_AUTO_DEFINE_FCN_VOID0(CastType, name, func) \
    static inline void name(void *v)                   \
    {                                                  \
        if (*((CastType *) v))                         \
            func(*((CastType *) v));                   \
    }

NM_AUTO_DEFINE_FCN0(GChecksum *, _nm_auto_checksum_free, g_checksum_free);
#define nm_auto_free_checksum nm_auto(_nm_auto_checksum_free)

NM_AUTO_DEFINE_FCN_VOID0(GMutex *, _nm_auto_unlock_g_mutex, g_mutex_unlock);

#define nm_auto_unlock_g_mutex nm_auto(_nm_auto_unlock_g_mutex)

#define NM_UNIQ_T(x, uniq) NM_PASTE(__unique_prefix_, NM_PASTE(x, uniq))

#define _NM_G_MUTEX_LOCKED(lock, uniq)                                      \
    _nm_unused nm_auto_unlock_g_mutex GMutex *NM_UNIQ_T(nm_lock, uniq) = ({ \
        GMutex *const _lock = (lock);                                       \
                                                                            \
        g_mutex_lock(_lock);                                                \
        _lock;                                                              \
    })

#define NM_UNIQ            __COUNTER__

#define NM_G_MUTEX_LOCKED(lock) _NM_G_MUTEX_LOCKED(lock, NM_UNIQ)

#define NM_UTILS_CHECKSUM_LENGTH_MD5    16
#define NM_UTILS_CHECKSUM_LENGTH_SHA1   20
#define NM_UTILS_CHECKSUM_LENGTH_SHA256 32

#define nm_utils_checksum_get_digest(sum, arr)                                   \
    G_STMT_START                                                                 \
    {                                                                            \
        GChecksum *const _sum = (sum);                                           \
        gsize            _len;                                                   \
                                                                                 \
        G_STATIC_ASSERT_EXPR(sizeof(arr) == NM_UTILS_CHECKSUM_LENGTH_MD5         \
                             || sizeof(arr) == NM_UTILS_CHECKSUM_LENGTH_SHA1     \
                             || sizeof(arr) == NM_UTILS_CHECKSUM_LENGTH_SHA256); \
        G_STATIC_ASSERT_EXPR(sizeof(arr) == G_N_ELEMENTS(arr));                  \
                                                                                 \
        nm_assert(_sum);                                                         \
                                                                                 \
        _len = G_N_ELEMENTS(arr);                                                \
                                                                                 \
        g_checksum_get_digest(_sum, (arr), &_len);                               \
        nm_assert(_len == G_N_ELEMENTS(arr));                                    \
    }                                                                            \
    G_STMT_END


#define _NM_IN_SET(uniq, op, type, x, ...)                                \
    ({                                                                    \
        type NM_UNIQ_T(xx, uniq) = (x);                                   \
                                                                          \
        /* trigger a -Wenum-compare warning */                            \
        nm_assert(true || NM_UNIQ_T(xx, uniq) == (x));                    \
                                                                          \
        !!(NM_VA_ARGS_FOREACH(, , op, _NM_IN_SET_OP, uniq, __VA_ARGS__)); \
    })

#define _NM_IN_SET_OP(x, idx, uniq) ((int) (NM_UNIQ_T(xx, uniq) == (x)))

/* Beware that this does short-circuit evaluation (use "||" instead of "|")
 * which has a possibly unexpected non-function-like behavior.
 * Use NM_IN_SET_SE if you need all arguments to be evaluated. */
#define NM_IN_SET(x, ...) _NM_IN_SET(NM_UNIQ, ||, typeof(x), x, __VA_ARGS__)

/*****************************************************************************/

/* Historically, our cleanup macros come from a long gone library
 * libgsystem, hence the "gs_" prefix. We still keep using them,
 * although maybe we should drop them and use our respective nm_auto*
 * macros (TODO).
 *
 * GLib also has g_auto() since 2.44. First of all, we still don't
 * depend on 2.44, so we would have add compat implementations to
 * "nm-glib.h" or bump the version.
 * Also, they work differently (nm_auto_unref_hashtable vs g_auto(GHashTable)).
 * If we were to switch to g_auto(), the change would be slightly more complicated
 * than replacing one macro with another (but still easy).
 * However, the reason for using our nm_auto* macros is that we also want cleanup
 * macros in libnm-std-aux, which has no glib dependency. So we still would have
 * some nm_auto* macros mixed with g_auto(). Instead, we consistently use
 * nm_auto* macros (and the gs_* aliases).
 *
 * Note that c-stdaux also brings cleanup macros like _c_cleanup_(c_freep).
 * We use c-stdaux like a proper internal library, so we could instead switch
 * from nm_auto* macros to _c_cleanup_(). Unlike glib, c-stdaux is used by
 * libnm-std-aux. Again, _c_cleanup_ follows a different pattern both from
 * nm_auto* and g_auto(). */
#define gs_free            nm_auto_g_free
#define gs_unref_object    nm_auto_unref_object
#define gs_unref_variant   nm_auto_unref_variant
#define gs_unref_array     nm_auto_unref_array
#define gs_unref_ptrarray  nm_auto_unref_ptrarray
#define gs_unref_hashtable nm_auto_unref_hashtable
#define gs_unref_bytes     nm_auto_unref_bytes
#define gs_strfreev        nm_auto_strfreev
#define gs_free_error      nm_auto_free_error

/*****************************************************************************/

NM_AUTO_DEFINE_FCN_VOID0(void *, _nm_auto_g_free, g_free);
#define nm_auto_g_free nm_auto(_nm_auto_g_free)

NM_AUTO_DEFINE_FCN_VOID0(GObject *, _nm_auto_unref_object, g_object_unref);
#define nm_auto_unref_object nm_auto(_nm_auto_unref_object)

NM_AUTO_DEFINE_FCN0(GVariant *, _nm_auto_unref_variant, g_variant_unref);
#define nm_auto_unref_variant nm_auto(_nm_auto_unref_variant)

NM_AUTO_DEFINE_FCN0(GArray *, _nm_auto_unref_array, g_array_unref);
#define nm_auto_unref_array nm_auto(_nm_auto_unref_array)

NM_AUTO_DEFINE_FCN0(GPtrArray *, _nm_auto_unref_ptrarray, g_ptr_array_unref);
#define nm_auto_unref_ptrarray nm_auto(_nm_auto_unref_ptrarray)

NM_AUTO_DEFINE_FCN0(GHashTable *, _nm_auto_unref_hashtable, g_hash_table_unref);
#define nm_auto_unref_hashtable nm_auto(_nm_auto_unref_hashtable)

NM_AUTO_DEFINE_FCN0(GSList *, _nm_auto_free_slist, g_slist_free);
#define nm_auto_free_slist nm_auto(_nm_auto_free_slist)

NM_AUTO_DEFINE_FCN0(GBytes *, _nm_auto_unref_bytes, g_bytes_unref);
#define nm_auto_unref_bytes nm_auto(_nm_auto_unref_bytes)

NM_AUTO_DEFINE_FCN0(char **, _nm_auto_strfreev, g_strfreev);
#define nm_auto_strfreev nm_auto(_nm_auto_strfreev)

NM_AUTO_DEFINE_FCN0(GError *, _nm_auto_free_error, g_error_free);
#define nm_auto_free_error nm_auto(_nm_auto_free_error)

NM_AUTO_DEFINE_FCN0(GKeyFile *, _nm_auto_unref_keyfile, g_key_file_unref);
#define nm_auto_unref_keyfile nm_auto(_nm_auto_unref_keyfile)

NM_AUTO_DEFINE_FCN0(GVariantIter *, _nm_auto_free_variant_iter, g_variant_iter_free);
#define nm_auto_free_variant_iter nm_auto(_nm_auto_free_variant_iter)

NM_AUTO_DEFINE_FCN0(GVariantBuilder *, _nm_auto_unref_variant_builder, g_variant_builder_unref);
#define nm_auto_unref_variant_builder nm_auto(_nm_auto_unref_variant_builder)

#define nm_auto_clear_variant_builder nm_auto(g_variant_builder_clear)

NM_AUTO_DEFINE_FCN0(GList *, _nm_auto_free_list, g_list_free);
#define nm_auto_free_list nm_auto(_nm_auto_free_list)

#define nm_auto_unset_gvalue nm_auto(g_value_unset)

NM_AUTO_DEFINE_FCN_VOID0(void *, _nm_auto_unref_gtypeclass, g_type_class_unref);
#define nm_auto_unref_gtypeclass nm_auto(_nm_auto_unref_gtypeclass)

NM_AUTO_DEFINE_FCN0(GByteArray *, _nm_auto_unref_bytearray, g_byte_array_unref);
#define nm_auto_unref_bytearray nm_auto(_nm_auto_unref_bytearray)

NM_AUTO_DEFINE_FCN0(GDateTime *, _nm_auto_unref_gdatetime, g_date_time_unref);
#define nm_auto_unref_gdatetime nm_auto(_nm_auto_unref_gdatetime)

static inline void
_nm_auto_free_gstring(GString **str)
{
    if (*str)
        g_string_free(*str, TRUE);
}
#define nm_auto_free_gstring nm_auto(_nm_auto_free_gstring)

NM_AUTO_DEFINE_FCN0(GSource *, _nm_auto_unref_gsource, g_source_unref);
#define nm_auto_unref_gsource nm_auto(_nm_auto_unref_gsource)

NM_AUTO_DEFINE_FCN0(guint, _nm_auto_remove_source, g_source_remove);
#define nm_auto_remove_source nm_auto(_nm_auto_remove_source)

NM_AUTO_DEFINE_FCN0(GIOChannel *, _nm_auto_unref_io_channel, g_io_channel_unref);
#define nm_auto_unref_io_channel nm_auto(_nm_auto_unref_io_channel)

NM_AUTO_DEFINE_FCN0(GMainLoop *, _nm_auto_unref_gmainloop, g_main_loop_unref);
#define nm_auto_unref_gmainloop nm_auto(_nm_auto_unref_gmainloop)

NM_AUTO_DEFINE_FCN0(GOptionContext *, _nm_auto_free_option_context, g_option_context_free);
#define nm_auto_free_option_context nm_auto(_nm_auto_free_option_context)

static inline void
_nm_auto_freev(gpointer ptr)
{
    gpointer **p = (gpointer **)ptr;
    gpointer  *_ptr;

    if (*p) {
        for (_ptr = *p; *_ptr; _ptr++)
            g_free(*_ptr);
        g_free(*p);
    }
}
/* g_free a NULL terminated array of pointers, with also freeing each
 * pointer with g_free(). It essentially does the same as
 * gs_strfreev / g_strfreev(), but not restricted to strv arrays. */
#define nm_auto_freev nm_auto(_nm_auto_freev)

#define NM_DBUS_DEFAULT_TIMEOUT_MSEC 25000

static inline void
nm_g_variant_unref_floating(GVariant *var)
{
    /* often a function wants to keep a reference to an input variant.
     * It uses g_variant_ref_sink() to either increase the ref-count,
     * or take ownership of a possibly floating reference.
     *
     * If the function doesn't actually want to do anything with the
     * input variant, it still must make sure that a passed in floating
     * reference is consumed. Hence, this helper which:
     *
     *   - does nothing if @var is not floating
     *   - unrefs (consumes) @var if it is floating. */
    if (g_variant_is_floating(var))
        g_variant_unref(var);
}

static inline void
nm_g_set_error_take(GError **error, GError *error_take)
{
    if (!error_take)
        g_return_if_reached();
    if (!error) {
        g_error_free(error_take);
        return;
    }
    if (*error) {
        g_error_free(error_take);
        g_return_if_reached();
    }
    *error = error_take;
}

#define nm_g_set_error_take_lazy(error, error_take_lazy)    \
    {                                                       \
        GError **_error = (error);                          \
                                                            \
        if (_error)                                         \
            nm_g_set_error_take(_error, (error_take_lazy)); \
    }

gboolean
nm_client_deactivate_connection(NMClient           *client,
                                NMActiveConnection *active,
                                GCancellable       *cancellable,
                                GError            **error);


NMActiveConnectionState
nmc_activation_get_effective_state(NMActiveConnection *active,
                                   NMDevice           *device,
                                   const char        **reason);
#ifdef __cplusplus
}
#endif
#endif //RASPBERRY_PI_OLED_PANEL_NM_UTILS_H
