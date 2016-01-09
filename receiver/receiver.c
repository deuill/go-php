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

static zval *receiver_get(zval *object, zval *member, int type, GET_PARAMS) {
	engine_receiver *this = (engine_receiver *) Z_OBJ_P(object);
	zval *val = NULL;

	#if PHP_MAJOR_VERSION >= 7
		val = rv;
	#else
		MAKE_STD_ZVAL(val);
	#endif

	engine_value *result = receiverGet(this->rcvr, Z_STRVAL_P(member));
	if (result == NULL) {
		ZVAL_NULL(val);
		return val;
	}

	value_copy(val, &result->value);
	value_destroy(result);

	return val;
}

static void receiver_set(zval *object, zval *member, zval *value, SET_PARAMS) {
	engine_receiver *this = (engine_receiver *) Z_OBJ_P(object);
	receiverSet(this->rcvr, Z_STRVAL_P(member), (void *) value);
}

static int receiver_exists(zval *object, zval *member, int check, EXISTS_PARAMS) {
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

		#if PHP_MAJOR_VERSION >= 7
			result = (Z_TYPE(val->value) == IS_TRUE) ? 1 : 0; 
		#else
			result = (Z_BVAL(val->value)) ? 1 : 0;
		#endif
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
	zend_internal_function *func = NULL;

	#if PHP_MAJOR_VERSION >= 7
		func = (zend_internal_function *) EX(func);
	#else
		func = (zend_internal_function *) EG(current_execute_data)->function_state.function;
	#endif

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

	#if PHP_MAJOR_VERSION >= 7
		zend_string_release(func->function_name);
	#else
		efree(func->function_name);
	#endif

	efree(args);
	efree(func);
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

static zend_function *receiver_get_method(METHOD_PARAMS) {
	zend_object *obj = NULL;
	zend_internal_function *method = emalloc(sizeof(zend_internal_function));

	#if PHP_MAJOR_VERSION >= 7
		obj = *object_ptr;
	#else
		engine_receiver *this = (engine_receiver *) Z_OBJ_P(*object_ptr);
		obj = &this->obj
	#endif

	method->type     = ZEND_INTERNAL_FUNCTION;
	method->handler  = receiver_call;
	method->arg_info = NULL;
	method->num_args = 0;
	method->scope    = obj->ce;
	method->fn_flags = ZEND_ACC_CALL_VIA_HANDLER;

	#if PHP_MAJOR_VERSION >= 7
		method->function_name = zend_string_copy(name);
	#else
		method->function_name = estrndup(name, len);
	#endif

	return (zend_function *) method;
}

static zend_function *receiver_constructor(CTOR_PARAMS) {
	zend_object *obj = NULL;
	static zend_internal_function ctor;

	#if PHP_MAJOR_VERSION >= 7
		obj = object;
	#else
		engine_receiver *this = (engine_receiver *) Z_OBJ_P(object);
		obj = &this->obj
	#endif

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

	#if PHP_MAJOR_VERSION < 7
		receiver_get_entry, // get_class_entry
	#endif

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

static RECEIVER_CREATE_RETURN receiver_create(zend_class_entry *class_type) {
	engine_receiver *this;

	this = malloc(sizeof(engine_receiver));
	memset(this, 0, sizeof(engine_receiver));

	zend_object_std_init(&this->obj, class_type);
	this->rcvr = receiver_get_pointer(class_type, "__rcvr__");

	#if PHP_MAJOR_VERSION >= 7
		this->obj.handlers = &receiver_handlers;
		return (zend_object *) this;
	#else
		zend_object_value object;

		object.handle = zend_objects_store_put(this, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) receiver_free, NULL);
		object.handlers = &receiver_handlers;
		return object;
	#endif
}

void receiver_define(char *name, void *rcvr) {
	zend_class_entry tmp;
	INIT_CLASS_ENTRY_EX(tmp, name, strlen(name), NULL);

	zend_class_entry *this = zend_register_internal_class(&tmp);
	this->create_object = receiver_create;

	receiver_handlers.get_class_name = zend_get_std_object_handlers()->get_class_name;

	// Method receiver is stored as internal class property.
	receiver_set_pointer(this, "__rcvr__", rcvr);
}
