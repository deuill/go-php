#include <errno.h>

#include <main/php.h>
#include <main/SAPI.h>
#include <main/php_main.h>

#include "context.h"

engine_context *context_new(void *parent) {
	engine_context *context;

	// Initialize context.
	context = (engine_context *) malloc((sizeof(engine_context)));
	if (context == NULL) {
		errno = 1;
		return NULL;
	}

	#ifdef ZTS
		TSRMLS_FETCH();
		context->ptsrm_ls = &tsrm_ls;
	#endif

	context->parent = parent;
	SG(server_context) = (void *) context;

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
	#ifdef ZTS
		void ***tsrm_ls = *context->ptsrm_ls;
	#endif

	// Attempt to execute script file.
	zend_first_try {
		zend_file_handle script;

		script.type = ZEND_HANDLE_FILENAME;
		script.filename = filename;
		script.opened_path = NULL;
		script.free_filename = 0;

		php_execute_script(&script TSRMLS_CC);
	} zend_end_try();

	errno = 0;
	return NULL;
}

void context_bind(engine_context *context, char *name, void *zvalptr) {
	zval *value = (zval *) zvalptr;

	#ifdef ZTS
		void ***tsrm_ls = *context->ptsrm_ls;
	#endif

	ZEND_SET_SYMBOL(EG(active_symbol_table), name, value);

	errno = 0;
	return NULL;
}

void context_destroy(engine_context *context) {
	php_request_shutdown((void *) 0);

	SG(server_context) = NULL;
	free(context);
}