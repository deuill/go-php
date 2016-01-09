// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___VALUE_H___
#define ___VALUE_H___

#define VALUE_TRUTH(v) (Z_TYPE(v) == IS_TRUE)

#define HASH_GET_CURRENT_KEY(h, k, i) zend_hash_get_current_key(h, k, i)
#define VALUE_CREATE_STRING(z, s) ZVAL_STRING(z, s);

#define VALUE_ARRAY_NEXT_GET(h, v)                         \
	do {                                                   \
		if ((v = zend_hash_get_current_data(h)) != NULL) { \
			zend_hash_move_forward(h);                     \
			return value_new(v);                           \
		}                                                  \
		return value_create_null();                        \
	} while (0)

#define VALUE_ARRAY_INDEX_GET(h, i, v)                  \
	do {                                                \
		if ((v = zend_hash_index_find(h, i)) != NULL) { \
			return value_new(v);                        \
		}                                               \
		return value_create_null();                     \
	} while (0)

#define VALUE_ARRAY_KEY_GET(h, k, v)                        \
	do {                                                    \
		zend_string *s = zend_string_init(k, strlen(k), 0); \
		zend_string_release(s);                             \
		if ((v = zend_hash_find(h, s)) != NULL) {           \
			zend_string_release(s);                         \
			return value_new(v);                            \
		}                                                   \
		zend_string_release(s);                             \
		return value_create_null();                         \
	} while (0)

#endif