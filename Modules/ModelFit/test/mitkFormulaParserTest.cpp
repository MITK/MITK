/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkFormulaParser.h"

using namespace mitk;

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
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, nullParser->lookupVariable("test"));
    delete nullParser;

    // variable map is empty
    std::map<std::string, double> varMap;
    FormulaParser *parser = new FormulaParser(&varMap);
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser->lookupVariable("test"));

    // lookup should succeed
    double var;
    varMap["test"] = 17;
    TEST_NOTHROW(var = parser->lookupVariable("test"),
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
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser->parse(""));

    // grammar can't process string
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser->parse("_"));

    // unexpected character
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser->parse("5="));

    // unknown variable
    MITK_TEST_FOR_EXCEPTION(FormulaParserException, parser->parse("a"));

    double d;

    // addition
    TEST_NOTHROW(d = parser->parse("1+2"),
      "Testing if addition throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 3,
      "Testing if addition produces the correct result");

    // subtraction
    TEST_NOTHROW(d = parser->parse("5-1"),
      "Testing if subtraction throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 4,
      "Testing if subtraction produces the correct result");

    // multiplication
    TEST_NOTHROW(d = parser->parse("3*4"),
      "Testing if multiplication throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 12,
      "Testing if multiplication produces the correct result");

    // division
    TEST_NOTHROW(d = parser->parse("28/4"),
      "Testing if division throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 7,
      "Testing if division produces the correct result");

    /*!	@TODO:	Reactivate as soon as exponentiation works again */
    // exponentiation
    //TEST_NOTHROW(d = parser->parse("2^3"),
    //	"Testing if exponentiation throws an unwanted exception");
    //MITK_TEST_CONDITION_REQUIRED(d == 8,
    //	"Testing if exponentiation produces the correct result");

    // algebraic signs
    TEST_NOTHROW(d = parser->parse("-7 + +1 - -1"),
      "Testing if algebraic signs throw an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == -5,
      "Testing if algebraic signs produce the correct result");

    // parentheses
    TEST_NOTHROW(d = parser->parse("(1+2)*(4-2)"),
      "Testing if parentheses throw an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 6,
      "Testing if parentheses produce the correct result");

    // variables
    TEST_NOTHROW(d = parser->parse("2*test-test"),
      "Testing if variables throw an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 17,
      "Testing if variables produce the correct result");

    // abs
    TEST_NOTHROW(d = parser->parse("abs(-5)"),
      "Testing if abs throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d == 5,
      "Testing if abs produces the correct result");

    const double eps = 0.0001;

    // exp
    TEST_NOTHROW(d = parser->parse("exp(1)"),
      "Testing if exp throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 2.71828182846) < eps,
      "Testing if exp produces the correct result");

    // sin
    TEST_NOTHROW(d = parser->parse("sin(1.57079632679)"),
      "Testing if sin throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 1) < eps,
      "Testing if sin produces the correct result");

    // cos
    TEST_NOTHROW(d = parser->parse("cos(3.14159265359)"),
      "Testing if cos throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d + 1) < eps,
      "Testing if cos produces the correct result");

    // tan
    TEST_NOTHROW(d = parser->parse("tan(0.46364760899)"),
      "Testing if tan throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 0.5) < eps,
      "Testing if tan produces the correct result");

    // sind
    TEST_NOTHROW(d = parser->parse("sind(145)"),
      "Testing if sind throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 0.57357643635) < eps,
      "Testing if sind produces the correct result");

    // cosd
    TEST_NOTHROW(d = parser->parse("cosd(90)"),
      "Testing if cosd throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(d < eps,
      "Testing if cosd produces the correct result");

    // tand
    TEST_NOTHROW(d = parser->parse("tand(15)"),
      "Testing if tand throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 0.26794919243) < eps,
      "Testing if tand produces the correct result");

    // fresnelS
    TEST_NOTHROW(d = parser->parse("fresnelS(1)"),
      "Testing if fresnelS throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 0.310268) < eps,
      "Testing if fresnelS produces the correct result");

    TEST_NOTHROW(d = parser->parse("fresnelC(1)"),
      "Testing if fresnelC throws an unwanted exception");
    MITK_TEST_CONDITION_REQUIRED(std::abs(d - 0.904524) < eps,
      "Testing if fresnelC produces the correct result");

    delete parser;
  }
};

int mitkFormulaParserTest(int, char *[])
{
  MITK_TEST_BEGIN("FormulaParser Test");

  FormulaParserTests::TestConstructor();
  FormulaParserTests::TestLookupVariable();
  FormulaParserTests::TestParse();

  MITK_TEST_END();
}
