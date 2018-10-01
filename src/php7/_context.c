// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

static void _context_bind(char *name, zval *value) {
	zend_hash_str_update(&EG(symbol_table), name, strlen(name), value);
}

static void _context_ini(char *name, char *value) {
	zend_string *n, *v;

	// Use "permanent" strings
	n = zend_string_init(name, strlen(name), 1);
	v = zend_string_init(value, strlen(value), 1);
	if (zend_alter_ini_entry_ex(n, v, PHP_INI_USER, PHP_INI_STAGE_RUNTIME, 0) == FAILURE) {
		errno = 1;
	}
}

static void _context_eval(zend_op_array *op, zval *ret, int *exit) {
	EG(no_extensions) = 1;

	zend_first_try {
		ZVAL_NULL(ret);
		zend_execute(op, ret);
		*exit = -1;
	} zend_catch {
		*exit = EG(exit_status);
	} zend_end_try();

	destroy_op_array(op);
	efree_size(op, sizeof(zend_op_array));

	EG(no_extensions) = 0;
}
