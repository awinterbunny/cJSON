/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "../cJSON.c"

static cJSON item[1];

static const unsigned char *error_pointer = NULL;

static void assert_is_object(cJSON *object_item)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(object_item, "Item is NULL.");

    TEST_ASSERT_NULL_MESSAGE(object_item->next, "Linked list next pointer is not NULL.");
    TEST_ASSERT_NULL_MESSAGE(object_item->prev, "Linked list previous pointer is not NULL");
    TEST_ASSERT_BITS_MESSAGE(0xFF, cJSON_Object, object_item->type, "Item type is not object.");
    TEST_ASSERT_BITS_MESSAGE(cJSON_IsReference, 0, object_item->type, "Item should not have a string as reference.");
    TEST_ASSERT_BITS_MESSAGE(cJSON_StringIsConst, 0, object_item->type, "Item should not have a const string.");
    TEST_ASSERT_NULL_MESSAGE(object_item->valuestring, "Valuestring is not NULL.");
    TEST_ASSERT_NULL_MESSAGE(object_item->string, "String is not NULL.");
}

static void assert_is_child(cJSON *child_item, const char *name, int type)
{
    TEST_ASSERT_NOT_NULL_MESSAGE(child_item, "Child item is NULL.");
    TEST_ASSERT_NOT_NULL_MESSAGE(child_item->string, "Child item doesn't have a name.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE(name, child_item->string, "Child item has the wrong name.");
    TEST_ASSERT_BITS(0xFF, type, child_item->type);
}

static void assert_not_object(const char *json)
{
    TEST_ASSERT_NULL(parse_object(item, (const unsigned char*)json, &error_pointer));
}

static void assert_parse_object(const char *json)
{
    TEST_ASSERT_NOT_NULL(parse_object(item, (const unsigned char*)json, &error_pointer));
    assert_is_object(item);
}

static void reset(void)
{
    if (item->child != NULL)
    {
        cJSON_Delete(item->child);
    }
    memset(item, 0, sizeof(cJSON));
}

static void parse_object_should_parse_empty_objects(void)
{
    assert_parse_object("{}");
    TEST_ASSERT_NULL(item->child);
    assert_parse_object("{\n\t}");
    TEST_ASSERT_NULL(item->child);
}

static void parse_array_should_parse_arrays_with_one_element(void)
{

    assert_parse_object("{\"one\":1}");
    assert_is_child(item->child, "one", cJSON_Number);
    reset();

    assert_parse_object("{\"hello\":\"world!\"}");
    assert_is_child(item->child, "hello", cJSON_String);
    reset();

    assert_parse_object("{\"array\":[]}");
    assert_is_child(item->child, "array", cJSON_Array);
    reset();

    assert_parse_object("{\"null\":null}");
    assert_is_child(item->child, "null", cJSON_NULL);
    reset();
}

static void parse_object_should_parse_objects_with_multiple_elements(void)
{
    assert_parse_object("{\"one\":1\t,\t\"two\"\n:2, \"three\":3}");
    assert_is_child(item->child, "one", cJSON_Number);
    assert_is_child(item->child->next, "two", cJSON_Number);
    assert_is_child(item->child->next->next, "three", cJSON_Number);
    reset();

    {
        size_t i = 0;
        cJSON *node = NULL;
        int expected_types[7] =
        {
            cJSON_Number,
            cJSON_NULL,
            cJSON_True,
            cJSON_False,
            cJSON_Array,
            cJSON_String,
            cJSON_Object
        };
        const char *expected_names[7] =
        {
            "one",
            "NULL",
            "TRUE",
            "FALSE",
            "array",
            "world",
            "object"
        };
        assert_parse_object("{\"one\":1, \"NULL\":null, \"TRUE\":true, \"FALSE\":false, \"array\":[], \"world\":\"hello\", \"object\":{}}");

        node = item->child;
        for (
                i = 0;
                (i < (sizeof(expected_types)/sizeof(int)))
                && (node != NULL);
                i++, node = node->next)
        {
            assert_is_child(node, expected_names[i], expected_types[i]);
        }
        TEST_ASSERT_EQUAL_INT(i, 7);
        reset();
    }
}

static void parse_object_should_not_parse_non_objects(void)
{
    assert_not_object("");
    assert_not_object("{");
    assert_not_object("}");
    assert_not_object("[\"hello\",{}]");
    assert_not_object("42");
    assert_not_object("3.14");
    assert_not_object("\"{}hello world!\n\"");
}

int main(void)
{
    /* initialize cJSON item */
    memset(item, 0, sizeof(cJSON));

    UNITY_BEGIN();
    RUN_TEST(parse_object_should_parse_empty_objects);
    RUN_TEST(parse_object_should_not_parse_non_objects);
    RUN_TEST(parse_object_should_parse_objects_with_multiple_elements);
    RUN_TEST(parse_array_should_parse_arrays_with_one_element);
    return UNITY_END();
}
