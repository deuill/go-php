// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <errno.h>
#include <stdbool.h>
#include <main/php.h>

#include "value.h"

engine_value *value_new(zval *zv) {
	engine_value *value = (engine_value *) malloc((sizeof(engine_value)));
	if (value == NULL) {
		errno = 1;
		return NULL;
	}

	value->value = zv;
	value->kind = Z_TYPE_P(zv);

	errno = 0;
	return value;
}

int value_kind(engine_value *val) {
	return val->kind;
}

engine_value *value_create_long(long int value) {
	zval *zv;

	MAKE_STD_ZVAL(zv);
	ZVAL_LONG(zv, value);

	return value_new(zv);
}

engine_value *value_create_double(double value) {
	zval *zv;

	MAKE_STD_ZVAL(zv);
	ZVAL_DOUBLE(zv, value);

	return value_new(zv);
}

engine_value *value_create_bool(bool value) {
	zval *zv;

	MAKE_STD_ZVAL(zv);
	ZVAL_BOOL(zv, value);

	return value_new(zv);
}

engine_value *value_create_string(char *value) {
	zval *zv;

	MAKE_STD_ZVAL(zv);
	ZVAL_STRING(zv, value, 1);

	return value_new(zv);
}

engine_value *value_create_array(unsigned int size) {
	zval *zv;

	MAKE_STD_ZVAL(zv);
	array_init_size(zv, size);

	return value_new(zv);
}

void value_array_set_index(engine_value *arr, unsigned long idx, engine_value *val) {
	add_index_zval(arr->value, idx, val->value);
}

void value_array_set_key(engine_value *arr, const char *key, engine_value *val) {
	add_assoc_zval(arr->value, key, val->value);
}

engine_value *value_create_object() {
	zval *zv;

	MAKE_STD_ZVAL(zv);
	object_init(zv);

	return value_new(zv);
}

void value_object_add_property(engine_value *obj, const char *key, engine_value *val) {
	add_property_zval(obj->value, key, val->value);
}

int value_get_long(engine_value *val) {
	// Covert value to long if needed.
	if (Z_TYPE_P(val->value) != IS_LONG) {
		convert_to_long(val->value);
		val->kind = Z_TYPE_P(val->value);
	}

	return Z_LVAL_P(val->value);
}

double value_get_double(engine_value *val) {
	// Covert value to double if needed.
	if (Z_TYPE_P(val->value) != IS_DOUBLE) {
		convert_to_double(val->value);
		val->kind = Z_TYPE_P(val->value);
	}

	return Z_DVAL_P(val->value);
}

bool value_get_bool(engine_value *val) {
	// Covert value to long if needed.
	if (Z_TYPE_P(val->value) != IS_BOOL) {
		convert_to_boolean(val->value);
		val->kind = Z_TYPE_P(val->value);
	}

	return Z_BVAL_P(val->value);
}

char *value_get_string(engine_value *val) {
	// Covert value to string if needed.
	if (Z_TYPE_P(val->value) != IS_STRING) {
		convert_to_cstring(val->value);
		val->kind = Z_TYPE_P(val->value);
	}

	return Z_STRVAL_P(val->value);
}

void value_destroy(engine_value *val) {
	zval_dtor(val->value);
	free(val);
}