/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <cmath>
#include <limits>
#include <locale>

#include <mitkTestingMacros.h>
#include <mitkFormulaParser.h>

using namespace mitk;

namespace
{
  // mimics locales like de_DE where '.' groups thousands and ',' is the
  // decimal separator
  struct GroupingNumpunct : std::numpunct<char>
  {
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
  };
}

#define TEST_NOTHROW(expression, MSG) \
    do \
    { \
        MITK_TEST_OUTPUT_NO_ENDL(<< MSG) \
        bool test_caught = false; \
        try \
        { \
            expression; \
        } \
        catch(...) \
        { \
            test_caught = true; \
            MITK_TEST_FAILED_MSG(<< "An unwanted exception was thrown"); \
        } \
        if(!test_caught) \
        { \
            MITK_TEST_OUTPUT(<< " [PASSED]") \
            mitk::TestManager::GetInstance()->TestPassed(); \
        } \
    } while(0)

/*!
 *	@author Sascha Diatschuk
 */
class FormulaParserTests
{
public:
  static void TestConstructor()
  {
    std::map<std::string, double> varMap;
    FormulaParser *nullParser = nullptr, *parser = nullptr;

    TEST_NOTHROW(nullParser = new FormulaParser(nullptr),
      "Testing constructor with NULL argument");
    TEST_NOTHROW(parser = new FormulaParser(&varMap),
      "Testing constructor with valid argument");

    delete nullParser;
    delete parser;
  }

  static void TestLookupVariable()
  {
    // variable map is NULL
    FormulaParser *nullParser = new FormulaParser(nullptr);
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, nullParser->LookupVariable("test"));
    delete nullParser;

    // variable map is empty
    std::map<std::string, double> varMap;
    FormulaParser *parser = new FormulaParser(&varMap);
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser->LookupVariable("test"));

    // lookup should succeed
    double var;
    varMap["test"] = 17;
    TEST_NOTHROW(var = parser->LookupVariable("test"),
      "Testing if lookupVariable throws unwanted exceptions");
    MITK_TEST_CONDITION_REQUIRED(var == 17,
      "Testing if lookupVariable returns the correct value");

    delete parser;
  }

  static void TestParse()
  {
    std::map<std::string, double> varMap;
    varMap["test"] = 17;
    FormulaParser *parser = new FormulaParser(&varMap);

    // empty string
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser->Parse(""));

    // grammar can't process string
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser->Parse("_"));

    // unexpected character
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser->Parse("5="));

    // unknown variable
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser->Parse("a"));

    double d;

    // addition
    TEST_NOTHROW(d = parser->Parse("1+2"),
      "Testing if addition throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 3,
      "Testing if addition produces the correct result");

    // subtraction
    TEST_NOTHROW(d = parser->Parse("5-1"),
      "Testing if subtraction throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 4,
      "Testing if subtraction produces the correct result");

    // multiplication
    TEST_NOTHROW(d = parser->Parse("3*4"),
      "Testing if multiplication throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 12,
      "Testing if multiplication produces the correct result");

    // division
    TEST_NOTHROW(d = parser->Parse("28/4"),
      "Testing if division throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 7,
      "Testing if division produces the correct result");

    // exponentiation
    TEST_NOTHROW(d = parser->Parse("2^3"),
      "Testing if exponentiation throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 8,
      "Testing if exponentiation produces the correct result");

    // algebraic signs
    TEST_NOTHROW(d = parser->Parse("-7 + +1 - -1"),
      "Testing if algebraic signs throw an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == -5,
      "Testing if algebraic signs produce the correct result");

    // parentheses
    TEST_NOTHROW(d = parser->Parse("(1+2)*(4-2)"),
      "Testing if parentheses throw an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 6,
      "Testing if parentheses produce the correct result");

    // variables
    TEST_NOTHROW(d = parser->Parse("2*test-test"),
      "Testing if variables throw an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 17,
      "Testing if variables produce the correct result");

    // abs
    TEST_NOTHROW(d = parser->Parse("abs(-5)"),
      "Testing if abs throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 5,
      "Testing if abs produces the correct result");

    const double eps = 0.0001;

    // exp
    TEST_NOTHROW(d = parser->Parse("exp(1)"),
      "Testing if exp throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 2.71828182846) < eps,
      "Testing if exp produces the correct result");

    // sin
    TEST_NOTHROW(d = parser->Parse("sin(1.57079632679)"),
      "Testing if sin throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 1) < eps,
      "Testing if sin produces the correct result");

    // cos
    TEST_NOTHROW(d = parser->Parse("cos(3.14159265359)"),
      "Testing if cos throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d + 1) < eps,
      "Testing if cos produces the correct result");

    // tan
    TEST_NOTHROW(d = parser->Parse("tan(0.46364760899)"),
      "Testing if tan throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 0.5) < eps,
      "Testing if tan produces the correct result");

    // sind
    TEST_NOTHROW(d = parser->Parse("sind(145)"),
      "Testing if sind throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 0.57357643635) < eps,
      "Testing if sind produces the correct result");

    // cosd
    TEST_NOTHROW(d = parser->Parse("cosd(90)"),
      "Testing if cosd throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d < eps,
      "Testing if cosd produces the correct result");

    // tand
    TEST_NOTHROW(d = parser->Parse("tand(15)"),
      "Testing if tand throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 0.26794919243) < eps,
      "Testing if tand produces the correct result");

    // fresnelS
    TEST_NOTHROW(d = parser->Parse("fresnelS(1)"),
      "Testing if fresnelS throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 0.310268) < eps,
      "Testing if fresnelS produces the correct result");

    TEST_NOTHROW(d = parser->Parse("fresnelC(1)"),
      "Testing if fresnelC throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 0.904524) < eps,
      "Testing if fresnelC produces the correct result");

    delete parser;
  }

  static void TestParsedValue(FormulaParser &parser, const std::string &formula,
    double expectedValue)
  {
    const double eps = 0.0001;
    double d = 0;

    TEST_NOTHROW(d = parser.Parse(formula),
      "Testing if '" + formula + "' does not throw an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - expectedValue) < eps,
      "Testing if '" + formula + "' evaluates to " + std::to_string(expectedValue));
  }

  static void TestParseExtended()
  {
    std::map<std::string, double> varMap;
    FormulaParser parser(&varMap);

    // precedence and associativity, matching the original grammar
    TestParsedValue(parser, "2^3^2", 64); // '^' folds left-associatively
    TestParsedValue(parser, "2+3*4^2", 50);
    TestParsedValue(parser, "-2^2", 4); // unary minus binds tighter than '^'
    TestParsedValue(parser, "2^-3", 0.125);
    TestParsedValue(parser, "2^-3^2", 0.015625);
    TestParsedValue(parser, "-(2^2)", -4);
    TestParsedValue(parser, "--5", 5);
    TestParsedValue(parser, "-+5", -5);

    // numeric literal forms
    TestParsedValue(parser, ".5", 0.5);
    TestParsedValue(parser, "5.", 5);
    TestParsedValue(parser, "1e3", 1000);
    TestParsedValue(parser, "1.e3", 1000);
    TestParsedValue(parser, "1e+3", 1000);
    TestParsedValue(parser, "2E2", 200);
    TestParsedValue(parser, ".5e-2", 0.005);
    TestParsedValue(parser, "1.5e-3", 0.0015);

    // whitespace is skipped between tokens
    TestParsedValue(parser, "sin (0)", 0);
    TestParsedValue(parser, "\t1 +\n 2\r", 3);

    // number literals must be locale-independent even if a host application
    // installs a global locale that uses '.' as thousands separator
    const std::locale originalLocale =
      std::locale::global(std::locale(std::locale::classic(), new GroupingNumpunct));
    TestParsedValue(parser, "1.234", 1.234);
    TestParsedValue(parser, "0.001", 0.001);
    std::locale::global(originalLocale);

    // variables and function-name/variable interaction
    varMap["test2_var_"] = 3;
    TestParsedValue(parser, "test2_var_*2", 6);
    varMap["sin"] = 2;
    TestParsedValue(parser, "sin*sin", 4); // function name without '(' is a variable
    TestParsedValue(parser, "sin(0)", 0);  // ...but with '(' the function wins
    varMap["infusion"] = 3;
    varMap["inf"] = 7;
    TestParsedValue(parser, "infusion", 3); // "inf"/"nan" are not literals
    TestParsedValue(parser, "inf", 7);
    TestParsedValue(parser, "abs(-5.7)", 5.7);

    // cached formulas read the current variable values
    varMap["x"] = 1;
    TestParsedValue(parser, "x*2", 2);
    varMap["x"] = 5;
    TestParsedValue(parser, "x*2", 10);

    // unknown variables fail at evaluation time, so inserting the variable
    // afterwards makes the same formula work
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("q"));
    varMap["q"] = 42;
    TestParsedValue(parser, "q", 42);

    // failed compilations must not poison the cache
    TestParsedValue(parser, "1+1", 2);
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("bad("));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("bad("));
    TestParsedValue(parser, "1+1", 2);
    TestParsedValue(parser, "2*3", 6);
    TestParsedValue(parser, "1+1", 2);

    FormulaParser freshParser(&varMap);
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, freshParser.Parse(""));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, freshParser.Parse(""));

    // syntax errors
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("2x"));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("2+"));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("(2"));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("foo(2)"));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("te st"));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("nan(2)"));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("Infinity"));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("0x10"));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("()"));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("^2"));
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse("1 2"));

    // overflow and division by zero follow IEEE semantics, no exception
    double d = 0;
    TEST_NOTHROW(d = parser.Parse("1/0"),
      "Testing if division by zero does not throw");
    MITK_TEST_CONDITION_REQUIRED(std::isinf(d) && d > 0,
      "Testing if division by zero yields positive infinity");
    TEST_NOTHROW(d = parser.Parse("1e400"),
      "Testing if an overflowing literal does not throw");
    MITK_TEST_CONDITION_REQUIRED(d >= std::numeric_limits<double>::max(),
      "Testing if an overflowing literal yields a huge value");

    // deep but sane nesting works, hostile nesting is rejected
    TestParsedValue(parser, std::string(400, '(') + "1" + std::string(400, ')'), 1);
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser.Parse(std::string(100000, '(')));
  }
};

int mitkFormulaParserTest(int, char *[])
{
  MITK_TEST_BEGIN("FormulaParser Test");

  FormulaParserTests::TestConstructor();
  FormulaParserTests::TestLookupVariable();
  FormulaParserTests::TestParse();
  FormulaParserTests::TestParseExtended();

  MITK_TEST_END();
}
