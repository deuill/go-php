// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___VALUE_H___
#define ___VALUE_H___

#define CASE_BOOL      IS_TRUE: case IS_FALSE
#define VALUE_TRUTH(v) (Z_TYPE_P(v) == IS_TRUE)

#define VALUE_SET_STRING(z, s) ZVAL_STRING(z, s);

#define VALUE_INIT(v) do {      \
	(v) = malloc(sizeof(zval)); \
	ZVAL_NULL(v);               \
} while (0)

#define HASH_GET_CURRENT_KEY(h, k, i) zend_hash_get_current_key(h, k, i)
#define HASH_SET_CURRENT_KEY(h, v) do {    \
	zval t;                                \
	zend_hash_get_current_key_zval(h, &t); \
	add_next_index_zval(v, &t);            \
} while (0)

#define VALUE_ARRAY_NEXT_GET(h, v) do {                \
	zval *t = NULL;                                    \
	if ((t = zend_hash_get_current_data(h)) != NULL) { \
		value_set_zval(v, t);                          \
		zend_hash_move_forward(h);                     \
	}                                                  \
	return v;                                          \
} while (0)

#define VALUE_ARRAY_INDEX_GET(h, i, v) do {         \
	zval *t = NULL;                                 \
	if ((t = zend_hash_index_find(h, i)) != NULL) { \
		value_set_zval(v, t);                       \
	}                                               \
	return v;                                       \
} while (0)

#define VALUE_ARRAY_KEY_GET(h, k, v) do {               \
	zval *t = NULL;                                     \
	zend_string *s = zend_string_init(k, strlen(k), 0); \
	if ((t = zend_hash_find(h, s)) != NULL) {           \
		value_set_zval(v, t);                           \
	}                                                   \
	zend_string_release(s);                             \
	return v;                                           \
} while (0)

// Destroy and free engine value.
static inline void value_destroy(engine_value *val) {
	zval_dtor(val->internal);
	free(val->internal);
	free(val);
}

#endif