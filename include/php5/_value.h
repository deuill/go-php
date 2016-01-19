// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___VALUE_H___
#define ___VALUE_H___

#define CASE_BOOL      IS_BOOL
#define VALUE_TRUTH(v) (Z_BVAL_P(v))

#define VALUE_SET_STRING(z, s) ZVAL_STRING(z, s, 1);

#define VALUE_INIT(v) do { \
	MAKE_STD_ZVAL(v);      \
	ZVAL_NULL(v);          \
} while (0)

#define VALUE_FREE(v) do { \
	zval_dtor(v);          \
} while (0)

#define HASH_GET_CURRENT_KEY(h, k, i) zend_hash_get_current_key(h, k, i, 0)
#define HASH_SET_CURRENT_KEY(h, v) do {    \
	zval *t;                               \
	MAKE_STD_ZVAL(t);                      \
	zend_hash_get_current_key_zval(h, t); \
	add_next_index_zval(v, t);            \
} while (0)

#define VALUE_ARRAY_NEXT_GET(h, v) do {                           \
	zval **t = NULL;                                              \
	if (zend_hash_get_current_data(h, (void **) &t) == SUCCESS) { \
		value_set_zval(v, *t);                                    \
		zend_hash_move_forward(h);                                \
	}                                                             \
	return v;                                                     \
} while (0)

#define VALUE_ARRAY_INDEX_GET(h, i, v) do {                    \
	zval **t = NULL;                                           \
	if (zend_hash_index_find(h, i, (void **) &t) == SUCCESS) { \
		value_set_zval(v, *t);                                 \
	}                                                          \
	return v;                                                  \
} while (0)

#define VALUE_ARRAY_KEY_GET(h, k, v) do {                               \
	zval **t = NULL;                                                    \
	if (zend_hash_find(h, k, strlen(k) + 1, (void **) &t) == SUCCESS) { \
		value_set_zval(v, *t);                                          \
	}                                                                   \
	return v;                                                           \
} while (0)

#endif