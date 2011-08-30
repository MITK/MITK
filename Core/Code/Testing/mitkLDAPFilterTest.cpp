/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkTestingMacros.h>

#include <mitkLDAPFilter.h>

int TestParsing()
{
  // WELL FORMED Expr
  try
  {
    MITK_TEST_OUTPUT(<< "Parsing (cn=Babs Jensen)")
    mitk::LDAPFilter ldap( "(cn=Babs Jensen)" );
    MITK_TEST_OUTPUT(<< "Parsing (!(cn=Tim Howes))")
    ldap = mitk::LDAPFilter( "(!(cn=Tim Howes))" );
    MITK_TEST_OUTPUT(<< "Parsing " << std::string("(&(") + mitk::ServiceConstants::OBJECTCLASS() + "=Person)(|(sn=Jensen)(cn=Babs J*)))")
    ldap = mitk::LDAPFilter( std::string("(&(") + mitk::ServiceConstants::OBJECTCLASS() + "=Person)(|(sn=Jensen)(cn=Babs J*)))" );
    MITK_TEST_OUTPUT(<< "Parsing (o=univ*of*mich*)")
    ldap = mitk::LDAPFilter( "(o=univ*of*mich*)" );
  }
  catch (const std::invalid_argument& e)
  {
    MITK_TEST_OUTPUT(<< e.what());
    return EXIT_FAILURE;
  }


  // MALFORMED Expr
  try
  {
    MITK_TEST_OUTPUT( << "Parsing malformed expr: cn=Babs Jensen)")
    mitk::LDAPFilter ldap( "cn=Babs Jensen)" );
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
    mitk::LDAPFilter ldap( "(Cn=Babs Jensen)" );
    mitk::ServiceProperties props;
    bool eval = false;

    // Several values
    props["cn"] = std::string("Babs Jensen");
    props["unused"] = std::string("Jansen");
    MITK_TEST_OUTPUT(<< "Evaluating expr: " << ldap.ToString())
    eval = ldap.Match(props);
    if (!eval)
    {
      return EXIT_FAILURE;
    }

    // WILDCARD
    ldap = mitk::LDAPFilter( "(cn=Babs *)" );
    props.clear();
    props["cn"] = std::string("Babs Jensen");
    MITK_TEST_OUTPUT(<< "Evaluating wildcard expr: " << ldap.ToString())
    eval = ldap.Match(props);
    if ( !eval )
    {
      return EXIT_FAILURE;
    }

    // NOT FOUND
    ldap = mitk::LDAPFilter( "(cn=Babs *)" );
    props.clear();
    props["unused"] = std::string("New");
    MITK_TEST_OUTPUT(<< "Expr not found test: " << ldap.ToString())
    eval = ldap.Match(props);
    if ( eval )
    {
      return EXIT_FAILURE;
    }

    // std::vector with integer values
    ldap = mitk::LDAPFilter( "  ( |(cn=Babs *)(sn=1) )" );
    props.clear();
    std::vector<mitk::Any> list;
    list.push_back(std::string("Babs Jensen"));
    list.push_back(std::string("1"));
    props["sn"] = list;
    MITK_TEST_OUTPUT(<< "Evaluating vector expr: " << ldap.ToString())
    eval = ldap.Match(props);
    if (!eval)
    {
      return EXIT_FAILURE;
    }

    // wrong case
    ldap = mitk::LDAPFilter( "(cN=Babs *)" );
    props.clear();
    props["cn"] = std::string("Babs Jensen");
    MITK_TEST_OUTPUT(<< "Evaluating case sensitive expr: " << ldap.ToString())
    eval = ldap.MatchCase(props);
    if (eval)
    {
      return EXIT_FAILURE;
    }
  }
  catch (const std::invalid_argument& e)
  {
    MITK_TEST_OUTPUT( << e.what() )
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int mitkLDAPFilterTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("LDAPFilterTest");

  MITK_TEST_CONDITION(TestParsing() == EXIT_SUCCESS, "Parsing LDAP expressions: ")
  MITK_TEST_CONDITION(TestEvaluate() == EXIT_SUCCESS, "Evaluating LDAP expressions: ")

  MITK_TEST_END()
}

