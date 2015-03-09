/**
 * \file
 * \brief
 * Functions and data structures for eventing.
 *
 * Each source that generates events SHOULD export a &lt;source_type&gt;_event
 * function that returns a named event, or NULL if it does not exist.
 * \code
 * ls_event <source_type>_event(<source_type> source, const char *name)
 * \endcode
 *
 * The name should match lexically in an ASCII case-insensitive manner.
 *
 * Copyrights
 *
 * Portions created or assigned to Cisco Systems, Inc. are
 * Copyright (c) 2010-2011 Cisco Systems, Inc.  All Rights Reserved.
 */

#pragma once

#include "ls_error.h"
#include "ls_mem.h"

/**
 * Datatype for an event dispatcher. Each event source contains an
 * event dispatcher. It creates and manages events, and regulates
 * any event triggerings for its owned events.
 */
typedef LS_API struct _ls_event_dispatcher_t *ls_event_dispatcher;

/**
 * Datatype for an event (notifier). It manages the callbacks and triggerings
 * for a given event.
 */
typedef LS_API struct _ls_event_t *ls_event;

/**
 * Event data passed to bound callbacks.
 */
typedef struct _ls_event_data_t
{
    /**
     * Event source
     */
    const void  *source;
    /**
     * Event name
     */
    const char  *name;
    /**
     * Event object
     */
    ls_event    notifier;
    /**
     * Data specific to this triggering of an event
     */
    void        *data;
    /**
     * Possible selection. Reserved for future use.
     */
    void        *selected;
    /**
     * Pool to use for any modification to this event data
     */
    ls_pool     pool;
    /**
      * Flag to indicate the event has been handled in some manner.
      * Callbacks may set this value to true; the eventing logic will
      * ensure this value, once set to true, is propagated to all further
      * callbacks.
      */
    bool        handled;
} *ls_event_data;

/**
 * Callback executed when an event is triggered. Callbacks should set the
 * handled flag in the ls_event_data to true to indicate the event was handled.
 *
 * \param[in] evt Event information
 * \param[in] arg An argument bound to this callback
 */
typedef void (*ls_event_notify_callback) (ls_event_data evt,
                                          void *arg);

/**
 * Callback executed when an event triggering is complete.
 *
 * \param[in] evt Event information
 * \param[in] result True if any notify callbacks returned true, false
 *            otherwise
 * \param[in] arg The user-provided data when the event was triggered
 */
typedef void (*ls_event_result_callback) (ls_event_data evt,
                                          bool result,
                                          void *arg);

/**
 * Creates a new ls_event_dispatcher for the given source.
 *
 * This function can generate the following errors (set when returning false):
 * \li \c LS_ERR_NO_MEMORY if the dispatcher could not be allocated
 *
 * \invariant source != NULL
 * \invariant dispatch != NULL
 * \param[in] source The event source
 * \param[out] dispatch The created event dispatcher
 * \param[out] err The error information (provide NULL to ignore)
 * \retval bool True if the dispatcher was created successfully.
 */
LS_API bool ls_event_dispatcher_create(const void *source,
                                       ls_event_dispatcher *dispatch,
                                       ls_err *err);

/**
 * Destroys the given dispatcher and frees its resources.
 *
 * \invariant dispatch != NULL
 * \param[in] dispatch The event dispatcher
 */
LS_API void ls_event_dispatcher_destroy(ls_event_dispatcher dispatch);

/**
 * Retrieves the event notifier from the dispatcher for the given name. Events are
 * matched using an ASCII case-insensitive lookup.
 *
 * \invariant dispatch != NULL
 * \invariant name != NULL
 * \param[in] dispatch The event dispatcher
 * \param[in] name The event name
 * \retval ls_event_notifier The event notifier, or NULL if not found
 */
LS_API ls_event ls_event_dispatcher_get_event(
               ls_event_dispatcher dispatch,
               const char *name);

/**
 * Create a new event for the given dispatcher and event name. When
 * created, this event is registered with the given dispatcher and can be
 * accessed via ls_event_dispatcher_get_event().
 *
 * \b NOTE: The event name is case-insensitive; while the original value may
 * be retained, most uses use a "lower-case" variant.
 * \b NOTE: The event name is assumed to be ASCII letters and numbers; no
 * attempt is made to validate or enforce this restriction
 *
 * This function can generate the following errors (set when returning false):
 * \li \c LS_ERR_NO_MEMORY if the event could not be allocated
 * \li \c LS_ERR_INVALID_ARG if {name} is the empty string
 * \li \c LS_ERR_INVALID_STATE if an event for {name} already exists in
 *     {dispatch}
 *
 * \invariant dispatch != NULL
 * \invariant name != NULL
 * \invariant event != NULL
 * \param[in] dispatch The owning event dispatcher
 * \param[in] name The event name
 * \param[out] event The created event
 * \param[out] err The error information (provide NULL to ignore)
 * \retval bool True if the event was created successfully.
 */
LS_API bool ls_event_dispatcher_create_event(
                    ls_event_dispatcher dispatch,
                    const char *name,
                    ls_event *event,
                    ls_err *err);

/**
 * Retrieves the name of this event. The value returned by this function is
 * owned by the event, and its memory is released when the event is destroyed.
 *
 * \invariant event != NULL
 * \param[in] event The event
 * \retval const char * The name of the event
 */
LS_API const char *ls_event_get_name(ls_event event);

/**
 * Retrieves the source for the given event.
 *
 * \invariant event != NULL
 * \param[in] event The event
 * \retval void * The event source
 */
LS_API const void *ls_event_get_source(ls_event event);

/**
 * Binds the given callback to the event.
 *
 * \b NOTE: Callbacks are unique by their pointer reference. Registering the
 * same function multiple times has no affect and will not change binding
 * list position.
 *
 * This function can generate the following errors (set when returning false):
 * \li \c LS_ERR_NO_MEMORY if the binding could not be allocated
 *
 * \invariant event != NULL
 * \invariant cb != NULL
 * \param[in] event The event
 * \param[in] cb The callback to execute when the event is triggered
 * \param[in] arg User-provided data for the callback
 * \param[out] err The error information (provide NULL to ignore)
 * \retval bool True if the callback was successfully bound.
 */
LS_API bool ls_event_bind(ls_event event,
                          ls_event_notify_callback cb,
                          void *arg,
                          ls_err *err);

/**
 * Unbinds the given event callback. If {cb} is not currently bound to the
 * event, this function does nothing.
 *
 * \invariant event != NULL
 * \invariant cb != NULL
 * \param[in] event The event
 * \param[in] cb The callback to unbind
 */
LS_API void ls_event_unbind(ls_event event,
                            ls_event_notify_callback cb);

/**
 * Fires an event on all registered callbacks, with the given data.
 * Triggered events are handled in a "breadth-first" fashion; events triggered
 * within an event callback are added to an event queue and processed when the
 * triggering callback returns. Each source has its own event queue.
 *
 * This function can generate the following errors (set when returning false):
 * \li \c LS_ERR_NO_MEMORY if the triggering info could not be allocated
 *
 * \invariant event != NULL
 * \param[in] event The event
 * \param[in] data The data for this event triggering
 * \param[in] result_cb Callback to receive trigger result
 * \param[in] result_arg User-specific data for result_cb
 * \param[out] err The error information (provide NULL to ignore)
 * \retval bool True if the callback was successfully bound.
 */
LS_API bool ls_event_trigger(ls_event event,
                             void *data,
                             ls_event_result_callback result_cb,
                             void *result_arg,
                             ls_err *err);