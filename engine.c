// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <stdio.h>
#include <errno.h>

#include <main/php.h>
#include <main/SAPI.h>
#include <main/php_main.h>
#include <main/php_variables.h>

#include "context.h"
#include "engine.h"
#include "_cgo_export.h"

// The php.ini defaults for the Go-PHP engine.
const char engine_ini_defaults[] = {
	"expose_php = 0\n"
	"default_mimetype =\n"
	"html_errors = 0\n"
	"log_errors = 1\n"
	"display_errors = 0\n"
	"error_reporting = E_ALL\n"
	"register_argc_argv = 1\n"
	"implicit_flush = 1\n"
	"output_buffering = 0\n"
	"max_execution_time = 0\n"
	"max_input_time = -1\n\0"
};

static int engine_ub_write(const char *str, uint len) {
	engine_context *context = SG(server_context);

	int written = engineWriteOut(context, (void *) str, len);
	if (written != len) {
		php_handle_aborted_connection();
	}

	return len;
}

static int engine_header_handler(sapi_header_struct *sapi_header, sapi_header_op_enum op, sapi_headers_struct *sapi_headers) {
	engine_context *context = SG(server_context);

	switch (op) {
	case SAPI_HEADER_REPLACE:
	case SAPI_HEADER_ADD:
	case SAPI_HEADER_DELETE:
		engineSetHeader(context, op, (void *) sapi_header->header, sapi_header->header_len);
		break;
	}

	return 0;
}

static void engine_send_header(sapi_header_struct *sapi_header, void *server_context) {
	// Do nothing.
}

static char *engine_read_cookies() {
	return NULL;
}

static void engine_register_variables(zval *track_vars_array) {
	php_import_environment_variables(track_vars_array);
}

#if PHP_VERSION_ID < 70100
static void engine_log_message(char *str) {
#else
static void engine_log_message(char *str, int syslog_type_int) {
#endif
	engine_context *context = SG(server_context);

	engineWriteLog(context, (void *) str, strlen(str));
}

static sapi_module_struct engine_module = {
	"gophp-engine",              // Name
	"Go PHP Engine Library",     // Pretty Name

	NULL,                        // Startup
	php_module_shutdown_wrapper, // Shutdown

	NULL,                        // Activate
	NULL,                        // Deactivate

	_engine_ub_write,            // Unbuffered Write
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
	NULL,                        // Child Terminate

	STANDARD_SAPI_MODULE_PROPERTIES
};

php_engine *engine_init(void) {
	php_engine *engine;

	#ifdef HAVE_SIGNAL_H
		#if defined(SIGPIPE) && defined(SIG_IGN)
			signal(SIGPIPE, SIG_IGN);
		#endif
	#endif

	sapi_startup(&engine_module);

	engine_module.ini_entries = malloc(sizeof(engine_ini_defaults));
	memcpy(engine_module.ini_entries, engine_ini_defaults, sizeof(engine_ini_defaults));

	if (php_module_startup(&engine_module, NULL, 0) == FAILURE) {
		sapi_shutdown();

		errno = 1;
		return NULL;
	}

	engine = malloc((sizeof(php_engine)));

	errno = 0;
	return engine;
}

void engine_shutdown(php_engine *engine) {
	php_module_shutdown();
	sapi_shutdown();

	free(engine_module.ini_entries);
	free(engine);
}

#include "_engine.c"
