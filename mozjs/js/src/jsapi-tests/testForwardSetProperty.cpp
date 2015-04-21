/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "jsapi-tests/tests.h"

BEGIN_TEST(testForwardSetProperty)
{
    JS::RootedValue v1(cx);
    EVAL("var foundValue; \n"
         "var obj1 = { set prop(val) { foundValue = this; } }; \n"
         "obj1;",
         &v1);

    JS::RootedValue v2(cx);
    EVAL("var obj2 = Object.create(obj1); \n"
         "obj2;",
         &v2);

    JS::RootedValue v3(cx);
    EVAL("var obj3 = {}; \n"
         "obj3;",
         &v3);

    JS::RootedObject obj1(cx, &v1.toObject());
    JS::RootedObject obj2(cx, &v2.toObject());
    JS::RootedObject obj3(cx, &v3.toObject());

    JS::RootedValue setval(cx, JS::Int32Value(42));

    JS::RootedValue propkey(cx);
    EVAL("'prop';", &propkey);

    JS::RootedId prop(cx);
    CHECK(JS_ValueToId(cx, propkey, &prop));

    EXEC("function assertEq(a, b, msg) \n"
         "{ \n"
         "  if (!Object.is(a, b)) \n"
         "    throw new Error('Assertion failure: ' + msg); \n"
         "}");

    // Non-strict setter

    ObjectOpResult result;
    CHECK(JS_ForwardSetPropertyTo(cx, obj2, prop, setval, v3, result));
    CHECK(result);

    EXEC("assertEq(foundValue, obj3, 'wrong receiver passed to setter');");

    CHECK(JS_ForwardSetPropertyTo(cx, obj2, prop, setval, setval, result));
    CHECK(result);

    EXEC("assertEq(typeof foundValue === 'object', true, \n"
         "         'passing 42 as receiver to non-strict setter ' + \n"
         "         'must box');");

    EXEC("assertEq(foundValue instanceof Number, true, \n"
         "         'passing 42 as receiver to non-strict setter ' + \n"
         "         'must box to a Number');");

    EXEC("assertEq(foundValue.valueOf(), 42, \n"
         "         'passing 42 as receiver to non-strict setter ' + \n"
         "         'must box to new Number(42)');");

    // Strict setter

    EVAL("obj1 = { set prop(val) { 'use strict'; foundValue = this; } }; \n"
         "obj1;",
         &v1);

    CHECK(JS_ForwardSetPropertyTo(cx, obj2, prop, setval, v3, result));
    CHECK(result);

    EXEC("assertEq(foundValue, obj3, 'wrong receiver passed to strict setter');");

    CHECK(JS_ForwardSetPropertyTo(cx, obj2, prop, setval, setval, result));
    CHECK(result);

    JS::RootedValue strictSetSupported(cx);
    EVAL("var strictSetSupported = false; \n"
         "Object.defineProperty(Object.prototype, \n"
         "                      'strictSetter', \n"
         "                      { \n"
         "                        set(v) { \n"
         "                          'use strict'; \n"
         "                          strictSetSupported = \n"
         "                            typeof this === 'number'; \n"
         "                        } \n"
         "                      }); \n"
         "17..strictSetter = 42; \n"
         "strictSetSupported;",
         &strictSetSupported);
    CHECK(strictSetSupported.isBoolean());

    if (strictSetSupported.toBoolean()) {
        // XXX Bug 603201 will fix this.
        MOZ_ASSERT(false,
                   "remove the support-testing check when bug 603201 is fixt");
        EXEC("assertEq(foundValue, 42, \n"
             "         '42 passed as receiver to strict setter ' + \n"
             "         'was mangled');");
    }

    return true;
}
END_TEST(testForwardSetProperty)
