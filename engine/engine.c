// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <stdio.h>
#include <errno.h>

#include <main/php.h>
#include <main/SAPI.h>
#include <main/php_main.h>
#include <main/php_variables.h>
#include <TSRM/TSRM.h>

#include "context.h"
#include "engine.h"
#include "_cgo_export.h"

const char engine_ini_defaults[] = {
	"expose_php = 0\n"
	"default_mimetype =\n"
	"html_errors = 0\n"
	"register_argc_argv = 1\n"
	"implicit_flush = 1\n"
	"output_buffering = 0\n"
	"max_execution_time = 0\n"
	"max_input_time = -1\n\0"
};

static int engine_ub_write(const char *str, uint str_length TSRMLS_DC) {
	engine_context *context = (engine_context *) SG(server_context);

	int written = engine_context_write(context->parent, (void *) str, str_length);
	if (written != str_length) {
		php_handle_aborted_connection();
	}

	return written;
}

static int engine_header_handler(sapi_header_struct *sapi_header, sapi_header_op_enum op, sapi_headers_struct *sapi_headers TSRMLS_DC) {
	engine_context *context = (engine_context *) SG(server_context);

	switch (op) {
	case SAPI_HEADER_ADD: case SAPI_HEADER_REPLACE: case SAPI_HEADER_DELETE:
		engine_context_header(context->parent, op, (void *) sapi_header->header, sapi_header->header_len);
		break;
	}

	return 0;
}

static void engine_send_header(sapi_header_struct *sapi_header, void *server_context TSRMLS_DC) {
	// Do nothing.
}

static char *engine_read_cookies(TSRMLS_D) {
	return NULL;
}

static void engine_register_variables(zval *track_vars_array TSRMLS_DC) {
	php_import_environment_variables(track_vars_array TSRMLS_CC);
}

static void engine_log_message(char *str TSRMLS_DC) {
	engine_context *context = (engine_context *) SG(server_context);

	engine_context_log(context->parent, (void *) str, strlen(str));
}

static sapi_module_struct engine_module = {
	"gophp-engine",              // Name
	"Go PHP Engine Library",     // Pretty Name

	NULL,                        // Startup
	php_module_shutdown_wrapper, // Shutdown

	NULL,                        // Activate
	NULL,                        // Deactivate

	engine_ub_write,             // Unbuffered Write
	NULL,                        // Flush
	NULL,                        // Get UID
	NULL,                        // Getenv

	php_error,                   // Error Handler

	engine_header_handler,       // Header Handler
	NULL,                        // Send Headers Handler
	engine_send_header,          // Send Header Handler

	NULL,                        // Read POST Data
	engine_read_cookies,         // Read Cookies

	engine_register_variables,   // Register Server Variables
	engine_log_message,          // Log Message
	NULL,                        // Get Request Time

	STANDARD_SAPI_MODULE_PROPERTIES
};

php_engine *engine_init(void) {
	php_engine *engine;

	#ifdef HAVE_SIGNAL_H
		#if defined(SIGPIPE) && defined(SIG_IGN)
			signal(SIGPIPE, SIG_IGN);
		#endif
	#endif

	#ifdef ZTS
		void ***tsrm_ls = NULL;
		tsrm_startup(1, 1, 0, NULL);
		tsrm_ls = ts_resource(0);
		*ptsrm_ls = tsrm_ls;
	#endif

	sapi_startup(&engine_module);

	engine_module.ini_entries = malloc(sizeof(engine_ini_defaults));
	memcpy(engine_module.ini_entries, engine_ini_defaults, sizeof(engine_ini_defaults));

	if (php_module_startup(&engine_module, NULL, 0) == FAILURE) {
		sapi_shutdown();

		#ifdef ZTS
			tsrm_shutdown();
		#endif

		errno = 1;
		return NULL;
	}

	engine = (php_engine *) malloc((sizeof(php_engine)));

	#ifdef ZTS
		engine->tsrm_ls = tsrm_ls;
	#endif

	errno = 0;
	return engine;
}

void engine_shutdown(php_engine *engine) {
	#ifdef ZTS
		void ***tsrm_ls = engine->tsrm_ls;
	#endif

	php_module_shutdown(TSRMLS_C);
	sapi_shutdown();

	#ifdef ZTS
		tsrm_shutdown();
	#endif

	free(engine_module.ini_entries);
	free(engine);
}