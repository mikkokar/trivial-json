#ifndef JSON_H
#define JSON_H

struct js_serialiser
{
    int level;
    int attribute_count;

    void (*print_function)(const void *call_arg, const char *output); 
    void *call_arg;
};
typedef struct js_serialiser js_serialiser_t;

void js_document(js_serialiser_t *s);
void js_document_end(js_serialiser_t *s);
void js_object(js_serialiser_t *s, char *name);
void js_object_end(js_serialiser_t *s);
void js_number(struct js_serialiser *s, char *name, double value);
void js_int_number(struct js_serialiser *s, char *name, long value);
void js_string(js_serialiser_t *s, char *name, char *value);
void js_boolean(js_serialiser_t *s, char *name, int boolean);

#endif

