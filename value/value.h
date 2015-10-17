// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef VALUE_H
#define VALUE_H

void *value_create_long(long int value);
void *value_create_double(double value);
void *value_create_bool(bool value);
void *value_create_string(char *value);
void *value_create_array(unsigned int size);
void value_array_set_index(void *arr, unsigned long idx, void *val);
void value_array_set_key(void *arr, const char *key, void *val);
void *value_create_object();
void value_object_add_property(void *obj, const char *key, void *val);
void value_destroy(void *zvalptr);

#endif