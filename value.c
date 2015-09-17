#include "value.h"

void value_destroy(void *val) {
	zval_dtor((zval *) val);
}