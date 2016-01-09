// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___RECEIVER_H___
#define ___RECEIVER_H___

#define RECEIVER_GET(o, m) zval *receiver_get(zval *o, zval *m, int t, const zend_literal *k)
#define RECEIVER_RETVAL_GET(v) NULL; MAKE_STD_ZVAL(v)

#define RECEIVER_SET(o, m, v) void receiver_set(zval *o, zval *m, zval *v, const zend_literal *k)
#define RECEIVER_EXISTS(o, m, c) int receiver_exists(zval *o, zval *m, int c, const zend_literal *k)

#define RECEIVER_FUNC_GET() (zend_internal_function *) EG(current_execute_data)->function_state.function
#define RECEIVER_FUNC_DESTROY(f) efree(f->function_name); efree(f)

#define RECEIVER_GET_METHOD(o, n, l) zend_function *receiver_get_method(zval **o, char *n, int l, const zend_literal *k)
#define RECEIVER_METHOD_NAME(n, l) estrndup(n, l)

#define RECEIVER_CONSTRUCTOR(o) zend_function *receiver_constructor(zval *o)
#define RECEIVER_CREATE(ct) zend_object_value receiver_create(zend_class_entry *ct)

static inline zend_object *RECEIVER_OBJECT_GET(zval *object_ptr) {
	engine_receiver *this = (engine_receiver *) Z_OBJ_P(object_ptr);
	return &this->obj;
}

#define RECEIVER_OBJECT_CREATE(r)                                          \
	do {                                                                   \
		zend_object_value object;                                          \
		object.handle = zend_objects_store_put(r,                          \
				(zend_objects_store_dtor_t) zend_objects_destroy_object,   \
				(zend_objects_free_object_storage_t) receiver_free, NULL); \
		object.handlers = &receiver_handlers;                              \
		return object;                                                     \
	} while (0)

#define RECEIVER_GET_ENTRY receiver_get_entry,

#endif