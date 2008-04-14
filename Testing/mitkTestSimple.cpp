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


// Boost.Test
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
using boost::unit_test_framework::test_suite;

// BOOST
#include <boost/lexical_cast.hpp>

// STL
#include <functional>
#include <iostream>
#include <memory>

#include <iostream>

#include <mitkDataTreeNode.h>
#include <mitkStringProperty.h>
#include <mitkProperties.h>

using boost::unit_test_framework::test_suite;

void TestFunction()
{

  mitk::DataTreeNode::Pointer node;
  mitk::StringProperty::Pointer Prop;
  node = NULL;

  node = mitk::DataTreeNode::New();
  node->SetData(NULL);
  Prop = mitk::StringProperty::New("TestProp");
  node->SetProperty("stringPropTest",Prop);
  Prop = dynamic_cast<mitk::StringProperty *>(node->GetProperty("stringPropTest").GetPointer());
  BOOST_CHECK( Prop.IsNotNull() );
  BOOST_CHECK( Prop->GetValueAsString() == "TestProp");


  int intPropValue = 6;
  node->SetProperty("intProp", mitk::IntProperty::New(intPropValue) );
  mitk::IntProperty::Pointer intProp = dynamic_cast<mitk::IntProperty *>(node->GetProperty("intProp").GetPointer());
  BOOST_CHECK( intProp.IsNotNull() );
  BOOST_CHECK( intProp->GetValue() == intPropValue);
  
  node->SetProperty("boolProp", mitk::BoolProperty::New(true) );
  mitk::BoolProperty::Pointer boolProp = dynamic_cast<mitk::BoolProperty *>(node->GetProperty("boolProp").GetPointer());
  BOOST_CHECK(boolProp.IsNotNull() );
  BOOST_CHECK(boolProp->GetValue() == true);

  node->SetVisibility(false,NULL);
  BOOST_CHECK(node->IsVisible(NULL) == false );

  node->SetOpacity(0.0,NULL);
  float o;
  node->GetOpacity(o,NULL);
  BOOST_CHECK( o == 0.0 );

  node->SetOpacity(100.0,NULL);
  node->GetOpacity(o,NULL);
  BOOST_CHECK( o == 100.0 );

}


test_suite*
init_unit_test_suite( int argc, char * argv[] ) {
   
    test_suite* test= BOOST_TEST_SUITE( "Unit test example 1" );

    test->add( BOOST_TEST_CASE( &TestFunction ), 0 /* expected zero errors */ );


    return test;
}
