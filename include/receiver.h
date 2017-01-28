// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __RECEIVER_H__
#define __RECEIVER_H__

typedef struct _engine_receiver {
	zend_object obj;
} engine_receiver;

void receiver_define(char *name);
void receiver_destroy(char *name);

#include "_receiver.h"

#endif
