// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __VALUE_H__
#define __VALUE_H__

enum {
	KIND_NULL,
	KIND_LONG,
	KIND_DOUBLE,
	KIND_BOOL,
	KIND_ARRAY,
	KIND_OBJECT,
	KIND_STRING,
	KIND_MAP
};

typedef struct _engine_value {
	zval *value;
	int kind;
} engine_value;

static inline zval *value_copy(zval *zv) {
	zval *tmp;

	ALLOC_ZVAL(tmp);
	INIT_PZVAL_COPY(tmp, zv);
	zval_copy_ctor(tmp);

	return tmp;
}

static inline void value_destroy(engine_value *val) {
	zval_dtor(val->value);
	free(val);
}

#define value_new_copy(v) value_new(value_copy(v))

engine_value *value_new(zval *zv);
int value_kind(engine_value *val);
void value_destroy(engine_value *val);

engine_value *value_create_null();
engine_value *value_create_long(long int value);
engine_value *value_create_double(double value);
engine_value *value_create_bool(bool value);
engine_value *value_create_string(char *value);

engine_value *value_create_array(unsigned int size);
void value_array_next_set(engine_value *arr, engine_value *val);
void value_array_index_set(engine_value *arr, unsigned long idx, engine_value *val);
void value_array_key_set(engine_value *arr, const char *key, engine_value *val);

engine_value *value_create_object();
void value_object_property_add(engine_value *obj, const char *key, engine_value *val);

int value_get_long(engine_value *val);
double value_get_double(engine_value *val);
bool value_get_bool(engine_value *val);
char *value_get_string(engine_value *val);

unsigned int value_array_size(engine_value *arr);
engine_value *value_array_keys(engine_value *arr);
void value_array_reset(engine_value *arr);
engine_value *value_array_next_get(engine_value *arr);
engine_value *value_array_index_get(engine_value *arr, unsigned long idx);
engine_value *value_array_key_get(engine_value *arr, char *key);

#endif