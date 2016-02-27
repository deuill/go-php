// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___RECEIVER_H___
#define ___RECEIVER_H___

#define RECEIVER_GET(o, m)           receiver_get(o, m, int t, void **c, zval *r)
#define RECEIVER_SET(o, m, v)        receiver_set(o, m, v, void **c)
#define RECEIVER_EXISTS(o, m, h)     receiver_exists(o, m, h, void **c)
#define RECEIVER_METHOD_GET(o, n, l) receiver_method_get(zend_object **o, zend_string *n, const zval *k)
#define RECEIVER_METHOD_CALL(m)      receiver_method_call(zend_string *m, zend_object *o, INTERNAL_FUNCTION_PARAMETERS)
#define RECEIVER_CONSTRUCTOR_GET(o)  receiver_constructor_get(zend_object *o)
#define RECEIVER_FREE(o)             receiver_free(zend_object *o)
#define RECEIVER_INIT(c)             zend_object *receiver_init(c)

#define RECEIVER_DESTROY(n) do {                                 \
	zval *c = zend_hash_str_find(CG(class_table), n, strlen(n)); \
	if (c != NULL) {                                             \
		destroy_zend_class(c);                                   \
		zend_hash_str_del(CG(class_table), n, strlen(n));        \
	}                                                            \
} while (0)

#define RECEIVER_RETVAL()       (r)
#define RECEIVER_THIS(o)        ((engine_receiver *) Z_OBJ_P(o))
#define RECEIVER_STRING_COPY(n) zend_string_copy(name)

#define RECEIVER_FUNC()               (zend_internal_function *) EX(func)
#define RECEIVER_FUNC_NAME(m)         (m)->val
#define RECEIVER_FUNC_SET_ARGFLAGS(f) zend_set_function_arg_flags((zend_function *) f);

#define RECEIVER_OBJECT(o) (o)
#define RECEIVER_OBJECT_CREATE(r, t) do {  \
	r = emalloc(sizeof(engine_receiver));  \
	memset(r, 0, sizeof(engine_receiver)); \
	zend_object_std_init(&r->obj, t);      \
	object_properties_init(&r->obj, t);    \
	r->obj.handlers = &receiver_handlers;  \
	return &r->obj;                        \
} while (0)

#define RECEIVER_OBJECT_DESTROY(r) do { \
	zend_object_std_dtor(&r->obj);      \
} while (0)

#define RECEIVER_HANDLERS_SET(h) do {                           \
	zend_object_handlers *std = zend_get_std_object_handlers(); \
	h.get_class_name  = std->get_class_name;                    \
	h.free_obj = receiver_free;                                 \
} while (0)

// Return class name for method receiver.
static inline char *receiver_get_name(engine_receiver *rcvr) {
	return rcvr->obj.ce->name->val;
}

#endif