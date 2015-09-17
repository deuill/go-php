#ifndef ENGINE_H
#define ENGINE_H

#define return_multi(value, error) errno = error; return value

typedef struct _php_engine {
	#ifdef ZTS
	void ***tsrm_ls; // Local storage for thread-safe operations, used across the PHP engine.
	#endif
} php_engine;

php_engine *engine_init(void);
void engine_shutdown(php_engine *engine);

#endif