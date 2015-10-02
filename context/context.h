#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct _engine_context {
	#ifdef ZTS
	void *ptsrm_ls; // Pointer to TSRM local storage.
	#endif

	void *parent; // Pointer to parent Go context, used for passing to callbacks.
} engine_context;

engine_context *context_new(void *parent);
void context_exec(engine_context *context, char *filename);
void context_bind(engine_context *context, char *name, void *zvalptr);
void context_destroy(engine_context *context);

#endif