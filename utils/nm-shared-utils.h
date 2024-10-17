/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2016 Red Hat, Inc.
 */

#ifndef __NM_SHARED_UTILS_H__
#define __NM_SHARED_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <glib.h>
#include <gio/gio.h>

typedef void (*NMUtilsInvokeOnIdleCallback)(gpointer user_data, GCancellable *cancellable);

#define nm_g_slice_free(ptr) g_slice_free(typeof(*(ptr)), ptr)
static inline gboolean
nm_clear_g_signal_handler(gpointer self, gulong *id)
{
    gulong v;

    if (id && (v = *id)) {
        *id = 0;
        g_signal_handler_disconnect(self, v);
        return TRUE;
    }
    return FALSE;
}

typedef struct {
    gpointer callback_user_data;
    GCancellable *cancellable;
    GSource *source;
    NMUtilsInvokeOnIdleCallback callback;
    gulong cancelled_id;
} InvokeOnIdleData;

static inline void
nm_g_object_unref(gpointer obj) {
    /* g_object_unref() doesn't accept NULL. Usually, we workaround that
     * by using g_clear_object(), but sometimes that is not convenient
     * (for example as destroy function for a hash table that can contain
     * NULL values). */
    if (obj)
        g_object_unref(obj);
}

void nm_utils_invoke_on_idle(GCancellable *cancellable,
                             NMUtilsInvokeOnIdleCallback callback,
                             gpointer callback_user_data);

void nm_utils_invoke_on_timeout(guint timeout_msec,
                                GCancellable *cancellable,
                                NMUtilsInvokeOnIdleCallback callback,
                                gpointer callback_user_data);
#ifdef __cplusplus
}
#endif

#endif /* __NM_SHARED_UTILS_H__ */
