// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

static size_t engine_ub_write_proxy(const char *str, size_t len) {
	return engine_ub_write(str, len);
}
