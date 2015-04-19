#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "json.h"

#define JS_BUFLEN_SIZE 1024

#define JS_FORMAT_BUFFER(format, ...) do { \
    int retval = 0; \
    retval = snprintf(print_buffer, JS_BUFLEN_SIZE, \
                      (format), __VA_ARGS__); \
    assert(retval >= 0); \
  } while (0)

static char print_buffer[JS_BUFLEN_SIZE];

static void print_to_stdout(const void *call_arg, const char *output)
{
    printf("%s", output);
}

static void indent(js_serialiser_t *s)
{
    int i;
    for (i = 0; i < s->level; i++) {
        s->print(s->call_arg, "  ");
    }
}

static void link_to_previous(js_serialiser_t *s)
{
    if (s->attribute_count++ > 0) {
        s->print(s->call_arg, ",\n");
    }
}


void js_document(js_serialiser_t *s)
{
    memset(s, 0, sizeof(*s));
    s->attribute_count = 0;
    s->print = print_to_stdout;
    s->call_arg = NULL;

    s->print(s->call_arg, "{\n");
    s->level = 1;
}

void js_document3(js_serialiser_t *s, 
                  print_function_t print_function, 
                  void *call_arg)
{
    js_document(s);
    s->print = print_function;
    s->call_arg = call_arg;
}

void js_document_end(js_serialiser_t *s)
{
    s->print(s->call_arg, "\n}");
}

void js_object(js_serialiser_t *s, char *name)
{
    link_to_previous(s);
    indent(s);
 
    JS_FORMAT_BUFFER("\"%s\": {\n", name);
    s->print(s->call_arg, print_buffer);

    s->attribute_count = 0;
    s->level++;
}

void js_object_end(js_serialiser_t *s)
{
    s->print(s->call_arg, "\n");

    s->level--;
    indent(s);
    s->print(s->call_arg, "}");

}

void js_number(struct js_serialiser *s, char *name, double value)
{
    link_to_previous(s);
    indent(s);

    JS_FORMAT_BUFFER("\"%s\": %1.1f", name, value);
    s->print(s->call_arg, print_buffer);
}

void js_int_number(struct js_serialiser *s, char *name, long value)
{
    link_to_previous(s);
    indent(s);

    JS_FORMAT_BUFFER("\"%s\": %ld", name, value);
    s->print(s->call_arg, print_buffer);
}

void js_string(js_serialiser_t *s, char *name, char *value)
{
    link_to_previous(s);
    indent(s);

    JS_FORMAT_BUFFER("\"%s\": \"%s\"", name, value);
    s->print(s->call_arg, print_buffer);
}

void js_boolean(js_serialiser_t *s, char *name, int boolean)
{
    link_to_previous(s);
    indent(s);

    JS_FORMAT_BUFFER("\"%s\": %s", name, boolean ? "true" : "false");
    s->print(s->call_arg, print_buffer);
}


int main(void)
{
  js_serialiser_t s;

  js_document(&s);
    js_object(&s, "outer");
      js_int_number(&s, "name", 1);
      js_string(&s, "foo", "bar");
      js_object(&s, "example");
        js_int_number(&s, "name", 1);
        js_number(&s, "foo", 2);
      js_object_end(&s);
      js_number(&s, "bar", 3.005);
    js_object_end(&s);
    js_boolean(&s, "true", 1);
    js_boolean(&s, "false", 0);
  js_document_end(&s);

  printf("\n");

  return 0;
}

