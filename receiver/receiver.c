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

// Fetch and return method receiver field.
static RECEIVER_GET(object, member) {
	engine_receiver *this = (engine_receiver *) Z_OBJ_P(object);
	zval *val = RECEIVER_RETVAL_GET(val);

	engine_value *result = receiverGet(this->rcvr, Z_STRVAL_P(member));
	if (result == NULL) {
		ZVAL_NULL(val);
		return val;
	}

	value_copy(val, &result->value);
	value_destroy(result);

	return val;
}

static RECEIVER_SET(object, member, value) {
	engine_receiver *this = (engine_receiver *) Z_OBJ_P(object);
	receiverSet(this->rcvr, Z_STRVAL_P(member), (void *) value);
}

static RECEIVER_EXISTS(object, member, check) {
	engine_receiver *this = (engine_receiver *) Z_OBJ_P(object);

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
		convert_to_boolean(&val->value);
		result = VALUE_TRUTH(val->value) ? 1 : 0;
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
	engine_receiver *this = (engine_receiver *) Z_OBJ_P(getThis());
	zend_internal_function *func = RECEIVER_FUNC_GET();

	zval *args = emalloc(sizeof(zval) * ZEND_NUM_ARGS());
	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
		RETVAL_NULL();
	} else {
		engine_value *result = receiverCall(this->rcvr, (char *) func->function_name, (void *) args);
		if (result == NULL) {
			RETVAL_NULL();
		} else {
			value_copy(return_value, &result->value);
			value_destroy(result);
		}
	}

	RECEIVER_FUNC_DESTROY(func);
	efree(args);
}

static void receiver_new(INTERNAL_FUNCTION_PARAMETERS) {
	engine_receiver *this = (engine_receiver *) Z_OBJ_P(getThis());

	zval *args = emalloc(sizeof(zval) * ZEND_NUM_ARGS());
	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
		zend_throw_error(NULL, "Could not parse parameters for method receiver");
		efree(args);

		return;
	}

	// Create receiver instance, reusing the `rcvr` member, since we do not
	// require the original receiver reference any longer. If creating the
	// receiver failed, we throw an exception.
	this->rcvr = receiverNew(this->rcvr, (void *) args);
	if (this->rcvr == NULL) {
		zend_throw_error(NULL, "Failed to instantiate method receiver");
	}

	efree(args);
}

static RECEIVER_GET_METHOD(object_ptr, name, len) {
	zend_object *obj = RECEIVER_OBJECT_GET(*object_ptr);
	zend_internal_function *method = emalloc(sizeof(zend_internal_function));

	method->type     = ZEND_INTERNAL_FUNCTION;
	method->handler  = receiver_call;
	method->arg_info = NULL;
	method->num_args = 0;
	method->scope    = obj->ce;
	method->fn_flags = ZEND_ACC_CALL_VIA_HANDLER;
	method->function_name = RECEIVER_METHOD_NAME(name, len);

	return (zend_function *) method;
}

static RECEIVER_CONSTRUCTOR(object) {
	zend_object *obj = RECEIVER_OBJECT_GET(object);
	static zend_internal_function ctor;

	ctor.type     = ZEND_INTERNAL_FUNCTION;
	ctor.handler  = receiver_new;
	ctor.arg_info = NULL;
	ctor.num_args = 0;
	ctor.scope    = obj->ce;
	ctor.fn_flags = 0;
	ctor.function_name = obj->ce->name;

	return (zend_function *) &ctor;
}

static zend_class_entry *receiver_get_entry(const zval *object) {
	engine_receiver *this = (engine_receiver *) Z_OBJ_P(object);
	return this->obj.ce;
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
	RECEIVER_GET_ENTRY    // get_class_entry

	NULL,                 // get_class_name

	NULL,                 // compare_objects
	NULL,                 // cast_object
	NULL,                 // count_elements
};

static void receiver_free(void *object) {
	engine_receiver *this = (engine_receiver *) object;

	zend_object_std_dtor(&this->obj);
	free(this);
}

static RECEIVER_CREATE(class_type) {
	engine_receiver *this;

	this = malloc(sizeof(engine_receiver));
	memset(this, 0, sizeof(engine_receiver));

	zend_object_std_init(&this->obj, class_type);
	this->rcvr = receiver_get_pointer(class_type, "__rcvr__");

	RECEIVER_OBJECT_CREATE(this);
}

void receiver_define(char *name, void *rcvr) {
	zend_class_entry tmp;
	INIT_CLASS_ENTRY_EX(tmp, name, strlen(name), NULL);

	zend_class_entry *this = zend_register_internal_class(&tmp);
	this->create_object = receiver_create;

	// Method receiver is stored as internal class property.
	receiver_set_pointer(this, "__rcvr__", rcvr);

	// Additional method handlers.
	receiver_handlers.get_class_name = zend_get_std_object_handlers()->get_class_name;
}
