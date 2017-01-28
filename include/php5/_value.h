// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___VALUE_H___
#define ___VALUE_H___

zval *_value_init();
void _value_destroy(engine_value *val);

int _value_truth(zval *val);
void _value_set_string(zval **val, char *str);

static int _value_current_key_get(HashTable *ht, char **str_index, ulong *num_index);
static void _value_current_key_set(HashTable *ht, engine_value *val);

static void _value_array_next_get(HashTable *ht, engine_value *val);
static void _value_array_index_get(HashTable *ht, unsigned long index, engine_value *val);
static void _value_array_key_get(HashTable *ht, char *key, engine_value *val);

#endif
