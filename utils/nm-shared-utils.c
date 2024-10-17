#include "nm-shared-utils.h"

static void
_nm_utils_invoke_on_idle_complete(InvokeOnIdleData *data) {
    nm_clear_g_signal_handler(data->cancellable, &data->cancelled_id);

    data->callback(data->callback_user_data, data->cancellable);

    nm_g_object_unref(data->cancellable);
    g_source_destroy(data->source);
    nm_g_slice_free(data);
}

static void
_nm_utils_invoke_on_idle_cb_cancelled(GCancellable *cancellable, InvokeOnIdleData *data) {
    if (data->cancelled_id == 0) {
        /* this can only happen during _nm_utils_invoke_on_idle_start(). Don't do anything,
         * we still schedule an idle action. */
        return;
    }

    /* On cancellation, we invoke the callback synchronously.
     *
     * Note that this is not thread-safe, meaning: you can only cancel the cancellable
     * while not iterating the GMainContext (that has the idle/timeout source attached).
     * Making this thread safe would be complicated, and it's simply not used by our
     * callers. */
    _nm_utils_invoke_on_idle_complete(data);
}

static gboolean
_nm_utils_invoke_on_idle_cb_idle(gpointer user_data) {
    _nm_utils_invoke_on_idle_complete(user_data);
    return G_SOURCE_REMOVE;
}

GSource *
nm_g_idle_source_new(int priority,
                     GSourceFunc func,
                     gpointer user_data,
                     GDestroyNotify destroy_notify) {
    GSource *source;

    source = g_idle_source_new();
    if (priority != G_PRIORITY_DEFAULT)
        g_source_set_priority(source, priority);
    g_source_set_callback(source, func, user_data, destroy_notify);
    return source;
}

GSource *
nm_g_timeout_source_new(guint timeout_msec,
                        int priority,
                        GSourceFunc func,
                        gpointer user_data,
                        GDestroyNotify destroy_notify) {
    GSource *source;

    source = g_timeout_source_new(timeout_msec);
    if (priority != G_PRIORITY_DEFAULT)
        g_source_set_priority(source, priority);
    g_source_set_callback(source, func, user_data, destroy_notify);
    return source;
}

static void
_nm_utils_invoke_on_idle_start(gboolean use_timeout,
                               guint timeout_msec,
                               GCancellable *cancellable,
                               NMUtilsInvokeOnIdleCallback callback,
                               gpointer callback_user_data) {
    InvokeOnIdleData *data;
    GSource *source;

    g_return_if_fail(callback);

    data = g_slice_new(InvokeOnIdleData);
    *data = (InvokeOnIdleData) {
            .callback           = callback,
            .callback_user_data = callback_user_data,
            .cancellable        = g_object_ref(cancellable),
            .cancelled_id       = 0,
    };

    if (cancellable) {
        gulong cancelled_id;

        cancelled_id = g_cancellable_connect(cancellable,
                                             G_CALLBACK(_nm_utils_invoke_on_idle_cb_cancelled),
                                             data,
                                             NULL);
        if (cancelled_id == 0) {
            /* the cancellable is already cancelled. We still schedule an idle action. */
            use_timeout = FALSE;
        } else
            data->cancelled_id = cancelled_id;
    }

    if (use_timeout) {
        /* We use G_PRIORITY_DEFAULT_IDLE both for the with/without timeout
         * case. The reason is not strong, but it seems right that the caller
         * requests a lower priority than G_PRIORITY_DEFAULT. That is unlike
         * what g_timeout_add() would do. */
        source = nm_g_timeout_source_new(timeout_msec,
                                         G_PRIORITY_DEFAULT_IDLE,
                                         _nm_utils_invoke_on_idle_cb_idle,
                                         data,
                                         NULL);
    } else {
        source = nm_g_idle_source_new(G_PRIORITY_DEFAULT_IDLE,
                                      _nm_utils_invoke_on_idle_cb_idle,
                                      data,
                                      NULL);
    }

    /* use the current thread default context. */
    g_source_attach(source, g_main_context_get_thread_default());

    data->source = source;
}

void
nm_utils_invoke_on_timeout(guint timeout_msec,
                           GCancellable *cancellable,
                           NMUtilsInvokeOnIdleCallback callback,
                           gpointer callback_user_data) {
    _nm_utils_invoke_on_idle_start(TRUE, timeout_msec, cancellable, callback, callback_user_data);
}
