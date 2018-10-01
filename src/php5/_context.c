// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

static void _context_bind(char *name, zval *value) {
	ZEND_SET_SYMBOL(EG(active_symbol_table), name, value);
}

static void _context_ini(char *name, char *value) {
	if (zend_alter_ini_entry_ex(name, strlen(name) + 1, value, strlen(value) + 1, PHP_INI_USER, PHP_INI_STAGE_RUNTIME, 0) == FAILURE) {
		errno = 1;
	}
}

static void _context_eval(zend_op_array *op, zval *ret, int *exit) {
	zend_op_array *oparr = EG(active_op_array);
	zval *retval = NULL;
	zval **retvalptr = EG(return_value_ptr_ptr);
	zend_op **opline = EG(opline_ptr);
	int interact = CG(interactive);

	EG(return_value_ptr_ptr) = &retval;
	EG(active_op_array) = op;
	EG(no_extensions) = 1;

	if (!EG(active_symbol_table)) {
		zend_rebuild_symbol_table();
	}

	CG(interactive) = 0;

	zend_try {
		zend_execute(op);
		*exit = -1;
	} zend_catch {
		*exit = EG(exit_status);
	} zend_end_try();

	destroy_op_array(op);
	efree(op);

	CG(interactive) = interact;

	if (retval) {
		ZVAL_COPY_VALUE(ret, retval);
		zval_copy_ctor(ret);
	} else {
		ZVAL_NULL(ret);
	}

	EG(no_extensions) = 0;
	EG(opline_ptr) = opline;
	EG(active_op_array) = oparr;
	EG(return_value_ptr_ptr) = retvalptr;
}
