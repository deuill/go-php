// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___CONTEXT_H___
#define ___CONTEXT_H___

static void context_bind_proxy(char *name, zval *value);

#define CONTEXT_EXECUTE(o, v) do {                  \
	zend_op_array *oparr = EG(active_op_array);     \
	zval *retval=NULL;                              \
	zval **retvalptr = EG(return_value_ptr_ptr);    \
	zend_op **opline = EG(opline_ptr);              \
	int interact = CG(interactive);                 \
	EG(return_value_ptr_ptr) = &retval;             \
	EG(active_op_array) = o;                        \
	EG(no_extensions) = 1;                          \
	if (!EG(active_symbol_table)) {                 \
		zend_rebuild_symbol_table();                \
	}                                               \
	CG(interactive) = 0;                            \
	zend_try {                                      \
		zend_execute(o);                            \
	} zend_catch {                                  \
		destroy_op_array(o);                        \
		efree(o);                                   \
		zend_bailout();                             \
	} zend_end_try();                               \
	destroy_op_array(o);                            \
	efree(o);                                       \
	CG(interactive) = interact;                     \
	if (retval) {                                   \
		ZVAL_COPY_VALUE(v, retval);                 \
		zval_copy_ctor(v);                          \
	} else {                                        \
		ZVAL_NULL(v);                               \
	}                                               \
	EG(no_extensions)=0;                            \
	EG(opline_ptr) = opline;                        \
	EG(active_op_array) = oparr;                    \
	EG(return_value_ptr_ptr) = retvalptr;           \
} while (0)

#endif
