#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct _engine_context {
	php_engine *engine; // Parent engine instance.
	void *parent;       // Pointer to parent Go context, used for passing to callbacks.
} engine_context;

engine_context *context_new(php_engine *engine, void *parent);
void context_exec(engine_context *context, char *filename);
void context_destroy(engine_context *context);

#endif