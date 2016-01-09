// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <errno.h>
#include <stdbool.h>
#include <main/php.h>

#include "value.h"

// Creates new value based on PHP zval internal type and value. The original
// PHP zval is always copied in, and is not affected in any way.
engine_value *value_new(zval *zv) {
	int kind;

	switch (Z_TYPE_P(zv)) {
	case IS_NULL:   kind = KIND_NULL;   break;
	case IS_LONG:   kind = KIND_LONG;   break;
	case IS_DOUBLE: kind = KIND_DOUBLE; break;
	case IS_STRING: kind = KIND_STRING; break;
	case IS_OBJECT: kind = KIND_OBJECT; break;

	#if PHP_MAJOR_VERSION >= 7
		case IS_FALSE: // Fallthrough to KIND_BOOL below.
		case IS_TRUE:  kind = KIND_BOOL; break;
	#else
		case IS_BOOL:  kind = KIND_BOOL; break;
	#endif

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

		unsigned long i = 0;

		for (zend_hash_internal_pointer_reset(h); i < h->nNumOfElements; i++) {
			unsigned long key;

			#if PHP_MAJOR_VERSION >= 7
				int type = zend_hash_get_current_key(h, NULL, &key);
			#else
				int type = zend_hash_get_current_key(h, NULL, &key, 0);
			#endif

			if (type == HASH_KEY_IS_STRING || key != i) {
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

	value_copy(&value->value, zv);
	value->kind = kind;

	errno = 0;
	return value;
}

int value_kind(engine_value *val) {
	return val->kind;
}

engine_value *value_create_null() {
	zval tmp;
	ZVAL_NULL(&tmp);

	return value_new(&tmp);
}

engine_value *value_create_long(long int value) {
	zval tmp;
	ZVAL_LONG(&tmp, value);

	return value_new(&tmp);
}

engine_value *value_create_double(double value) {
	zval tmp;
	ZVAL_DOUBLE(&tmp, value);

	return value_new(&tmp);
}

engine_value *value_create_bool(bool value) {
	zval tmp;
	ZVAL_BOOL(&tmp, value);

	return value_new(&tmp);
}

engine_value *value_create_string(char *value) {
	zval tmp;

	#if PHP_MAJOR_VERSION >= 7
		ZVAL_STRING(&tmp, value);
	#else
		ZVAL_STRING(&tmp, value, 1);
	#endif

	engine_value *val = value_new(&tmp);
	zval_dtor(&tmp);

	return val;
}

engine_value *value_create_array(unsigned int size) {
	zval tmp;
	array_init_size(&tmp, size);

	engine_value *val = value_new(&tmp);
	zval_dtor(&tmp);

	return val;
}

void value_array_next_set(engine_value *arr, engine_value *val) {
	add_next_index_zval(&arr->value, &val->value);
}

void value_array_index_set(engine_value *arr, unsigned long idx, engine_value *val) {
	add_index_zval(&arr->value, idx, &val->value);
	arr->kind = KIND_MAP;
}

void value_array_key_set(engine_value *arr, const char *key, engine_value *val) {
	add_assoc_zval(&arr->value, key, &val->value);
	arr->kind = KIND_MAP;
}

engine_value *value_create_object() {
	zval tmp;
	object_init(&tmp);

	engine_value *val = value_new(&tmp);
	zval_dtor(&tmp);

	return val;
}

void value_object_property_add(engine_value *obj, const char *key, engine_value *val) {
	add_property_zval(&obj->value, key, &val->value);
}

int value_get_long(engine_value *val) {
	zval tmp;

	// Return value directly if already in correct type.
	if (val->kind == KIND_LONG) {
		return Z_LVAL(val->value);
	}

	value_copy(&tmp, &val->value);
	convert_to_long(&tmp);

	return Z_LVAL(tmp);
}

double value_get_double(engine_value *val) {
	zval tmp;

	// Return value directly if already in correct type.
	if (val->kind == KIND_DOUBLE) {
		return Z_DVAL(val->value);
	}

	value_copy(&tmp, &val->value);
	convert_to_double(&tmp);

	return Z_DVAL(tmp);
}

bool value_get_bool(engine_value *val) {
	zval tmp;

	// Return value directly if already in correct type.
	if (val->kind == KIND_BOOL) {
		#if PHP_MAJOR_VERSION >= 7
			return Z_TYPE(val->value) == IS_TRUE; 
		#else
			return Z_BVAL(val->value);
		#endif
	}

	value_copy(&tmp, &val->value);
	convert_to_boolean(&tmp);

	#if PHP_MAJOR_VERSION >= 7
		return Z_TYPE(tmp) == IS_TRUE; 
	#else
		return Z_BVAL(tmp);
	#endif
}

char *value_get_string(engine_value *val) {
	zval tmp;
	int result;

	switch (val->kind) {
	case KIND_STRING:
		tmp = val->value;
		break;
	case KIND_OBJECT:
		result = zend_std_cast_object_tostring(&val->value, &tmp, IS_STRING);
		if (result == FAILURE) {
			ZVAL_EMPTY_STRING(&tmp);
		}

		break;
	default:
		value_copy(&tmp, &val->value);
		convert_to_cstring(&tmp);
	}

	int len = Z_STRLEN(tmp) + 1;
	char *str = malloc(len);
	memcpy(str, Z_STRVAL(tmp), len);

	if (val->kind != KIND_STRING) {
		zval_dtor(&tmp);
	}

	return str;
}

unsigned int value_array_size(engine_value *arr) {
	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		return Z_ARRVAL(arr->value)->nNumOfElements;
	case KIND_OBJECT:
		// Object size is determined by the number of properties, regardless of
		// visibility.
		return Z_OBJPROP(arr->value)->nNumOfElements;
	case KIND_NULL:
		// Null values are considered empty.
		return 0;
	}

	// Non-array or object values are considered to be single-value arrays.
	return 1;
}

engine_value *value_array_keys(engine_value *arr) {
	HashTable *h = NULL;
	engine_value *keys = value_create_array(value_array_size(arr));

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
	case KIND_OBJECT:
		if (arr->kind == KIND_OBJECT) {
			h = Z_OBJPROP(arr->value);
		} else {
			h = Z_ARRVAL(arr->value);
		}

		zend_hash_internal_pointer_reset(h);

		while (zend_hash_get_current_key_type(h) != HASH_KEY_NON_EXISTENT) {
			zval v;

			zend_hash_get_current_key_zval(h, &v);
			add_next_index_zval(&keys->value, &v);

			zend_hash_move_forward(h);
		}

		break;
	case KIND_NULL:
		// Null values are considered empty.
		break;
	default:
		// Non-array or object values are considered to contain a single key, '0'.
		add_next_index_long(&keys->value, 0);
	}

	return keys;
}

void value_array_reset(engine_value *arr) {
	HashTable *h = NULL;

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		h = Z_ARRVAL(arr->value);
		break;
	case KIND_OBJECT:
		h = Z_OBJPROP(arr->value);
		break;
	default:
		return;
	}

	zend_hash_internal_pointer_reset(h);
}

engine_value *value_array_next_get(engine_value *arr) {
	HashTable *h = NULL;
	zval *tmp = NULL;

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		h = Z_ARRVAL(arr->value);
		break;
	case KIND_OBJECT:
		h = Z_OBJPROP(arr->value);
		break;
	default:
		// Attempting to return the next index of a non-array value will return
		// the value itself, allowing for implicit conversions of scalar values
		// to arrays.
		return value_new(&arr->value);
	}

	#if PHP_MAJOR_VERSION >= 7
		while ((tmp = zend_hash_get_current_data(h)) != NULL) {
			zend_hash_move_forward(h);
			return value_new(tmp);
		}
	#else
		while (zend_hash_get_current_data(h, (void **) &tmp) == SUCCESS) {
			zend_hash_move_forward(h);
			return value_new(tmp);
		}
	#endif

	return value_create_null();
}

engine_value *value_array_index_get(engine_value *arr, unsigned long idx) {
	HashTable *h = NULL;
	zval *tmp = NULL;

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		h = Z_ARRVAL(arr->value);
		break;
	case KIND_OBJECT:
		h = Z_OBJPROP(arr->value);
		break;
	default:
		// Attempting to return the first index of a non-array value will return
		// the value itself, allowing for implicit conversions of scalar values
		// to arrays.
		if (idx == 0) {
			return value_new(&arr->value);
		}

		return value_create_null();
	}

	#if PHP_MAJOR_VERSION >= 7
		if ((tmp = zend_hash_index_find(h, idx)) != NULL) {
			return value_new(tmp);
		}
	#else
		if (zend_hash_index_find(h, idx, (void **) &tmp) == SUCCESS) {
			return value_new(tmp);
		}
	#endif

	return value_create_null();
}

engine_value *value_array_key_get(engine_value *arr, char *key) {
	HashTable *h = NULL;
	zval *tmp = NULL;

	switch (arr->kind) {
	case KIND_ARRAY:
	case KIND_MAP:
		h = Z_ARRVAL(arr->value);
		break;
	case KIND_OBJECT:
		h = Z_OBJPROP(arr->value);
		break;
	default:
		return value_create_null();
	}

	#if PHP_MAJOR_VERSION >= 7
		zend_string *k = zend_string_init(key, strlen(key), 0);
		tmp = zend_hash_find(h, k);

		zend_string_release(k);

		if (tmp != NULL) {
			return value_new(tmp);
		}
	#else
		if (zend_hash_find(h, key, strlen(key) + 1, (void **) &tmp) == SUCCESS) {
			return value_new(tmp);
		}
	#endif

	return value_create_null();
}