// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

zval *_value_init() {
	zval *tmp = NULL;

	MAKE_STD_ZVAL(tmp);
	ZVAL_NULL(tmp);

	return tmp;
}

// Destroy and free engine value.
void _value_destroy(engine_value *val) {
	zval_dtor(val->internal);
	free(val);
}

int _value_truth(zval *val) {
	return (Z_TYPE_P(val) != IS_BOOL) ? -1 : ((Z_BVAL_P(val)) ? 1 : 0);
}

void _value_set_string(zval **val, char *str) {
	ZVAL_STRING(*val, str, 1);
}

static int _value_current_key_get(HashTable *ht, char **str_index, ulong *num_index) {
	return zend_hash_get_current_key(ht, str_index, num_index, 0);
}

static void _value_current_key_set(HashTable *ht, engine_value *val) {
	zval *tmp;

	MAKE_STD_ZVAL(tmp);
	zend_hash_get_current_key_zval(ht, tmp);
	add_next_index_zval(val->internal, tmp);
}

static void _value_array_next_get(HashTable *ht, engine_value *val) {
	zval **tmp = NULL;

	if (zend_hash_get_current_data(ht, (void **) &tmp) == SUCCESS) {
		value_set_zval(val, *tmp);
		zend_hash_move_forward(ht);
	}
}

static void _value_array_index_get(HashTable *ht, unsigned long index, engine_value *val) {
	zval **tmp = NULL;

	if (zend_hash_index_find(ht, index, (void **) &tmp) == SUCCESS) {
		value_set_zval(val, *tmp);
	}
}

static void _value_array_key_get(HashTable *ht, char *key, engine_value *val) {
	zval **tmp = NULL;

	if (zend_hash_find(ht, key, strlen(key) + 1, (void **) &tmp) == SUCCESS) {
		value_set_zval(val, *tmp);
	}
}
