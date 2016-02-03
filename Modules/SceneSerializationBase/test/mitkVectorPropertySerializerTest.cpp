/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include "mitkVectorProperty.h"
#include "mitkBasePropertySerializer.h"
#include <boost/lexical_cast.hpp>

#include <limits>
#include <math.h>

#include "mitkEqual.h"

/**
 \brief Test for VectorPropertySerializer.

 Creates simple std::vector instances, puts them
 into a VectorProperty of appropriate type, then
 asks a serializer to serialize them into XML.

 Test expects that there is a deserializer somewhere
 in the system (i.e. registered with the ITK object
 factory. The test further expects that this
 deserializer is able to create a VectorProperty
 from XML and that this VectorProperty equals the
 input of serialization.
*/
class mitkVectorPropertySerializerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkVectorPropertySerializerTestSuite);
    MITK_TEST(TestSerialize<int>);
    MITK_TEST(TestSerialize<double>);
    MITK_TEST(TestSerializeIntTypedef);
    MITK_TEST(TestSerializeDoubleTypedef);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() override
  {
  }


  void tearDown() override
  {
  }

  template< typename DATATYPE >
  typename mitk::VectorProperty<DATATYPE>::Pointer MakeExampleProperty()
  {
    typename mitk::VectorProperty<DATATYPE>::Pointer vectorProperty = mitk::VectorProperty<DATATYPE>::New();
    std::vector<DATATYPE> data;
    data.push_back( -918273674.6172838 );
    data.push_back( 0 );
    data.push_back( +6172838.918273674 );
    data.push_back( sqrt(2) );
    if (std::numeric_limits<DATATYPE>::has_infinity)
    {
         data.push_back( std::numeric_limits<DATATYPE>::infinity() );
         data.push_back( -std::numeric_limits<DATATYPE>::infinity() );
    }
    // do NOT test NaN: cannot be == to itself, so cannot be tested like the others
    // NaN is covered in a different test (FloatToStringTest at the time of writing this)
    //data.push_back( std::numeric_limits<DATATYPE>::quiet_NaN() );
    vectorProperty->SetValue( data );

    return vectorProperty;
  }

  mitk::BaseProperty::Pointer TestSerialize(mitk::BaseProperty* property)
  {
    std::string serializername = std::string(property->GetNameOfClass()) + "Serializer";

    std::list<itk::LightObject::Pointer> allSerializers = itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
    CPPUNIT_ASSERT_EQUAL( size_t(1), allSerializers.size() );

    mitk::BasePropertySerializer* serializer = dynamic_cast<mitk::BasePropertySerializer*>(allSerializers.begin()->GetPointer());
    CPPUNIT_ASSERT( serializer != nullptr );
    if ( !serializer ) return nullptr;

    serializer->SetProperty(property);
    TiXmlElement* serialization(nullptr);
    try
    {
        serialization = serializer->Serialize();
    } catch ( ... )
    {
    }
    CPPUNIT_ASSERT( serialization != nullptr );
    if (!serialization) return nullptr;

    mitk::BaseProperty::Pointer restoredProperty = serializer->Deserialize( serialization );
    CPPUNIT_ASSERT(restoredProperty.IsNotNull());
    return restoredProperty;
  }

  template < typename DATATYPE >
  void TestSerialize()
  {
    auto property = MakeExampleProperty<DATATYPE>();
    mitk::BaseProperty::Pointer restored_property = TestSerialize(property);

    typename mitk::VectorProperty<DATATYPE>::Pointer restored_vector_property =
        dynamic_cast<mitk::VectorProperty<DATATYPE>*>(restored_property.GetPointer());
    CPPUNIT_ASSERT(restored_vector_property.IsNotNull());

    auto orig_vector = property->GetValue();
    auto restored_vector = restored_vector_property->GetValue();

    CPPUNIT_ASSERT_EQUAL( orig_vector.size(), restored_vector.size() );
    for (unsigned int i = 0; i < orig_vector.size(); ++i)
    {
      // compare using Equal, i.e. with tolerance of mitk::eps
      CPPUNIT_ASSERT_MESSAGE(std::string("Verifying element ") + boost::lexical_cast<std::string>(i),
                             mitk::Equal(orig_vector[i], restored_vector[i]) );
    }
  }

  void TestSerializeIntTypedef()
  {
    mitk::IntVectorProperty::Pointer intVectorProperty = MakeExampleProperty<int>().GetPointer();
    TestSerialize(intVectorProperty.GetPointer());
  }


  void TestSerializeDoubleTypedef()
  {
    mitk::DoubleVectorProperty::Pointer doubleVectorProperty = MakeExampleProperty<double>().GetPointer();
    TestSerialize(doubleVectorProperty.GetPointer());
  }

}; // class

MITK_TEST_SUITE_REGISTRATION(mitkVectorPropertySerializer)
