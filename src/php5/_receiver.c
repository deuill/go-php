// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

static zval *_receiver_get(zval *object, zval *member, int type, const zend_literal *key) {
	zval *retval = NULL;
	MAKE_STD_ZVAL(retval);

	engine_value *result = receiver_get(object, member);
	if (result == NULL) {
		ZVAL_NULL(retval);
		return retval;
	}

	value_copy(retval, result->internal);
	_value_destroy(result);

	return retval;
}

static void _receiver_set(zval *object, zval *member, zval *value, const zend_literal *key) {
	receiver_set(object, member, value);
}

static int _receiver_exists(zval *object, zval *member, int check, const zend_literal *key) {
	return receiver_exists(object, member, check);
}

static int _receiver_method_call(const char *method, INTERNAL_FUNCTION_PARAMETERS) {
	return receiver_method_call((char *) method, INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

static zend_function *_receiver_method_get(zval **object, char *name, int len, const zend_literal *key) {
	zend_object *obj = &(_receiver_this(*object)->obj);
	zend_internal_function *func = receiver_method_get(obj);

	func->function_name = estrndup(name, len);

	return (zend_function *) func;
}

static zend_function *_receiver_constructor_get(zval *object) {
	zend_object *obj = &(_receiver_this(object)->obj);
	zend_internal_function *func = receiver_constructor_get(obj);

	return (zend_function *) func;
}

// Free storage for allocated method receiver instance.
static void _receiver_free(void *object) {
	engine_receiver *this = (engine_receiver *) object;
	zend_object_std_dtor(&(this->obj));
}

// Initialize instance of method receiver object. The method receiver itself is
// attached in the constructor function call.
static zend_object_value _receiver_init(zend_class_entry *class_type) {
	engine_receiver *this = emalloc(sizeof(engine_receiver));
	memset(this, 0, sizeof(engine_receiver));

	zend_object_std_init(&this->obj, class_type);

	zend_object_value object;
	object.handle = zend_objects_store_put(this, (zend_objects_store_dtor_t) zend_objects_destroy_object, (zend_objects_free_object_storage_t) _receiver_free, NULL);
	object.handlers = &receiver_handlers;

	return object;
}

static void _receiver_destroy(char *name) {
	zend_class_entry **class;

	if (zend_hash_find(CG(class_table), name, strlen(name), (void **) &class) == SUCCESS) {
		destroy_zend_class(class);
		zend_hash_del_key_or_index(CG(class_table), name, strlen(name), 0, HASH_DEL_KEY);
	}
}

static engine_receiver *_receiver_this(zval *object) {
	return (engine_receiver *) zend_object_store_get_object(object);
}

static void _receiver_handlers_set(zend_object_handlers *handlers) {
	zend_object_handlers *std = zend_get_std_object_handlers();

	handlers->get_class_name  = std->get_class_name;
	handlers->get_class_entry = std->get_class_entry;
}

// Return class name for method receiver.
char *_receiver_get_name(engine_receiver *rcvr) {
	return (char *) rcvr->obj.ce->name;
}
