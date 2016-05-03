// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

static void context_bind_zval(char *name, zval *value) {
	ZEND_SET_SYMBOL(EG(active_symbol_table), name, value);
}
