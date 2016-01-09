// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __RECEIVER_H__
#define __RECEIVER_H__

#if PHP_MAJOR_VERSION >= 7
	#define GET_PARAMS    void **cache_slot, zval *rv
	#define SET_PARAMS    void **cache_slot
	#define EXISTS_PARAMS void **cache_slot
	#define METHOD_PARAMS zend_object **object_ptr, zend_string *name, const zval *key
	#define CTOR_PARAMS   zend_object *object

	#define RECEIVER_CREATE_RETURN zend_object *
#else
	#define GET_PARAMS    const struct _zend_literal *key
	#define SET_PARAMS    const struct _zend_literal *key
	#define EXISTS_PARAMS const struct _zend_literal *key
	#define METHOD_PARAMS zval **object_ptr, char *name, int len, const zend_literal *key
	#define CTOR_PARAMS   zval *object

	#define RECEIVER_CREATE_RETURN zend_object_value
#endif

#define receiver_get_pointer(ce, name) \
	(void *) Z_LVAL_P(zend_read_static_property(ce, name, sizeof(name) - 1, 1))

#define receiver_set_pointer(ce, name, ptr) \
	zend_declare_property_long(ce, name, sizeof(name) - 1, (long int) ptr, \
	ZEND_ACC_STATIC | ZEND_ACC_PRIVATE)

typedef struct _engine_receiver {
	zend_object obj;
	void *rcvr;
} engine_receiver;

void receiver_define(char *name, void *rcvr);

#endif