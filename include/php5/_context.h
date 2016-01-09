// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___CONTEXT_H___
#define ___CONTEXT_H___

static inline void context_value_bind(char *name, zval *val) {
	ZEND_SET_SYMBOL(EG(active_symbol_table), name, val);
}

#endif