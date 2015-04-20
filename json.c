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

static void print_to_stdout(void *call_arg, const char *output)
{
    printf("%s", output);
}

struct print_buffer {
    char *output;
    int  max_length;
    int  next_offset;
};
typedef struct print_buffer print_buffer_t;

static void print_to_buffer(void *call_arg, const char *output)
{
    int written = 0;
    int remaining = 0;

    print_buffer_t *context = call_arg;

    remaining = context->max_length - context->next_offset;

    // printf("at offset %03d, wrote: [%s]\n", context->next_offset, output);

    written = snprintf(context->output + context->next_offset, remaining,
                       "%s", output);

    if (context->next_offset + written > context->max_length) {
        printf("something fishy going on:\n");
        printf("next_offset: %d\nwritten: %d\nmax_length: %d\n",
               context->next_offset, written, context->max_length);
    }
    assert(context->next_offset + written <= context->max_length);

    context->next_offset += written;
}

static void push_scopes(js_serialiser_t *s)
{
    s->level++;
    s->scopes[s->level].in_array = 0;
    s->scopes[s->level].attribute_count = 0;
}

static void pop_scopes(js_serialiser_t *s)
{
    memset(&s->scopes[s->level], 0, sizeof(s->scopes[s->level]));
    s->level--;
}

static int in_array(js_serialiser_t *s)
{
    return s->scopes[s->level].in_array;
}

static void indent_if_pretty(js_serialiser_t *s)
{
    if (s->pretty_print) {
        int i;
        for (i = 0; i < s->level; i++) {
            s->print(s->call_arg, "  ");
        }
    }
}

static void newline_if_pretty(js_serialiser_t *s)
{
    if (s->pretty_print) {
        s->print(s->call_arg, "\n");
    }
}

static void link_to_previous(js_serialiser_t *s)
{
    if (s->scopes[s->level].attribute_count++ > 0) {
        s->print(s->call_arg, ",");
        newline_if_pretty(s);
    }
}

static void js_document_init(js_serialiser_t *s)
{
    memset(s, 0, sizeof(*s));
    s->scopes[s->level].attribute_count = 0;
    s->print = print_to_stdout;
    s->call_arg = NULL;
}

static void js_document_print(js_serialiser_t *s)
{
    s->print(s->call_arg, "{");
    newline_if_pretty(s);

    push_scopes(s);
}

void js_document(js_serialiser_t *s)
{
    js_document_init(s);
    js_document_print(s);
}

void js_document4(js_serialiser_t *s, 
                  print_function_t print_function, 
                  void *call_arg,
                  int pretty_print)
{
    js_document_init(s);

    s->print = print_function;
    s->call_arg = call_arg;
    s->pretty_print = pretty_print;

    js_document_print(s);
}

void js_document_end(js_serialiser_t *s)
{
    pop_scopes(s);

    newline_if_pretty(s);
    s->print(s->call_arg, "}");
}

void js_object(js_serialiser_t *s, char *name)
{
    link_to_previous(s);
    indent_if_pretty(s);
 
    if (in_array(s)) {
        s->print(s->call_arg, "{");       
    }
    else {
        JS_FORMAT_BUFFER("\"%s\": {", name);
        s->print(s->call_arg, print_buffer);
    }

    newline_if_pretty(s);

    push_scopes(s);
}

void js_object_end(js_serialiser_t *s)
{
    newline_if_pretty(s);

    pop_scopes(s);

    indent_if_pretty(s);
    s->print(s->call_arg, "}");
}

void js_array(js_serialiser_t *s, char *name)
{
    link_to_previous(s);
    indent_if_pretty(s);
 
    if (in_array(s)) {
        s->print(s->call_arg, "[");
    }
    else {
        JS_FORMAT_BUFFER("\"%s\": [", name);
        s->print(s->call_arg, print_buffer);
    }
    newline_if_pretty(s);

    push_scopes(s);
    s->scopes[s->level].in_array = 1; 
}

void js_array_end(js_serialiser_t *s)
{
    // Todo: clear the old stack frame
    pop_scopes(s);

    indent_if_pretty(s);
    s->print(s->call_arg, "]");
}

void js_number(struct js_serialiser *s, char *name, double value)
{
    link_to_previous(s);
    indent_if_pretty(s);

    if (in_array(s)) {
        JS_FORMAT_BUFFER("%1.1f", value);
        s->print(s->call_arg, print_buffer);
    }
    else {
        JS_FORMAT_BUFFER("\"%s\": %1.1f", name, value);
        s->print(s->call_arg, print_buffer);
     }
}

void js_int_number(struct js_serialiser *s, char *name, long value)
{
    link_to_previous(s);
    indent_if_pretty(s);

    if (in_array(s)) {
        JS_FORMAT_BUFFER("%ld", value);
        s->print(s->call_arg, print_buffer);
    }
    else {
        JS_FORMAT_BUFFER("\"%s\": %ld", name, value);
        s->print(s->call_arg, print_buffer);
    }
}

void js_string(js_serialiser_t *s, char *name, char *value)
{
    link_to_previous(s);
    indent_if_pretty(s);

    if (in_array(s)) {
        JS_FORMAT_BUFFER("\"%s\"", value);
        s->print(s->call_arg, print_buffer);
    }
    else {
        JS_FORMAT_BUFFER("\"%s\": \"%s\"", name, value);
        s->print(s->call_arg, print_buffer);
     }
}

void js_boolean(js_serialiser_t *s, char *name, int boolean)
{
    link_to_previous(s);
    indent_if_pretty(s);

    if (in_array(s)) {
        JS_FORMAT_BUFFER("%s", boolean ? "true" : "false");
        s->print(s->call_arg, print_buffer);
    }
    else {
        JS_FORMAT_BUFFER("\"%s\": %s", name, boolean ? "true" : "false");
        s->print(s->call_arg, print_buffer);
    }
}

static void init_string_printer(print_buffer_t *printer_param, 
                                char *buffer,
                                int buf_size)
{
    memset(buffer, 0, buf_size);
    memset(printer_param, 0, sizeof(*printer_param));

    printer_param->output = buffer;
    printer_param->max_length = buf_size;
}


void test_array_elements_do_not_have_names()
{
    char buffer[JS_BUFLEN_SIZE];
    print_buffer_t printer_parameter;
    js_serialiser_t s;

    init_string_printer(&printer_parameter, buffer, JS_BUFLEN_SIZE);

    js_document4(&s, print_to_buffer, &printer_parameter, 0);
    js_array(&s, "foo");
      js_int_number(&s, "i", 1);
      js_number(&s, "n", 2);
      js_boolean(&s, "bool", 1);
      js_object(&s, "object");
      js_object_end(&s);
      js_string(&s, "str", "foo");
    js_array_end(&s);
    js_document_end(&s);

    printf("output:\n%s\n", buffer);
    assert(0 == strncmp(buffer, "{\"foo\": [1,2.0,true,{},\"foo\"]}",
                        JS_BUFLEN_SIZE));
}

int main(void)
{
  test_array_elements_do_not_have_names();
/*
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
    js_array(&s, "array");
      js_boolean(&s, "e1", 1);
      js_boolean(&s, "e2", 0);
      js_boolean(&s, "e3", 1);
      js_boolean(&s, "e4", 0);
    js_array_end(&s); 
    js_boolean(&s, "false", 0);
  js_document_end(&s);

  printf("\n");
*/
  return 0;
}

