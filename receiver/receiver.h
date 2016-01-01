// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __RECEIVER_H__
#define __RECEIVER_H__

typedef struct _engine_receiver {
	zend_object obj;
	void *rcvr;
} engine_receiver;

#define receiver_get_pointer(ce, name) \
	(void *) Z_LVAL_P(zend_read_static_property(ce, name, sizeof(name) - 1, 1 \
	TSRMLS_CC))

#define receiver_set_pointer(ce, name, ptr) \
	zend_declare_property_long(ce, name, sizeof(name) - 1, (long int) ptr, \
	ZEND_ACC_STATIC | ZEND_ACC_PRIVATE TSRMLS_CC)

void receiver_define(char *name, void *rcvr);

#endif