// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef VALUE_H
#define VALUE_H

typedef struct _engine_value {
	zval *value;
	int kind;
} engine_value;

engine_value *value_new(zval *zv);
int value_kind(engine_value *val);
void value_destroy(engine_value *val);

engine_value *value_create_long(long int value);
engine_value *value_create_double(double value);
engine_value *value_create_bool(bool value);
engine_value *value_create_string(char *value);

engine_value *value_create_array(unsigned int size);
void value_array_set_index(engine_value *arr, unsigned long idx, engine_value *val);
void value_array_set_key(engine_value *arr, const char *key, engine_value *val);

engine_value *value_create_object();
void value_object_add_property(engine_value *obj, const char *key, engine_value *val);

int value_get_long(engine_value *val);
double value_get_double(engine_value *val);
bool value_get_bool(engine_value *val);
char *value_get_string(engine_value *val);

#endif