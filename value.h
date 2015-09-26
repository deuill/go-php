#ifndef VALUE_H
#define VALUE_H

void *value_create_long(long int value);
void *value_create_double(double value);
void *value_create_bool(bool value);
void *value_create_string(char *value);
void value_destroy(void *zvalptr);

#endif