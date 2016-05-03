// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

static void context_bind_proxy(char *name, zval *value) {
	zend_hash_str_update(&EG(symbol_table), name, strlen(name), value);
}

static void context_eval_proxy(zend_op_array *op, zval *ret) {
	EG(no_extensions) = 1;

	zend_try {
		ZVAL_NULL(ret);
		zend_execute(op, ret);
	} zend_catch {
		destroy_op_array(op);
		efree_size(op, sizeof(zend_op_array));
		zend_bailout();
	} zend_end_try();

	destroy_op_array(op);
	efree_size(op, sizeof(zend_op_array));

	EG(no_extensions) = 0;
}
