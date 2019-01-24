#pragma once

#define BUFW_STRUCT(type, name, typei) \
type * name; \
typei name##_i; \
typei name##_n

#define BUFW_CALLOC(obj, type, name, n) \
obj.name = (type *) calloc (n, sizeof (type)); \
obj.name##_n = n; \
obj.name##_i = 0

#define BUFW_LAST(obj, name) \
obj.name + obj.name##_n
