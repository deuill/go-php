// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __ENGINE_H__
#define __ENGINE_H__

#if PHP_MAJOR_VERSION >= 7
	#define UBWRITE_RETURN  size_t
	#define UBWRITE_STR_LEN size_t
#else
	#define UBWRITE_RETURN  int
	#define UBWRITE_STR_LEN uint
#endif

typedef struct _php_engine {
	#ifdef ZTS
		void ***tsrm_ls;
	#endif
} php_engine;

php_engine *engine_init(void);
void engine_shutdown(php_engine *engine);

#endif