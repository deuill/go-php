#include <errno.h>
#include <stdbool.h>

#include <main/php.h>

#include "value.h"

void *value_create_long(long int value) {
	zval *v;

	MAKE_STD_ZVAL(v);
	ZVAL_LONG(v, value);

	return (void *) v;
}

void *value_create_double(double value) {
	zval *v;

	MAKE_STD_ZVAL(v);
	ZVAL_DOUBLE(v, value);

	return (void *) v;
}

void *value_create_bool(bool value) {
	zval *v;

	MAKE_STD_ZVAL(v);
	ZVAL_BOOL(v, value);

	return (void *) v;
}

void *value_create_string(char *value) {
	zval *v;

	MAKE_STD_ZVAL(v);
	ZVAL_STRING(v, value, 1);

	return (void *) v;
}

void *value_create_array(unsigned int size) {
	zval *v;

	MAKE_STD_ZVAL(v);
	array_init_size(v, size);

	return (void *) v;
}

void value_array_set_index(void *arr, unsigned long idx, void *val) {
	add_index_zval((zval *) arr, idx, (zval *) val);
}

void value_array_set_key(void *arr, const char *key, void *val) {
	add_assoc_zval((zval *) arr, key, (zval *) val);
}

void value_destroy(void *zvalptr) {
	zval_dtor((zval *) zvalptr);
}