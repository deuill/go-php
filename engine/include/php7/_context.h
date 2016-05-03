// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___CONTEXT_H___
#define ___CONTEXT_H___

static void context_bind_proxy(char *name, zval *value);
static void context_eval_proxy(zend_op_array *op, zval *ret);

#endif
