// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <errno.h>
#include <stdbool.h>

#include <main/php.h>
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

	context->ctx = ctx;
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
	int status;
	zval tmp;

	// Attempt to evaluate inline script.
	zend_first_try {
		status = zend_eval_string(script, &tmp, "gophp-engine");
	} zend_catch {
		errno = 1;
		return NULL;
	} zend_end_try();

	if (status == FAILURE) {
		errno = 1;
		return NULL;
	}

	zval *result = malloc(sizeof(zval));
	value_copy(result, &tmp);

	errno = 0;
	return result;
}

void context_bind(engine_context *context, char *name, void *value) {
	engine_value *v = (engine_value *) value;
	CONTEXT_VALUE_BIND(name, &v->value);
}

void context_destroy(engine_context *context) {
	php_request_shutdown(NULL);

	SG(server_context) = NULL;
	free(context);
}