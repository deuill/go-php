// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___RECEIVER_H___
#define ___RECEIVER_H___

#define RECEIVER_GET(o, m) zval *receiver_get(zval *o, zval *m, int t, void **c, zval *r)
#define RECEIVER_RETVAL_GET(v) r

#define RECEIVER_SET(o, m, v) void receiver_set(zval *o, zval *m, zval *v, void **c)
#define RECEIVER_EXISTS(o, m, ck) int receiver_exists(zval *o, zval *m, int ck, void **c)

#define RECEIVER_FUNC_GET() (zend_internal_function *) EX(func)
#define RECEIVER_FUNC_DESTROY(f) zend_string_release(f->function_name); efree(f)

#define RECEIVER_GET_METHOD(o, n, l) zend_function *receiver_get_method(zend_object **o, zend_string *n, const zval *k)
#define RECEIVER_METHOD_NAME(n, l) zend_string_copy(name)

#define RECEIVER_CONSTRUCTOR(o) zend_function *receiver_constructor(zend_object *o)
#define RECEIVER_CREATE(ct) zend_object *receiver_create(zend_class_entry *ct)

#define RECEIVER_OBJECT_GET(o) o
#define RECEIVER_OBJECT_CREATE(r)             \
	do {                                      \
		r->obj.handlers = &receiver_handlers; \
		return (zend_object *) r;             \
	} while (0)

#define RECEIVER_GET_ENTRY

#endif