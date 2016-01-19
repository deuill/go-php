// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ___CONTEXT_H___
#define ___CONTEXT_H___

#define CONTEXT_VALUE_BIND(n, v) do {               \
	ZEND_SET_SYMBOL(EG(active_symbol_table), n, v); \
} while (0)

#endif