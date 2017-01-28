// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___RECEIVER_H___
#define ___RECEIVER_H___

static zval *_receiver_get(zval *object, zval *member, int type, const zend_literal *key);
static void _receiver_set(zval *object, zval *member, zval *value, const zend_literal *key);
static int _receiver_exists(zval *object, zval *member, int check, const zend_literal *key);

static int _receiver_method_call(const char *method, INTERNAL_FUNCTION_PARAMETERS);
static zend_function *_receiver_method_get(zval **object, char *name, int len, const zend_literal *key);
static zend_function *_receiver_constructor_get(zval *object);

static void _receiver_free(void *object);
static zend_object_value _receiver_init(zend_class_entry *class_type);
static void _receiver_destroy(char *name);

static engine_receiver *_receiver_this(zval *object);
static void _receiver_handlers_set(zend_object_handlers *handlers);
char *_receiver_get_name(engine_receiver *rcvr);

#endif
