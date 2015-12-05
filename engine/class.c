// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <stdio.h>
#include <stdbool.h>

#include <main/php.h>

#include "value.h"
#include "engine.h"
#include "class.h"
#include "_cgo_export.h"

ZEND_BEGIN_ARG_INFO_EX(arginfo___get, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo___set, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo___call, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

PHP_METHOD(GoPHPEngineClass, __get) {
	char *name;
	int len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &len) == FAILURE) {
		return;
	}

	void *rcvr = engine_class_get_pointer(Z_OBJCE_P(this_ptr), "__goreceiver");
	engine_value *result = (engine_value *) engine_class_get(rcvr, name);

	if (result == NULL) {
		return;
	}

	zval *tmp = value_copy(result->value);
	value_destroy(result);

	RETURN_ZVAL(tmp, 0, 0);
}


PHP_METHOD(GoPHPEngineClass, __set) {
	char *name;
	int len;
	zval *val;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &len, &val) == FAILURE) {
		return;
	}

	void *rcvr = engine_class_get_pointer(Z_OBJCE_P(this_ptr), "__goreceiver");
	engine_class_set(rcvr, name, (void *) val);
}

PHP_METHOD(GoPHPEngineClass, __call) {
	char *name;
	int len;
	zval *args;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &len, &args) == FAILURE) {
		return;
	}

	void *rcvr = engine_class_get_pointer(Z_OBJCE_P(this_ptr), "__goreceiver");
	engine_value *result = (engine_value *) engine_class_call(rcvr, name, (void *) args);

	if (result == NULL) {
		return;
	}

	zval *tmp = value_copy(result->value);
	value_destroy(result);

	RETURN_ZVAL(tmp, 0, 0);
}

const zend_function_entry engine_class_functions[] = {
	PHP_ME(GoPHPEngineClass, __get, arginfo___get, ZEND_ACC_PUBLIC)
	PHP_ME(GoPHPEngineClass, __set, arginfo___set, ZEND_ACC_PUBLIC)
	PHP_ME(GoPHPEngineClass, __call, arginfo___call, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

bool engine_class_define(void *rcvr, char *name) {
	zend_class_entry tmp_class;
	INIT_CLASS_ENTRY_EX(tmp_class, name, strlen(name), engine_class_functions);

	zend_class_entry *this = zend_register_internal_class(&tmp_class TSRMLS_CC);
	if (this == NULL) {
		return false;
	}

	// Store method receiver as internal class property for future use.
	engine_class_set_pointer(this, "__goreceiver", rcvr);

	return true;
}