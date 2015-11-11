// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <errno.h>
#include <stdbool.h>
#include <main/php.h>

#include "value.h"

engine_value *value_new(zval *zv) {
	int kind;

	switch (Z_TYPE_P(zv)) {
	case IS_NULL: case IS_LONG: case IS_DOUBLE: case IS_BOOL: case IS_OBJECT: case IS_STRING:
		kind = Z_TYPE_P(zv);
		break;
	case IS_ARRAY:
		kind = KIND_ARRAY;
		HashTable *ht = (Z_ARRVAL_P(zv));

		// Determine if array is associative or indexed. In the simplest case, a
		// associative array will have different values for the number of elements
		// and the index of the next free element. In cases where the number of
		// elements and the next free index is equal, we must iterate through
		// the hash table and check the keys themselves.
		if (ht->nNumOfElements != ht->nNextFreeElement) {
			kind = KIND_MAP;
			break;
		}

		unsigned long key;
		unsigned long i = 0;

		for (zend_hash_internal_pointer_reset(ht); i < ht->nNumOfElements; i++) {
			if (zend_hash_get_current_key_type(ht) != HASH_KEY_IS_LONG) {
				kind = KIND_MAP;
				break;
			}

			zend_hash_get_current_key(ht, NULL, &key, 0);
			if (key != i) {
				kind = KIND_MAP;
				break;
			}

			zend_hash_move_forward(ht);
		}

		break;
	default:
		errno = 1;
		return NULL;
	}

	engine_value *value = (engine_value *) malloc((sizeof(engine_value)));
	if (value == NULL) {
		errno = 1;
		return NULL;
	}

	value->value = zv;
	value->kind = kind;

	errno = 0;
	return value;
}

int value_kind(engine_value *val) {
	return val->kind;
}

engine_value *value_create_null() {
	zval *zv;

	MAKE_STD_ZVAL(zv);
	ZVAL_NULL(zv);

	return value_new(zv);
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

void value_array_set_next(engine_value *arr, engine_value *val) {
	add_next_index_zval(arr->value, val->value);
}

void value_array_set_index(engine_value *arr, unsigned long idx, engine_value *val) {
	arr->kind = KIND_MAP;
	add_index_zval(arr->value, idx, val->value);
}

void value_array_set_key(engine_value *arr, const char *key, engine_value *val) {
	arr->kind = KIND_MAP;
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
	// Return value directly if already in correct type.
	if (val->kind == KIND_LONG) {
		return Z_LVAL_P(val->value);
	}

	zval *tmp = value_copy(val->value);

	convert_to_long(tmp);
	long v = Z_LVAL_P(tmp);

	zval_dtor(tmp);

	return v;
}

double value_get_double(engine_value *val) {
	// Return value directly if already in correct type.
	if (val->kind == KIND_DOUBLE) {
		return Z_DVAL_P(val->value);
	}

	zval *tmp = value_copy(val->value);

	convert_to_double(tmp);
	double v = Z_DVAL_P(tmp);

	zval_dtor(tmp);

	return v;
}

bool value_get_bool(engine_value *val) {
	// Return value directly if already in correct type.
	if (val->kind == KIND_BOOL) {
		return Z_BVAL_P(val->value);
	}

	zval *tmp = value_copy(val->value);

	convert_to_boolean(tmp);
	bool v = Z_BVAL_P(tmp);

	zval_dtor(tmp);

	return v;
}

char *value_get_string(engine_value *val) {
	// Return value directly if already in correct type.
	if (val->kind == KIND_STRING) {
		return Z_STRVAL_P(val->value);
	}

	zval *tmp = value_copy(val->value);

	convert_to_cstring(tmp);
	char *v = Z_STRVAL_P(tmp);

	zval_dtor(tmp);

	return v;
}

unsigned int value_array_size(engine_value *arr) {
	// Non-array values are considered as single-value arrays.
	if (arr->kind != KIND_ARRAY && arr->kind != KIND_MAP) {
		return 1;
	}

	return Z_ARRVAL_P(arr->value)->nNumOfElements;
}

engine_value *value_array_get_index(engine_value *arr, unsigned long idx) {
	zval **zv = NULL;

	// Attempting to return the first index of a non-array value will return the
	// value itself, allowing for implicit conversions of scalar values to arrays.
	if (arr->kind != KIND_ARRAY && arr->kind != KIND_MAP) {
		if (idx == 1) {
			return value_new(value_copy(arr->value));
		}

		return value_create_null();
	}

	if (zend_hash_index_find(Z_ARRVAL_P(arr->value), idx, (void **) &zv) == SUCCESS) {
		return value_new(*zv);
	}

	return value_create_null();
}

void value_destroy(engine_value *val) {
	zval_dtor(val->value);
	free(val);
}