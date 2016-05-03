// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___CONTEXT_H___
#define ___CONTEXT_H___

static void context_bind_proxy(char *name, zval *value);

#define CONTEXT_EXECUTE(o, v) do {            \
	EG(no_extensions) = 1;                    \
	zend_try {                                \
		ZVAL_NULL(v);                         \
		zend_execute(o, v);                   \
	} zend_catch {                            \
		destroy_op_array(o);                  \
		efree_size(o, sizeof(zend_op_array)); \
		zend_bailout();                       \
	} zend_end_try();                         \
	destroy_op_array(o);                      \
	efree_size(o, sizeof(zend_op_array));     \
	EG(no_extensions) = 0;                    \
} while (0)

#endif
