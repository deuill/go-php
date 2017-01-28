// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <errno.h>
#include <stdbool.h>
#include <main/php.h>

#include "value.h"

// Creates a new value and initializes type to null.
engine_value *value_new() {
	engine_value *val = malloc(sizeof(engine_value));
	if (val == NULL) {
		errno = 1;
		return NULL;
	}

	val->internal = _value_init();
	val->kind = KIND_NULL;

	errno = 0;
	return val;
}

// Creates a complete copy of a zval.
// The destination zval needs to be correctly initialized before use.
void value_copy(zval *dst, zval *src) {
	ZVAL_COPY_VALUE(dst, src);
	zval_copy_ctor(dst);
}

// Returns engine value type. Usually compared against KIND_* constants, defined
// in the `value.h` header file.
int value_kind(engine_value *val) {
	return val->kind;
}

// Set type and value to null.
void value_set_null(engine_value *val) {
	ZVAL_NULL(val->internal);
	val->kind = KIND_NULL;
}

// Set type and value to integer.
void value_set_long(engine_value *val, long int num) {
	ZVAL_LONG(val->internal, num);
	val->kind = KIND_LONG;
}

// Set type and value to floating point.
void value_set_double(engine_value *val, double num) {
	ZVAL_DOUBLE(val->internal, num);
	val->kind = KIND_DOUBLE;
}

// Set type and value to boolean.
void value_set_bool(engine_value *val, bool status) {
	ZVAL_BOOL(val->internal, status);
	val->kind = KIND_BOOL;
}

// Set type and value to string.
void value_set_string(engine_value *val, char *str) {
	_value_set_string(&val->internal, str);
	val->kind = KIND_STRING;
}

// Set type and value to array with a preset initial size.
void value_set_array(engine_value *val, unsigned int size) {
	array_init_size(val->internal, size);
	val->kind = KIND_ARRAY;
}

// Set type and value to object.
void value_set_object(engine_value *val) {
	object_init(val->internal);
	val->kind = KIND_OBJECT;
}

// Set type and value from zval. The source zval is copied and is otherwise not
// affected.
void value_set_zval(engine_value *val, zval *src) {
	int kind;

	// Determine concrete type from source zval.
	switch (Z_TYPE_P(src)) {
	case IS_NULL:
		kind = KIND_NULL;
		break;
	case IS_LONG:
		kind = KIND_LONG;
		break;
	case IS_DOUBLE:
		kind = KIND_DOUBLE;
		break;
	case IS_STRING:
		kind = KIND_STRING;
		break;
	case IS_OBJECT:
		kind = KIND_OBJECT;
		break;
	case IS_ARRAY:
		kind = KIND_ARRAY;
		HashTable *h = (Z_ARRVAL_P(src));

		// Determine if array is associative or indexed. In the simplest case, a
		// associative array will have different values for the number of elements
		// and the index of the next free element. In cases where the number of
		// elements and the next free index is equal, we must iterate through
		// the hash table and check the keys themselves.
		if (h->nNumOfElements != h->nNextFreeElement) {
			kind = KIND_MAP;
			break;
		}

		unsigned long i = 0;

		for (zend_hash_internal_pointer_reset(h); i < h->nNumOfElements; i++) {
			unsigned long index;
			int type = _value_current_key_get(h, NULL, &index);

			if (type == HASH_KEY_IS_STRING || index != i) {
				kind = KIND_MAP;
				break;
			}

			zend_hash_move_forward(h);
		}

		break;
	default:
		// Booleans need special handling for different PHP versions.
		if (_value_truth(src) != -1) {
			kind = KIND_BOOL;
			break;
		}

		errno = 1;
		return;
	}

	value_copy(val->internal, src);
	val->kind = kind;

	errno = 0;
}

// Set next index of array or map value.
void value_array_next_set(engine_value *arr, engine_value *val) {
	add_next_index_zval(arr->internal, val->internal);
}

void value_array_index_set(engine_value *arr, unsigned long idx, engine_value *val) {
	add_index_zval(arr->internal, idx, val->internal);
	arr->kind = KIND_MAP;
}

void value_array_key_set(engine_value *arr, const char *key, engine_value *val) {
	add_assoc_zval(arr->internal, key, val->internal);
	arr->kind = KIND_MAP;
}

void value_object_property_set(engine_value *obj, const char *key, engine_value *val) {
	add_property_zval(obj->internal, key, val->internal);
}

int value_get_long(engine_value *val) {
	zval tmp;

	// Return value directly if already in correct type.
	if (val->kind == KIND_LONG) {
		return Z_LVAL_P(val->internal);
	}

	value_copy(&tmp, val->internal);
	convert_to_long(&tmp);

	return Z_LVAL(tmp);
}

double value_get_double(engine_value *val) {
	zval tmp;

	// Return value directly if already in correct type.
	if (val->kind == KIND_DOUBLE) {
		return Z_DVAL_P(val->internal);
	}

	value_copy(&tmp, val->internal);
	convert_to_double(&tmp);

	return Z_DVAL(tmp);
}

bool value_get_bool(engine_value *val) {
	zval tmp;

	// Return value directly if already in correct type.
	if (val->kind == KIND_BOOL) {
		return _value_truth(val->internal);
	}

	value_copy(&tmp, val->internal);
	convert_to_boolean(&tmp);

	return _value_truth(&tmp);
}

char *value_get_string(engine_value *val) {
	zval tmp;
	int result;

	switch (val->kind) {
	case KIND_STRING:
		value_copy(&tmp, val->internal);
		break;
	case KIND_OBJECT:
		result = zend_std_cast_object_tostring(val->internal, &tmp, IS_STRING);
		if (result == FAILURE) {
			ZVAL_EMPTY_STRING(&tmp);
		}

		break;
	default:
		value_copy(&tmp, val->internal);
		convert_to_cstring(&tmp);
	}

	int len = Z_STRLEN(tmp) + 1;
	char *str = malloc(len);
	memcpy(str, Z_STRVAL(tmp), len);

	zval_dtor(&tmp);

	return str;
}

unsigned int value_array_size(engine_value *arr) {
	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		return Z_ARRVAL_P(arr->internal)->nNumOfElements;
	case KIND_OBJECT:
		// Object size is determined by the number of properties, regardless of
		// visibility.
		return Z_OBJPROP_P(arr->internal)->nNumOfElements;
	case KIND_NULL:
		// Null values are considered empty.
		return 0;
	}

	// Non-array or object values are considered to be single-value arrays.
	return 1;
}

engine_value *value_array_keys(engine_value *arr) {
	HashTable *h = NULL;
	engine_value *keys = value_new();

	value_set_array(keys, value_array_size(arr));

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
	case KIND_OBJECT:
		if (arr->kind == KIND_OBJECT) {
			h = Z_OBJPROP_P(arr->internal);
		} else {
			h = Z_ARRVAL_P(arr->internal);
		}

		unsigned long i = 0;

		for (zend_hash_internal_pointer_reset(h); i < h->nNumOfElements; i++) {
			_value_current_key_set(h, keys);
			zend_hash_move_forward(h);
		}

		break;
	case KIND_NULL:
		// Null values are considered empty.
		break;
	default:
		// Non-array or object values are considered to contain a single key, '0'.
		add_next_index_long(keys->internal, 0);
	}

	return keys;
}

void value_array_reset(engine_value *arr) {
	HashTable *h = NULL;

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		h = Z_ARRVAL_P(arr->internal);
		break;
	case KIND_OBJECT:
		h = Z_OBJPROP_P(arr->internal);
		break;
	default:
		return;
	}

	zend_hash_internal_pointer_reset(h);
}

engine_value *value_array_next_get(engine_value *arr) {
	HashTable *ht = NULL;
	engine_value *val = value_new();

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		ht = Z_ARRVAL_P(arr->internal);
		break;
	case KIND_OBJECT:
		ht = Z_OBJPROP_P(arr->internal);
		break;
	default:
		// Attempting to return the next index of a non-array value will return
		// the value itself, allowing for implicit conversions of scalar values
		// to arrays.
		value_set_zval(val, arr->internal);
		return val;
	}

	_value_array_next_get(ht, val);
	return val;
}

engine_value *value_array_index_get(engine_value *arr, unsigned long idx) {
	HashTable *ht = NULL;
	engine_value *val = value_new();

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		ht = Z_ARRVAL_P(arr->internal);
		break;
	case KIND_OBJECT:
		ht = Z_OBJPROP_P(arr->internal);
		break;
	default:
		// Attempting to return the first index of a non-array value will return
		// the value itself, allowing for implicit conversions of scalar values
		// to arrays.
		if (idx == 0) {
			value_set_zval(val, arr->internal);
			return val;
		}

		return val;
	}

	_value_array_index_get(ht, idx, val);
	return val;
}

engine_value *value_array_key_get(engine_value *arr, char *key) {
	HashTable *ht = NULL;
	engine_value *val = value_new();

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		ht = Z_ARRVAL_P(arr->internal);
		break;
	case KIND_OBJECT:
		ht = Z_OBJPROP_P(arr->internal);
		break;
	default:
		return val;
	}

	_value_array_key_get(ht, key, val);
	return val;
}

#include "_value.c"
