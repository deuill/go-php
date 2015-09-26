#include <errno.h>
#include <stdbool.h>
#include <main/php.h>

#include "engine.h"
#include "value.h"

void *value_create_long(long int value) {
	zval *v;

	MAKE_STD_ZVAL(v);
	ZVAL_LONG(v, value);

	return_multi((void *) v, 0);
}

void *value_create_double(double value) {
	zval *v;

	MAKE_STD_ZVAL(v);
	ZVAL_DOUBLE(v, value);

	return_multi((void *) v, 0);
}

void *value_create_bool(bool value) {
	zval *v;

	MAKE_STD_ZVAL(v);
	ZVAL_BOOL(v, value);

	return_multi((void *) v, 0);
}

void *value_create_string(char *value) {
	zval *v;

	MAKE_STD_ZVAL(v);
	ZVAL_STRING(v, value, 1);

	return_multi((void *) v, 0);
}

void value_destroy(void *zvalptr) {
	zval_dtor((zval *) zvalptr);
}