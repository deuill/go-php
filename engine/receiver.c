// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <stdio.h>
#include <stdbool.h>

#include <main/php.h>

#include "value.h"
#include "engine.h"
#include "receiver.h"
#include "_cgo_export.h"

static zval *engine_receiver_get_proxy(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);
	zval *val = NULL;

	engine_value *result = (engine_value *) engine_receiver_get(this->rcvr, Z_STRVAL_P(member));
	if (result == NULL) {
		MAKE_STD_ZVAL(val);
		ZVAL_NULL(val);

		return val;
	}

	val = value_copy(result->value);
	value_destroy(result);

	return val;
}

static void engine_receiver_set_proxy(zval *object, zval *member, zval *value, const zend_literal *key TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);

	engine_receiver_set(this->rcvr, Z_STRVAL_P(member), (void *) value);
}

static int engine_receiver_exists_proxy(zval *object, zval *member, int check, const zend_literal *key TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);
	engine_value *val = NULL;
	int result = 0;

	if (!engine_receiver_exists(this->rcvr, Z_STRVAL_P(member))) {
		return result;
	}

	switch (check) {
	case 2:
		// Value exists.
		result = 1;
		break;
	case 1:
		val = (engine_value *) engine_receiver_get(this->rcvr, Z_STRVAL_P(member));

		// Value exists and is "truthy".
		convert_to_boolean(val->value);
		result = (Z_BVAL_P(val->value)) ? 1 : 0;
		value_destroy(val);

		break;
	case 0:
		val = (engine_value *) engine_receiver_get(this->rcvr, Z_STRVAL_P(member));

		// Value exists and is not null.
		result = (val->kind != KIND_NULL) ? 1 : 0;
		value_destroy(val);

		break;
	}

	return result;
}

static void engine_receiver_call_proxy(INTERNAL_FUNCTION_PARAMETERS) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(getThis() TSRMLS_CC);
	zend_internal_function *func = (zend_internal_function *) EG(current_execute_data)->function_state.function;
	zval *args = NULL;

	MAKE_STD_ZVAL(args);
	array_init_size(args, ZEND_NUM_ARGS());

	if (zend_copy_parameters_array(ZEND_NUM_ARGS(), args TSRMLS_CC) == FAILURE) {
		zval_dtor(args);
		RETURN_NULL();
	}

	engine_value *result = (engine_value *) engine_receiver_call(this->rcvr, (char *) func->function_name, (void *) args);
	if (result == NULL) {
		zval_dtor(args);
		RETURN_NULL();
	}

	zval_dtor(args);

	zval *val = value_copy(result->value);
	value_destroy(result);

	RETURN_ZVAL(val, 0, 0);
}

static zend_function *engine_receiver_method(zval **object_ptr, char *name, int len, const zend_literal *key TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(*object_ptr TSRMLS_CC);
	zend_internal_function *method = emalloc(sizeof(zend_internal_function));

	method->type = ZEND_INTERNAL_FUNCTION;
	method->module = 0;
	method->handler = engine_receiver_call_proxy;
	method->arg_info = NULL;
	method->num_args = 0;
	method->scope = this->obj.ce;
	method->fn_flags = ZEND_ACC_CALL_VIA_HANDLER;
	method->function_name = estrndup(name, len);

	return (zend_function *) method;
}

static zend_function *engine_receiver_constructor(zval *object TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);

	return this->obj.ce->constructor;
}

static zend_class_entry *engine_receiver_entry(const zval *object TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);

	return this->obj.ce;
}

static int engine_receiver_name(const zval *object, const char **name, zend_uint *len, int parent TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);

	if (parent) {
		return FAILURE;
	}

	*len = this->obj.ce->name_length;
	*name = estrndup(this->obj.ce->name, this->obj.ce->name_length);

	return SUCCESS;
}

static zend_object_handlers engine_receiver_handlers = {
	ZEND_OBJECTS_STORE_HANDLERS,

	engine_receiver_get_proxy,    // read_property
	engine_receiver_set_proxy,    // write_property
	NULL,                         // read_dimension
	NULL,                         // write_dimension

	NULL,                         // get_property_ptr_ptr
	NULL,                         // get
	NULL,                         // set

	engine_receiver_exists_proxy, // has_property
	NULL,                         // unset_property
	NULL,                         // has_dimension
	NULL,                         // unset_dimension

	NULL,                         // get_properties

	engine_receiver_method,       // get_method
	NULL,                         // call_method

	engine_receiver_constructor,  // get_constructor
	engine_receiver_entry,        // get_class_entry
	engine_receiver_name,         // get_class_name

	NULL,                         // compare_objects
	NULL,                         // cast_object
	NULL,                         // count_elements
};

static void engine_receiver_free(void *object TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) object;

	zend_object_std_dtor(&this->obj TSRMLS_CC);
	efree(this);
}

static zend_object_value engine_receiver_create(zend_class_entry *class_type TSRMLS_DC) {
	engine_receiver *this;
	zend_object_value object;

	this = emalloc(sizeof(engine_receiver));
	memset(this, 0, sizeof(engine_receiver));

	zend_object_std_init(&this->obj, class_type TSRMLS_CC);
	this->rcvr = engine_receiver_get_pointer(class_type, "__goreceiver");

	object.handle = zend_objects_store_put(this, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) engine_receiver_free, NULL TSRMLS_CC);
	object.handlers = &engine_receiver_handlers;

	return object;
}

void engine_receiver_define(void *rcvr, char *name) {
	zend_class_entry tmp;
	INIT_CLASS_ENTRY_EX(tmp, name, strlen(name), NULL);

	zend_class_entry *this = zend_register_internal_class(&tmp TSRMLS_CC);
	this->create_object = engine_receiver_create;

	// Store method receiver as internal class property for future use.
	engine_receiver_set_pointer(this, "__goreceiver", rcvr);
}