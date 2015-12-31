// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <stdio.h>
#include <stdbool.h>

#include <main/php.h>
#include <zend_exceptions.h>

#include "value.h"
#include "receiver.h"
#include "_cgo_export.h"

static zval *receiver_get(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);
	zval *val = NULL;

	engine_value *result = (engine_value *) receiverGet(this->rcvr, Z_STRVAL_P(member));
	if (result == NULL) {
		MAKE_STD_ZVAL(val);
		ZVAL_NULL(val);

		return val;
	}

	val = value_copy(result->value);
	value_destroy(result);

	return val;
}

static void receiver_set(zval *object, zval *member, zval *value, const zend_literal *key TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);

	receiverSet(this->rcvr, Z_STRVAL_P(member), (void *) value);
}

static int receiver_exists(zval *object, zval *member, int check, const zend_literal *key TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);

	if (!receiverExists(this->rcvr, Z_STRVAL_P(member))) {
		// Value does not exist.
		return 0;
	} else if (check == 2) {
		// Value exists.
		return 1;
	}

	int result = 0;
	engine_value *val = receiverGet(this->rcvr, Z_STRVAL_P(member));

	if (check == 1) {
		// Value exists and is "truthy".
		convert_to_boolean(val->value);
		result = (Z_BVAL_P(val->value)) ? 1 : 0;
	} else if (check == 0) {
		// Value exists and is not null.
		result = (val->kind != KIND_NULL) ? 1 : 0;
	} else {
		// Check value is invalid.
		result = 0;
	}

	value_destroy(val);
	return result;
}

static void receiver_call(INTERNAL_FUNCTION_PARAMETERS) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(getThis() TSRMLS_CC);
	zend_internal_function *func = (zend_internal_function *) EG(current_execute_data)->function_state.function;
	zval *args = NULL;

	MAKE_STD_ZVAL(args);
	array_init_size(args, ZEND_NUM_ARGS());

	if (zend_copy_parameters_array(ZEND_NUM_ARGS(), args TSRMLS_CC) == FAILURE) {
		zval_dtor(args);
		RETURN_NULL();
	}

	engine_value *result = (engine_value *) receiverCall(this->rcvr, (char *) func->function_name, (void *) args);
	if (result == NULL) {
		zval_dtor(args);
		RETURN_NULL();
	}

	zval_dtor(args);

	zval *val = value_copy(result->value);
	value_destroy(result);

	RETURN_ZVAL(val, 0, 0);
}

static void receiver_new(INTERNAL_FUNCTION_PARAMETERS) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(getThis() TSRMLS_CC);
	zval *args = NULL;

	MAKE_STD_ZVAL(args);
	array_init_size(args, ZEND_NUM_ARGS());

	if (zend_copy_parameters_array(ZEND_NUM_ARGS(), args TSRMLS_CC) == FAILURE) {
		zval_dtor(args);
		return;
	}

	// Create receiver instance, reusing the `rcvr` member, since we do not
	// require the original receiver reference any longer. If creating the
	// receiver failed, we throw an exception.
	this->rcvr = receiverNew(this->rcvr, (void *) args);
	if (this->rcvr == NULL) {
		zend_throw_exception(NULL, "Failed to instantiate method receiver", 0 TSRMLS_CC);
		zval_dtor(args);

		return;
	}

	zval_dtor(args);
}

static zend_function *receiver_get_method(zval **object_ptr, char *name, int len, const zend_literal *key TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(*object_ptr TSRMLS_CC);
	zend_internal_function *method = emalloc(sizeof(zend_internal_function));

	method->type = ZEND_INTERNAL_FUNCTION;
	method->handler = receiver_call;
	method->arg_info = NULL;
	method->num_args = 0;
	method->scope = this->obj.ce;
	method->fn_flags = ZEND_ACC_CALL_VIA_HANDLER;
	method->function_name = estrndup(name, len);

	return (zend_function *) method;
}

static zend_function *receiver_constructor(zval *object TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);
	static zend_internal_function ctor;

	ctor.type = ZEND_INTERNAL_FUNCTION;
	ctor.handler = receiver_new;
	ctor.arg_info = NULL;
	ctor.num_args = 0;
	ctor.scope = this->obj.ce;
	ctor.fn_flags = 0;
	ctor.function_name = (char *) this->obj.ce->name;

	return (zend_function *) &ctor;
}

static zend_class_entry *receiver_entry(const zval *object TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);

	return this->obj.ce;
}

static int receiver_name(const zval *object, const char **name, zend_uint *len, int parent TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) zend_object_store_get_object(object TSRMLS_CC);

	if (parent) {
		return FAILURE;
	}

	*len = this->obj.ce->name_length;
	*name = estrndup(this->obj.ce->name, this->obj.ce->name_length);

	return SUCCESS;
}

static zend_object_handlers receiver_handlers = {
	ZEND_OBJECTS_STORE_HANDLERS,

	receiver_get,         // read_property
	receiver_set,         // write_property
	NULL,                 // read_dimension
	NULL,                 // write_dimension

	NULL,                 // get_property_ptr_ptr
	NULL,                 // get
	NULL,                 // set

	receiver_exists,      // has_property
	NULL,                 // unset_property
	NULL,                 // has_dimension
	NULL,                 // unset_dimension

	NULL,                 // get_properties

	receiver_get_method,  // get_method
	NULL,                 // call_method

	receiver_constructor, // get_constructor
	receiver_entry,       // get_class_entry
	receiver_name,        // get_class_name

	NULL,                 // compare_objects
	NULL,                 // cast_object
	NULL,                 // count_elements
};

static void receiver_free(void *object TSRMLS_DC) {
	engine_receiver *this = (engine_receiver *) object;

	zend_object_std_dtor(&this->obj TSRMLS_CC);
	efree(this);
}

static zend_object_value receiver_create(zend_class_entry *class_type TSRMLS_DC) {
	engine_receiver *this;
	zend_object_value object;

	this = emalloc(sizeof(engine_receiver));
	memset(this, 0, sizeof(engine_receiver));

	zend_object_std_init(&this->obj, class_type TSRMLS_CC);
	this->rcvr = receiver_get_pointer(class_type, "__rcvr__");

	object.handle = zend_objects_store_put(this, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) receiver_free, NULL TSRMLS_CC);
	object.handlers = &receiver_handlers;

	return object;
}

void receiver_define(char *name, void *rcvr) {
	zend_class_entry tmp;
	INIT_CLASS_ENTRY_EX(tmp, name, strlen(name), NULL);

	zend_class_entry *this = zend_register_internal_class(&tmp TSRMLS_CC);
	this->create_object = receiver_create;

	// Method receiver is stored as internal class property.
	receiver_set_pointer(this, "__rcvr__", rcvr);
}