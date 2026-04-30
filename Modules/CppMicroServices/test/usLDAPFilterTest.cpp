/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usLDAPFilter.h>
#include <usLDAPProp.h>

#include "usTestingMacros.h"

#include <stdexcept>

using namespace us;

int TestParsing()
{
  // WELL FORMED Expr
  try
  {
    US_TEST_OUTPUT(<< "Parsing (cn=Babs Jensen)")
    LDAPFilter ldap( "(cn=Babs Jensen)" );
    US_TEST_OUTPUT(<< "Parsing (!(cn=Tim Howes))")
    ldap = LDAPFilter( "(!(cn=Tim Howes))" );
    US_TEST_OUTPUT(<< "Parsing " << std::string("(&(") + ServiceConstants::OBJECTCLASS() + "=Person)(|(sn=Jensen)(cn=Babs J*)))")
    ldap = LDAPFilter( std::string("(&(") + ServiceConstants::OBJECTCLASS() + "=Person)(|(sn=Jensen)(cn=Babs J*)))" );
    US_TEST_OUTPUT(<< "Parsing (o=univ*of*mich*)")
    ldap = LDAPFilter( "(o=univ*of*mich*)" );
  }
  catch (const std::invalid_argument& e)
  {
    US_TEST_OUTPUT(<< e.what());
    return EXIT_FAILURE;
  }


  // MALFORMED Expr
  try
  {
    US_TEST_OUTPUT( << "Parsing malformed expr: cn=Babs Jensen)")
    LDAPFilter ldap( "cn=Babs Jensen)" );
    return EXIT_FAILURE;
  }
  catch (const std::invalid_argument&)
  {
  }

  return EXIT_SUCCESS;
}


int TestEvaluate()
{
  // EVALUATE
  try
  {
    LDAPFilter ldap( "(Cn=Babs Jensen)" );
    ServiceProperties props;
    bool eval = false;

    // Several values
    props["cn"] = std::string("Babs Jensen");
    props["unused"] = std::string("Jansen");
    US_TEST_OUTPUT(<< "Evaluating expr: " << ldap.ToString())
    eval = ldap.Match(props);
    if (!eval)
    {
      return EXIT_FAILURE;
    }

    // WILDCARD
    ldap = LDAPFilter( "(cn=Babs *)" );
    props.clear();
    props["cn"] = std::string("Babs Jensen");
    US_TEST_OUTPUT(<< "Evaluating wildcard expr: " << ldap.ToString())
    eval = ldap.Match(props);
    if ( !eval )
    {
      return EXIT_FAILURE;
    }

    // NOT FOUND
    ldap = LDAPFilter( "(cn=Babs *)" );
    props.clear();
    props["unused"] = std::string("New");
    US_TEST_OUTPUT(<< "Expr not found test: " << ldap.ToString())
    eval = ldap.Match(props);
    if ( eval )
    {
      return EXIT_FAILURE;
    }

    // std::vector with integer values
    ldap = LDAPFilter( "  ( |(cn=Babs *)(sn=1) )" );
    props.clear();
    std::vector<Any> list;
    list.push_back(std::string("Babs Jensen"));
    list.push_back(std::string("1"));
    props["sn"] = list;
    US_TEST_OUTPUT(<< "Evaluating vector expr: " << ldap.ToString())
    eval = ldap.Match(props);
    if (!eval)
    {
      return EXIT_FAILURE;
    }

    // wrong case
    ldap = LDAPFilter( "(cN=Babs *)" );
    props.clear();
    props["cn"] = std::string("Babs Jensen");
    US_TEST_OUTPUT(<< "Evaluating case sensitive expr: " << ldap.ToString())
    eval = ldap.MatchCase(props);
    if (eval)
    {
      return EXIT_FAILURE;
    }
  }
  catch (const std::invalid_argument& e)
  {
    US_TEST_OUTPUT( << e.what() )
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void TestLDAPExpressions()
{
  LDAPFilter filter(
        LDAPProp("bla") == "jo" && !(LDAPProp("ha") == 1) &&
        (LDAPProp("presence") || !LDAPProp("absence")) &&
        LDAPProp("le") <= 4.1 && LDAPProp("ge") >= -3 &&
        LDAPProp("approx").Approx("Approx")
        );
  const std::string filterStr = "(&(&(&(&(&(bla=jo)(!(ha=1)))(|(presence=*)(!(absence=*))))(le<=4.1))(ge>=-3))(approx~=Approx))";
  US_TEST_CONDITION(filter.ToString() == filterStr, "test generated filter string")

  std::string emptyValue;
  std::string someValue = "some";
  std::string filter1 = LDAPProp("key2") == someValue && LDAPProp("key3");
  std::string filter2 = LDAPProp("key2") == someValue && (LDAPProp("key1") == emptyValue || LDAPProp("key3"));
  US_TEST_CONDITION(filter1 == filter2, "test null expressions")
}

void TestBoolComparison()
{
  // Regression: std::equal only checked pattern.size() characters, so a
  // shorter pattern could falsely match a longer bool literal. Both
  // "(key=tru)" and "(key=fals)" must reject bool true/false.
  ServiceProperties propsTrue;
  propsTrue["key"] = true;
  ServiceProperties propsFalse;
  propsFalse["key"] = false;

  US_TEST_CONDITION( LDAPFilter("(key=true)").Match(propsTrue),
                     "full bool literal matches true")
  US_TEST_CONDITION(!LDAPFilter("(key=tru)").Match(propsTrue),
                     "truncated bool pattern must not match true")
  US_TEST_CONDITION(!LDAPFilter("(key=trueX)").Match(propsTrue),
                     "longer bool pattern must not match true")
  US_TEST_CONDITION( LDAPFilter("(key=false)").Match(propsFalse),
                     "full bool literal matches false")
  US_TEST_CONDITION(!LDAPFilter("(key=fals)").Match(propsFalse),
                     "truncated bool pattern must not match false")
}

int usLDAPFilterTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("LDAPFilterTest");

  TestLDAPExpressions();
  US_TEST_CONDITION(TestParsing() == EXIT_SUCCESS, "Parsing LDAP expressions: ")
  US_TEST_CONDITION(TestEvaluate() == EXIT_SUCCESS, "Evaluating LDAP expressions: ")
  TestBoolComparison();

  US_TEST_END()
}
