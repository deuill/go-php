// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___RECEIVER_H___
#define ___RECEIVER_H___

static zval *_receiver_get(zval *object, zval *member, int type, void **cache_slot, zval *retval);
static void _receiver_set(zval *object, zval *member, zval *value, void **cache_slot);
static int _receiver_exists(zval *object, zval *member, int check, void **cache_slot);

static int _receiver_method_call(zend_string *method, zend_object *object, INTERNAL_FUNCTION_PARAMETERS);
static zend_function *_receiver_method_get(zend_object **object, zend_string *name, const zval *key);
static zend_function *_receiver_constructor_get(zend_object *object);

static void _receiver_free(zend_object *object);
static zend_object *_receiver_init(zend_class_entry *class_type);
static void _receiver_destroy(char *name);

static engine_receiver *_receiver_this(zval *object);
static void _receiver_handlers_set(zend_object_handlers *handlers);
char *_receiver_get_name(engine_receiver *rcvr);

#endif
