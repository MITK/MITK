/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usModuleResourceContainer_p.h"

#include "usTestingMacros.h"

using namespace us;

namespace {

bool Matches(const std::string& name, const std::string& pattern)
{
  return ModuleResourceContainer::Matches(name, pattern);
}

}

int usModuleResourceContainerTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleResourceContainerTest");

  // Short-cut: bare '*' matches anything (including empty).
  US_TEST_CONDITION(Matches("",         "*"), "'*' matches empty")
  US_TEST_CONDITION(Matches("anything", "*"), "'*' matches non-empty")

  // Literal pattern (no wildcards): name must equal the pattern exactly.
  US_TEST_CONDITION( Matches("foo",    "foo"),    "literal exact match")
  US_TEST_CONDITION(!Matches("foobar", "foo"),    "literal rejects extra suffix")
  US_TEST_CONDITION(!Matches("barfoo", "foo"),    "literal rejects extra prefix")
  US_TEST_CONDITION(!Matches("fo",     "foo"),    "literal rejects shorter name")

  // Trailing wildcard: name must start with the literal prefix.
  US_TEST_CONDITION( Matches("foobar", "foo*"),   "trailing '*' matches prefix")
  US_TEST_CONDITION( Matches("foo",    "foo*"),   "trailing '*' allows empty tail")
  US_TEST_CONDITION(!Matches("xfoo",   "foo*"),   "trailing '*' rejects non-prefix")

  // Leading wildcard: name must end with the literal suffix.
  US_TEST_CONDITION( Matches("file.txt",     "*.txt"), "leading '*' matches suffix")
  US_TEST_CONDITION(!Matches("file.txt.bak", "*.txt"), "leading '*' rejects non-suffix (regression)")
  US_TEST_CONDITION( Matches("file.txt.txt", "*.txt"), "leading '*' matches when suffix repeats (regression)")
  US_TEST_CONDITION( Matches(".txt",         "*.txt"), "leading '*' allows empty prefix")
  US_TEST_CONDITION( Matches("fooXfoo",      "*foo"),  "leading '*' matches multi-occurrence tail (regression)")

  // Middle wildcard: start and end anchored, wildcard in between.
  US_TEST_CONDITION( Matches("foobar",      "foo*bar"), "infix '*' between anchored parts")
  US_TEST_CONDITION( Matches("foobazbar",   "foo*bar"), "infix '*' spans arbitrary middle")
  US_TEST_CONDITION( Matches("foo-bar-bar", "foo*bar"), "infix '*' matches when suffix repeats (regression)")
  US_TEST_CONDITION(!Matches("foobar",      "foo*baz"), "infix '*' rejects wrong suffix")
  US_TEST_CONDITION( Matches("aXbXb",       "a*b"),     "infix '*' matches multi-occurrence tail (regression)")
  US_TEST_CONDITION(!Matches("ab",          "a*b*c"),   "infix '*' rejects too-short name")

  // Both ends wildcarded: literal must appear anywhere.
  US_TEST_CONDITION( Matches("xyabcxy", "*abc*"), "'*abc*' matches substring")
  US_TEST_CONDITION( Matches("abc",     "*abc*"), "'*abc*' matches at both ends")
  US_TEST_CONDITION(!Matches("abd",     "*abc*"), "'*abc*' rejects non-substring")

  // Consecutive wildcards collapse to a single wildcard.
  US_TEST_CONDITION( Matches("file.txt",     "**.txt"), "'**' acts as '*'")
  US_TEST_CONDITION( Matches("file.txt.txt", "**.txt"), "'**' anchors suffix correctly")
  US_TEST_CONDITION(!Matches("file.txt.bak", "**.txt"), "'**' still rejects non-suffix")

  US_TEST_END()
}
