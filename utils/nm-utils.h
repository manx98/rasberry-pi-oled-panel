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
#include <errno.h>

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

const char *nmc_device_reason_to_string(NMDeviceStateReason reason);

const char *nmc_device_state_to_string(NMDeviceState state);

#define _(str) str
#define N_(str) str

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

#if defined(__clang__)
/* Clang can emit -Wunused-but-set-variable warning for cleanup variables
 * that are only assigned (never used otherwise). Hack around */
#define _nm_auto_extra _nm_unused
#else
#define _nm_auto_extra
#endif

static inline void
_nm_auto_protect_errno(const int *p_saved_errno) {
    errno = *p_saved_errno;
}

#define _nm_unused             __attribute__((__unused__))

#define NM_AUTO_PROTECT_ERRNO(errsv_saved) \
    nm_auto(_nm_auto_protect_errno) _nm_unused const int errsv_saved = (errno)

#define nm_auto(fcn) _nm_auto_extra __attribute__((__cleanup__(fcn)))

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

NM_AUTO_DEFINE_FCN_VOID0(GMutex *, _nm_auto_unlock_g_mutex, g_mutex_unlock);

#define gs_free            nm_auto_g_free

NM_AUTO_DEFINE_FCN_VOID0(void *, _nm_auto_g_free, g_free);
#define nm_auto_g_free nm_auto(_nm_auto_g_free)

static inline void
_nm_auto_free_gstring(GString **str)
{
    if (*str)
        g_string_free(*str, TRUE);
}
#define nm_auto_free_gstring nm_auto(_nm_auto_free_gstring)

NMActiveConnectionState
nmc_activation_get_effective_state(NMActiveConnection *active,
                                   NMDevice           *device,
                                   const char        **reason);
#ifdef __cplusplus
}
#endif
#endif //RASPBERRY_PI_OLED_PANEL_NM_UTILS_H
