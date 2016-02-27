// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___RECEIVER_H___
#define ___RECEIVER_H___

#define RECEIVER_GET(o, m)           receiver_get(o, m, int t, const zend_literal *k)
#define RECEIVER_SET(o, m, v)        receiver_set(o, m, v, const zend_literal *k)
#define RECEIVER_EXISTS(o, m, c)     receiver_exists(o, m, c, const zend_literal *k)
#define RECEIVER_METHOD_GET(o, n, l) receiver_method_get(zval **o, char *n, int l, const zend_literal *k)
#define RECEIVER_METHOD_CALL(m)      receiver_method_call(const char *m, INTERNAL_FUNCTION_PARAMETERS)
#define RECEIVER_CONSTRUCTOR_GET(o)  receiver_constructor_get(zval *o)
#define RECEIVER_FREE(o)             receiver_free(void *o)
#define RECEIVER_INIT(c)             zend_object_value receiver_init(c)

#define RECEIVER_DESTROY(n) do {                                                    \
	zend_class_entry **c;                                                           \
	if (zend_hash_find(CG(class_table), n, strlen(n), (void **) &c) == SUCCESS) {   \
		destroy_zend_class(c);                                                      \
		zend_hash_del_key_or_index(CG(class_table), n, strlen(n), 0, HASH_DEL_KEY); \
	}                                                                               \
} while (0)

static inline zval *RECEIVER_RETVAL() {
	zval *val = NULL;
	MAKE_STD_ZVAL(val);
	return val;
}

#define RECEIVER_THIS(o)        ((engine_receiver *) zend_object_store_get_object(o))
#define RECEIVER_STRING_COPY(n) estrndup(n, len)

#define RECEIVER_FUNC()         (zend_internal_function *) EG(current_execute_data)->function_state.function
#define RECEIVER_FUNC_NAME(m)   (char *) (m)
#define RECEIVER_FUNC_SET_ARGFLAGS(f)

#define RECEIVER_OBJECT(o) ((zend_object *) (&(RECEIVER_THIS(o)->obj)))
#define RECEIVER_OBJECT_CREATE(r, t) do {                              \
	r = emalloc(sizeof(engine_receiver));                              \
	memset(r, 0, sizeof(engine_receiver));                             \
	zend_object_std_init(&this->obj, t);                               \
	zend_object_value object;                                          \
	object.handle = zend_objects_store_put(r,                          \
			(zend_objects_store_dtor_t) zend_objects_destroy_object,   \
			(zend_objects_free_object_storage_t) receiver_free, NULL); \
	object.handlers = &receiver_handlers;                              \
	return object;                                                     \
} while (0)

#define RECEIVER_OBJECT_DESTROY(r) do {                                \
	zend_object_std_dtor(&r->obj);                                     \
	efree(r);                                                          \
} while (0)

#define RECEIVER_HANDLERS_SET(h) do {                                  \
	zend_object_handlers *std = zend_get_std_object_handlers();        \
	h.get_class_name  = std->get_class_name;                           \
	h.get_class_entry = std->get_class_entry;                          \
} while (0)

// Return class name for method receiver.
static inline char *receiver_get_name(engine_receiver *rcvr) {
	return (char *) rcvr->obj.ce->name;
}

#endif