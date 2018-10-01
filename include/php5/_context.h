// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___CONTEXT_H___
#define ___CONTEXT_H___

static void _context_bind(char *name, zval *value);
static void _context_ini(char *name, char *value);
static void _context_eval(zend_op_array *op, zval *ret);

#endif
