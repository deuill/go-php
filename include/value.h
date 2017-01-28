// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __VALUE_H__
#define __VALUE_H__

typedef struct _engine_value {
	zval *internal;
	int  kind;
} engine_value;

enum {
	KIND_NULL,
	KIND_LONG,
	KIND_DOUBLE,
	KIND_BOOL,
	KIND_STRING,
	KIND_ARRAY,
	KIND_MAP,
	KIND_OBJECT
};

engine_value *value_new();
void value_copy(zval *dst, zval *src);
int value_kind(engine_value *val);

void value_set_null(engine_value *val);
void value_set_long(engine_value *val, long int num);
void value_set_double(engine_value *val, double num);
void value_set_bool(engine_value *val, bool status);
void value_set_string(engine_value *val, char *str);
void value_set_array(engine_value *val, unsigned int size);
void value_set_object(engine_value *val);
void value_set_zval(engine_value *val, zval *src);

void value_array_next_set(engine_value *arr, engine_value *val);
void value_array_index_set(engine_value *arr, unsigned long idx, engine_value *val);
void value_array_key_set(engine_value *arr, const char *key, engine_value *val);
void value_object_property_set(engine_value *obj, const char *key, engine_value *val);

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

#include "_value.h"

#endif
