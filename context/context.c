// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <errno.h>
#include <stdbool.h>

#include <main/php.h>
#include <main/SAPI.h>
#include <main/php_main.h>

#include "value.h"
#include "context.h"

engine_context *context_new(void *ctx) {
	engine_context *context;

	// Initialize context.
	context = malloc((sizeof(engine_context)));
	if (context == NULL) {
		errno = 1;
		return NULL;
	}

	#ifdef ZTS
		TSRMLS_FETCH();
		context->tsrm_ls = tsrm_ls;
	#endif

	context->ctx = ctx;
	SG(server_context) = context;

	// Initialize request lifecycle.
	if (php_request_startup(TSRMLS_C) == FAILURE) {
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

	#ifdef ZTS
		void ***tsrm_ls = context->tsrm_ls;
	#endif

	// Attempt to execute script file.
	zend_first_try {
		zend_file_handle script;

		script.type = ZEND_HANDLE_FILENAME;
		script.filename = filename;
		script.opened_path = NULL;
		script.free_filename = 0;

		ret = php_execute_script(&script TSRMLS_CC);
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
	int ret;

	#ifdef ZTS
		void ***tsrm_ls = context->tsrm_ls;
	#endif

	zval *retval;
	MAKE_STD_ZVAL(retval);

	// Attempt to evaluate inline script.
	zend_first_try {
		ret = zend_eval_string(script, retval, "gophp-engine" TSRMLS_CC);
	} zend_catch {
		zval_dtor(retval);
		errno = 1;
		return NULL;
	} zend_end_try();

	if (ret == FAILURE) {
		zval_dtor(retval);
		errno = 1;
		return NULL;
	}

	errno = 0;
	return (void *) retval;
}

void context_bind(engine_context *context, char *name, void *value) {
	engine_value *v = (engine_value *) value;

	#ifdef ZTS
		void ***tsrm_ls = context->tsrm_ls;
	#endif

	ZEND_SET_SYMBOL(EG(active_symbol_table), name, v->value);

	errno = 0;
	return;
}

void context_destroy(engine_context *context) {
	php_request_shutdown((void *) 0);

	SG(server_context) = NULL;
	free(context);
}