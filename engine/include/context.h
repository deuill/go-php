// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "_context.h"

typedef struct _engine_context {
} engine_context;

engine_context *context_new();
void context_exec(engine_context *context, char *filename);
void *context_eval(engine_context *context, char *script);
void context_bind(engine_context *context, char *name, void *value);
void context_destroy(engine_context *context);

#endif