#ifndef VALUE_H
#define VALUE_H

void *value_long(long int value);
void *value_double(double value);
void *value_bool(bool value);
void *value_string(char *value);
void value_destroy(void *zvalptr);

#endif