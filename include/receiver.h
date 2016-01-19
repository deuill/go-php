// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __RECEIVER_H__
#define __RECEIVER_H__

#include "_receiver.h"

#define RECEIVER_POINTER(ce, name) \
	(void *) Z_LVAL_P(zend_read_static_property(ce, name, sizeof(name) - 1, 1))

#define RECEIVER_POINTER_SET(ce, name, ptr) \
	zend_declare_property_long(ce, name, sizeof(name) - 1, (long int) ptr, \
	ZEND_ACC_STATIC | ZEND_ACC_PRIVATE)

typedef struct _engine_receiver {
	zend_object obj;
	void *rcvr;
} engine_receiver;

void receiver_define(char *name, void *rcvr);
void receiver_destroy(char *name);

#endif