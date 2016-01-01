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
	case IS_NULL:
	case IS_LONG:
	case IS_DOUBLE:
	case IS_BOOL:
	case IS_OBJECT:
	case IS_STRING:
		kind = Z_TYPE_P(zv);
		break;
	case IS_ARRAY:
		kind = KIND_ARRAY;
		HashTable *h = (Z_ARRVAL_P(zv));

		// Determine if array is associative or indexed. In the simplest case, a
		// associative array will have different values for the number of elements
		// and the index of the next free element. In cases where the number of
		// elements and the next free index is equal, we must iterate through
		// the hash table and check the keys themselves.
		if (h->nNumOfElements != h->nNextFreeElement) {
			kind = KIND_MAP;
			break;
		}

		unsigned long key;
		unsigned long i = 0;

		for (zend_hash_internal_pointer_reset(h); i < h->nNumOfElements; i++) {
			if (zend_hash_get_current_key_type(h) != HASH_KEY_IS_LONG) {
				kind = KIND_MAP;
				break;
			}

			zend_hash_get_current_key(h, NULL, &key, 0);
			if (key != i) {
				kind = KIND_MAP;
				break;
			}

			zend_hash_move_forward(h);
		}

		break;
	default:
		errno = 1;
		return NULL;
	}

	engine_value *value = malloc((sizeof(engine_value)));
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

void value_array_next_set(engine_value *arr, engine_value *val) {
	add_next_index_zval(arr->value, val->value);
}

void value_array_index_set(engine_value *arr, unsigned long idx, engine_value *val) {
	arr->kind = KIND_MAP;
	add_index_zval(arr->value, idx, val->value);
}

void value_array_key_set(engine_value *arr, const char *key, engine_value *val) {
	arr->kind = KIND_MAP;
	add_assoc_zval(arr->value, key, val->value);
}

engine_value *value_create_object() {
	zval *zv;

	#ifdef ZTS
		TSRMLS_FETCH();
	#endif

	MAKE_STD_ZVAL(zv);
	object_init(zv);

	return value_new(zv);
}

void value_object_property_add(engine_value *obj, const char *key, engine_value *val) {
	#ifdef ZTS
		TSRMLS_FETCH();
	#endif

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
	zval *tmp;
	int result;

	#ifdef ZTS
		TSRMLS_FETCH();
	#endif

	switch (val->kind) {
	case KIND_STRING:
		tmp = val->value;
		break;
	case KIND_OBJECT:
		result = zend_std_cast_object_tostring(val->value, tmp, IS_STRING TSRMLS_CC);
		if (result == FAILURE) {
			MAKE_STD_ZVAL(tmp);
			ZVAL_EMPTY_STRING(tmp);
		}

		break;
	default:
		tmp = value_copy(val->value);
		convert_to_cstring(tmp);
	}

	int len = Z_STRLEN_P(tmp) + 1;
	char *str = malloc(len);
	memcpy(str, Z_STRVAL_P(tmp), len);

	if (val->kind != KIND_STRING) {
		zval_dtor(tmp);
	}

	return str;
}

unsigned int value_array_size(engine_value *arr) {
	#ifdef ZTS
		TSRMLS_FETCH();
	#endif

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		return Z_ARRVAL_P(arr->value)->nNumOfElements;
	case KIND_OBJECT:
		// Object size is determined by the number of properties, regardless of
		// visibility.
		return Z_OBJPROP_P(arr->value)->nNumOfElements;
	case KIND_NULL:
		// Null values are considered empty.
		return 0;
	}

	// Non-array or object values are considered to be single-value arrays.
	return 1;
}

engine_value *value_array_keys(engine_value *arr) {
	int t = 0;
	char *k = NULL;
	unsigned long i = 0;

	#ifdef ZTS
		TSRMLS_FETCH();
	#endif

	HashTable *h = NULL;
	engine_value *keys = value_create_array(value_array_size(arr));

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
	case KIND_OBJECT:
		if (arr->kind == KIND_OBJECT) {
			h = Z_OBJPROP_P(arr->value);
		} else {
			h = Z_ARRVAL_P(arr->value);
		}

		zend_hash_internal_pointer_reset(h);
		while ((t = zend_hash_get_current_key(h, &k, &i, 0)) != HASH_KEY_NON_EXISTENT) {
			switch (t) {
			case HASH_KEY_IS_LONG:
				add_next_index_long(keys->value, i);
				break;
			case HASH_KEY_IS_STRING:
				add_next_index_string(keys->value, k, 1);
				break;
			}

			zend_hash_move_forward(h);
		}

		break;
	case KIND_NULL:
		// Null values are considered empty.
		break;
	default:
		// Non-array or object values are considered to contain a single key, '0'.
		add_next_index_long(keys->value, 0);
	}

	return keys;
}

void value_array_reset(engine_value *arr) {
	HashTable *h = NULL;

	#ifdef ZTS
		TSRMLS_FETCH();
	#endif

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		h = Z_ARRVAL_P(arr->value);
		break;
	case KIND_OBJECT:
		h = Z_OBJPROP_P(arr->value);
		break;
	default:
		return;
	}

	zend_hash_internal_pointer_reset(h);
}

engine_value *value_array_next_get(engine_value *arr) {
	zval **tmp = NULL;
	HashTable *h = NULL;

	#ifdef ZTS
		TSRMLS_FETCH();
	#endif

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		h = Z_ARRVAL_P(arr->value);
		break;
	case KIND_OBJECT:
		h = Z_OBJPROP_P(arr->value);
		break;
	default:
		// Attempting to return the next index of a non-array value will return
		// the value itself, allowing for implicit conversions of scalar values
		// to arrays.
		return value_new_copy(arr->value);
	}

	while (zend_hash_get_current_data(h, (void **) &tmp) == SUCCESS) {
		zend_hash_move_forward(h);
		return value_new_copy(*tmp);
	}

	return value_create_null();
}

engine_value *value_array_index_get(engine_value *arr, unsigned long idx) {
	zval **zv = NULL;
	HashTable *h = NULL;

	#ifdef ZTS
		TSRMLS_FETCH();
	#endif

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		h = Z_ARRVAL_P(arr->value);
		break;
	case KIND_OBJECT:
		h = Z_OBJPROP_P(arr->value);
		break;
	default:
		// Attempting to return the first index of a non-array value will return
		// the value itself, allowing for implicit conversions of scalar values
		// to arrays.
		if (idx == 0) {
			return value_new_copy(arr->value);
		}

		return value_create_null();
	}

	if (zend_hash_index_find(h, idx, (void **) &zv) == SUCCESS) {
		return value_new_copy(*zv);
	}

	return value_create_null();
}

engine_value *value_array_key_get(engine_value *arr, char *key) {
	zval **zv = NULL;
	HashTable *h = NULL;

	#ifdef ZTS
		TSRMLS_FETCH();
	#endif

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		h = Z_ARRVAL_P(arr->value);
		break;
	case KIND_OBJECT:
		h = Z_OBJPROP_P(arr->value);
		break;
	default:
		return value_create_null();
	}

	if (zend_hash_find(h, key, strlen(key) + 1, (void **) &zv) == SUCCESS) {
		return value_new_copy(*zv);
	}

	return value_create_null();
}