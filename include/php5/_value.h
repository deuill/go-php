// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___VALUE_H___
#define ___VALUE_H___

#define IS_FALSE (IS_BOOL)
#define IS_TRUE  (IS_BOOL)

#define VALUE_TRUTH(v) (Z_BVAL(v))

#define HASH_GET_CURRENT_KEY(h, k, i) zend_hash_get_current_key(h, k, i, 0)
#define VALUE_CREATE_STRING(z, s) ZVAL_STRING(z, s, 1);

#define VALUE_ARRAY_NEXT_GET(h, v)                                    \
	do {                                                              \
		if (zend_hash_get_current_data(h, (void **) &v) == SUCCESS) { \
			zend_hash_move_forward(h);                                \
			return value_new(v);                                      \
		}                                                             \
		return value_create_null();                                   \
	} while (0)

#define VALUE_ARRAY_INDEX_GET(h, i, v)                             \
	do {                                                           \
		if (zend_hash_index_find(h, i, (void **) &v) == SUCCESS) { \
			return value_new(v);                                   \
		}                                                          \
		return value_create_null();                                \
	} while (0)

#define VALUE_ARRAY_KEY_GET(h, k, v)                                        \
	do {                                                                    \
		if (zend_hash_find(h, k, strlen(k) + 1, (void **) &v) == SUCCESS) { \
			return value_new(v);                                            \
		}                                                                   \
		return value_create_null();                                         \
	} while (0)

#endif