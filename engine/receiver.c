// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <stdio.h>
#include <stdbool.h>

#include <main/php.h>
#include <zend_exceptions.h>
#include <ext/standard/php_string.h>

#include "value.h"
#include "receiver.h"
#include "_cgo_export.h"

// Fetch and return field for method receiver.
static zval *RECEIVER_GET(zval *object, zval *member) {
	engine_receiver *this = RECEIVER_THIS(object);
	zval *val = RECEIVER_RETVAL();

	engine_value *result = engineReceiverGet(this, Z_STRVAL_P(member));
	if (result == NULL) {
		ZVAL_NULL(val);
		return val;
	}

	value_copy(val, result->internal);
	value_destroy(result);

	return val;
}

// Set field for method receiver.
static void RECEIVER_SET(zval *object, zval *member, zval *value) {
	engine_receiver *this = RECEIVER_THIS(object);
	engineReceiverSet(this, Z_STRVAL_P(member), (void *) value);
}

// Check if field exists for method receiver.
static int RECEIVER_EXISTS(zval *object, zval *member, int check) {
	engine_receiver *this = RECEIVER_THIS(object);

	if (!engineReceiverExists(this, Z_STRVAL_P(member))) {
		// Value does not exist.
		return 0;
	} else if (check == 2) {
		// Value exists.
		return 1;
	}

	int result = 0;
	engine_value *val = engineReceiverGet(this, Z_STRVAL_P(member));

	if (check == 1) {
		// Value exists and is "truthy".
		convert_to_boolean(val->internal);
		result = VALUE_TRUTH(val->internal) ? 1 : 0;
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

// Call function with arguments passed and return value (if any).
static int RECEIVER_METHOD_CALL(method) {
	engine_receiver *this = RECEIVER_THIS(getThis());
	char *name = RECEIVER_FUNC_NAME(method);
	zval args;

	array_init_size(&args, ZEND_NUM_ARGS());

	if (zend_copy_parameters_array(ZEND_NUM_ARGS(), &args) == FAILURE) {
		RETVAL_NULL();
	} else {
		engine_value *result = engineReceiverCall(this, name, (void *) &args);
		if (result == NULL) {
			RETVAL_NULL();
		} else {
			value_copy(return_value, result->internal);
			value_destroy(result);
		}
	}

	zval_dtor(&args);
}

// Create new method receiver instance and attach to instantiated PHP object.
// Returns an exception if method receiver failed to initialize for any reason.
static void receiver_new(INTERNAL_FUNCTION_PARAMETERS) {
	engine_receiver *this = RECEIVER_THIS(getThis());
	zval args;

	array_init_size(&args, ZEND_NUM_ARGS());

	if (zend_copy_parameters_array(ZEND_NUM_ARGS(), &args) == FAILURE) {
		zend_throw_exception(NULL, "Could not parse parameters for method receiver", 0);
	} else {	
		// Create receiver instance. Throws an exception if creation fails.
		int result = engineReceiverNew(this, (void *) &args);
		if (result != 0) {
			zend_throw_exception(NULL, "Failed to instantiate method receiver", 0);
		}
	}

	zval_dtor(&args);
}

// Fetch and return function definition for method receiver. The method call
// happens in the method handler, as returned by this function.
static zend_function *RECEIVER_METHOD_GET(object_ptr, name, len) {
	zend_object *obj = RECEIVER_OBJECT(*object_ptr);
	zend_internal_function *func = emalloc(sizeof(zend_internal_function));

	func->type     = ZEND_OVERLOADED_FUNCTION;
	func->handler  = NULL;
	func->arg_info = NULL;
	func->num_args = 0;
	func->scope    = obj->ce;
	func->fn_flags = ZEND_ACC_CALL_VIA_HANDLER;
	func->function_name = RECEIVER_STRING_COPY(name);

	RECEIVER_FUNC_SET_ARGFLAGS(func);

	return (zend_function *) func;
}

// Fetch and return constructor function definition for method receiver. The
// construct call happens in the constructor handler, as returned by this
// function.
static zend_function *RECEIVER_CONSTRUCTOR_GET(object) {
	zend_object *obj = RECEIVER_OBJECT(object);
	static zend_internal_function func;

	func.type     = ZEND_INTERNAL_FUNCTION;
	func.handler  = receiver_new;
	func.arg_info = NULL;
	func.num_args = 0;
	func.scope    = obj->ce;
	func.fn_flags = 0;
	func.function_name = obj->ce->name;

	RECEIVER_FUNC_SET_ARGFLAGS(&func);

	return (zend_function *) &func;
}

// Table of handler functions for method receivers.
static zend_object_handlers receiver_handlers = {
	ZEND_OBJECTS_STORE_HANDLERS,

	receiver_get,            // read_property
	receiver_set,            // write_property
	NULL,                    // read_dimension
	NULL,                    // write_dimension

	NULL,                    // get_property_ptr_ptr
	NULL,                    // get
	NULL,                    // set

	receiver_exists,         // has_property
	NULL,                    // unset_property
	NULL,                    // has_dimension
	NULL,                    // unset_dimension

	NULL,                    // get_properties

	receiver_method_get,     // get_method
	receiver_method_call,    // call_method

	receiver_constructor_get // get_constructor
};

// Free storage for allocated method receiver instance.
static void RECEIVER_FREE(object) {
	engine_receiver *this = (engine_receiver *) object;
	RECEIVER_OBJECT_DESTROY(this);
}

// Initialize instance of method receiver object. The method receiver itself is
// attached in the constructor function call.
static RECEIVER_INIT(zend_class_entry *class_type) {
	engine_receiver *this;
	RECEIVER_OBJECT_CREATE(this, class_type);
}

// Define class with unique name.
void receiver_define(char *name) {
	zend_class_entry tmp;
	INIT_CLASS_ENTRY_EX(tmp, name, strlen(name), NULL);

	zend_class_entry *this = zend_register_internal_class(&tmp);

	this->create_object = receiver_init;
	this->ce_flags |= ZEND_ACC_FINAL;

	// Set standard handlers for receiver.
	RECEIVER_HANDLERS_SET(receiver_handlers);
}

void receiver_destroy(char *name) {
	name = php_strtolower(name, strlen(name));
	RECEIVER_DESTROY(name);
}
