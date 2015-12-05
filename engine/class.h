// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __CLASS_H__
#define __CLASS_H__

#define engine_class_set_pointer(ce, name, value) \
	zend_declare_property_long(ce, name, sizeof(name) - 1, (long int) value, \
	ZEND_ACC_STATIC | ZEND_ACC_PRIVATE TSRMLS_CC);

#define engine_class_get_pointer(ce, name) \
	(void *) Z_LVAL_P(zend_read_static_property(ce, name, sizeof(name) - 1, 1))

bool engine_class_define(void *rcvr, char *name);

#endif