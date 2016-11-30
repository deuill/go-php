// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <main/php.h>
#include <main/php_main.h>

#include "value.h"
#include "context.h"

engine_context *context_new() {
	engine_context *context;

	// Initialize context.
	context = malloc((sizeof(engine_context)));
	if (context == NULL) {
		errno = 1;
		return NULL;
	}

	SG(server_context) = context;

	// Initialize request lifecycle.
	if (php_request_startup() == FAILURE) {
		SG(server_context) = NULL;
		free(context);

		errno = 1;
		return NULL;
	}

	errno = 0;
	return context;
}

void context_exec(engine_context *context, char *filename) {
	int ret;

	// Attempt to execute script file.
	zend_first_try {
		zend_file_handle script;

		script.type = ZEND_HANDLE_FILENAME;
		script.filename = filename;
		script.opened_path = NULL;
		script.free_filename = 0;

		ret = php_execute_script(&script);
	} zend_catch {
		errno = 1;
		return;
	} zend_end_try();

	if (ret == FAILURE) {
		errno = 1;
		return;
	}

	errno = 0;
	return;
}

void *context_eval(engine_context *context, char *script) {
	zval *str = _value_init();
	_value_set_string(&str, script);

	// Compile script value.
	uint32_t compiler_options = CG(compiler_options);

	CG(compiler_options) = ZEND_COMPILE_DEFAULT_FOR_EVAL;
	zend_op_array *op = zend_compile_string(str, "gophp-engine");
	CG(compiler_options) = compiler_options;

	zval_dtor(str);

	// Return error if script failed to compile.
	if (!op) {
		errno = 1;
		return NULL;
	}

	// Attempt to execute compiled string.
	zval tmp;
	_context_eval(op, &tmp);

	// Allocate result value and copy temporary execution result in.
	zval *result = malloc(sizeof(zval));
	value_copy(result, &tmp);

	errno = 0;
	return result;
}

void context_bind(engine_context *context, char *name, void *value) {
	engine_value *v = (engine_value *) value;
	_context_bind(name, v->internal);
}

void context_destroy(engine_context *context) {
	php_request_shutdown(NULL);

	SG(server_context) = NULL;
	free(context);
}

#include "_context.c"
