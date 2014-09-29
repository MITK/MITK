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


#include "mitkTestingMacros.h"
#include "mitkPixelType.h"
#include <itkImage.h>
#include <mitkLogMacros.h>

#include <itkVectorImage.h>

struct MyObscurePixelType
{
  typedef float ValueType;
  static const size_t Length = 2;

  float val1;
  int val2;
};

//## Documentation
//## main testing method
int mitkPixelTypeTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("PixelTypeTest");

  MITK_INFO << "ptype = mitk::MakePixelType<int, int, 5>();";
  MITK_INFO << "itkPtype = mitk::MakePixelType<ItkImageType>();\n with  itk::Image<itk::FixedArray< int, 5>, 3> ItkImageType";

  mitk::PixelType ptype = mitk::MakePixelType<int, int, 5>();
  typedef itk::Image<itk::FixedArray< int, 5>, 3> ItkImageType;
  mitk::PixelType itkPtype = mitk::MakePixelType<ItkImageType>();

  MITK_TEST_CONDITION_REQUIRED( ptype.GetComponentType() == itk::ImageIOBase::INT, "GetComponentType()");
 // MITK_TEST_CONDITION( ptype.GetPixelTypeId() == typeid(ItkImageType), "GetPixelTypeId()");

  MITK_INFO << ptype.GetPixelTypeAsString();
  MITK_INFO << typeid(ItkImageType).name();

  MITK_TEST_CONDITION_REQUIRED( ptype.GetBpe() == 8*sizeof(int)*5, "[ptype] GetBpe()");
  MITK_TEST_CONDITION_REQUIRED( ptype.GetNumberOfComponents() == 5, "[ptype]GetNumberOfComponents()");
  MITK_TEST_CONDITION_REQUIRED( ptype.GetBitsPerComponent() == 8*sizeof(int), "[ptype]GetBitsPerComponent()");

  MITK_TEST_CONDITION_REQUIRED( itkPtype.GetBpe() == 8*sizeof(int)*5, "[itkPType] GetBpe()");
  MITK_TEST_CONDITION_REQUIRED( itkPtype.GetNumberOfComponents() == 5, "[itkPType] GetNumberOfComponents()");
  MITK_TEST_CONDITION_REQUIRED( itkPtype.GetBitsPerComponent() == 8*sizeof(int), "[itkPType] GetBitsPerComponent()");

  // MITK_TEST_CONDITION_REQUIRED( itkPtype == ptype, "[itkPtype = ptype]");

  //MITK_TEST_CONDITION( ptype.GetPixelTypeAsString().compare("unknown") == 0, "GetPixelTypeAsString()");
  {

    {
      mitk::PixelType ptype2( ptype);
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetComponentType() == itk::ImageIOBase::INT, "ptype2( ptype)- GetComponentType()");
      MITK_TEST_CONDITION( ptype2.GetPixelType() == ptype.GetPixelType(), "ptype2( ptype)-GetPixelType(");
      MITK_TEST_CONDITION( ptype2.GetComponentType() == ptype.GetComponentType(), "ptype2( ptype)-GetComponentType(");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetBpe() == 8*sizeof(int)*5, "ptype2( ptype)-GetBpe()");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetNumberOfComponents() == 5, "ptype2( ptype)-GetNumberOfComponents()");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetBitsPerComponent() == 8*sizeof(int), "ptype2( ptype)-GetBitsPerComponent()");
 //     MITK_TEST_CONDITION_REQUIRED( ptype.GetPixelTypeAsString().compare("unknown") == 0, "ptype2( ptype)-GetPixelTypeAsString()");
    }

    {
      mitk::PixelType ptype2 = ptype;
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetComponentType() == itk::ImageIOBase::INT, "ptype2 = ptype- GetComponentType()");
      MITK_TEST_CONDITION( ptype2.GetPixelType() == ptype.GetPixelType(), "ptype2 = ptype- GetPixelType(");
      MITK_TEST_CONDITION( ptype2.GetComponentType() == ptype.GetComponentType(), "ptype2( ptype)-GetComponentType(");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetBpe() == 8*sizeof(int)*5, "ptype2 = ptype- GetBpe()");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetNumberOfComponents() == 5, "ptype2 = ptype- GetNumberOfComponents()");
      MITK_TEST_CONDITION_REQUIRED( ptype2.GetBitsPerComponent() == 8*sizeof(int), "ptype2 = ptype- GetBitsPerComponent()");
 //     MITK_TEST_CONDITION_REQUIRED( ptype.GetPixelTypeAsString().compare("unknown") == 0, "ptype2 = ptype- GetPixelTypeAsString()");
    }

    {
      mitk::PixelType ptype2 = ptype;
      MITK_TEST_CONDITION_REQUIRED( ptype == ptype2, "operator ==");
      //MITK_TEST_CONDITION_REQUIRED( ptype == typeid(int), "operator ==");
      //mitk::PixelType ptype3 = mitk::MakePixelType<char, char ,5>;
      //MITK_TEST_CONDITION_REQUIRED( ptype != ptype3, "operator !=");
      //MITK_TEST_CONDITION_REQUIRED( *ptype3 != typeid(int), "operator !=");
    }
  }

  // test instantiation
  typedef itk::Image< MyObscurePixelType > MyObscureImageType;
  mitk::PixelType obscurePixelType = mitk::MakePixelType< MyObscureImageType >();

  MITK_TEST_CONDITION( obscurePixelType.GetPixelType() == itk::ImageIOBase::UNKNOWNPIXELTYPE, "PixelTypeId is 'UNKNOWN' ");
  MITK_TEST_CONDITION( obscurePixelType.GetNumberOfComponents() == MyObscurePixelType::Length, "Lenght was set correctly");
  MITK_TEST_CONDITION( obscurePixelType.GetComponentType() == mitk::MapPixelComponentType<MyObscurePixelType::ValueType>::value, "ValueType corresponds."   );

  typedef itk::VectorImage< short, 3> VectorImageType;
  mitk::PixelType vectorPixelType = mitk::MakePixelType< VectorImageType >( 78 );
  //vectorPixelType.SetVectorLength( 78 );

  typedef itk::Image< itk::Vector< short, 7> > FixedVectorImageType;
  mitk::PixelType fixedVectorPixelType = mitk::MakePixelType< FixedVectorImageType >();

  mitk::PixelType scalarPixelType = mitk::MakeScalarPixelType< float >();

  // test ImageTypeTrait traits class
  MITK_TEST_CONDITION(typeid(mitk::ImageTypeTrait<int, 3>::ImageType) == typeid(itk::Image<int, 3>), "ImageTypeTrait");
  MITK_TEST_CONDITION((mitk::ImageTypeTrait<int, 3>::IsVectorImage == false), "ImageTypeTrait");

  MITK_TEST_CONDITION(typeid(mitk::ImageTypeTrait<itk::FixedArray<short, 2>, 3>::ImageType) == typeid(itk::Image<itk::FixedArray<short, 2>, 3>), "ImageTypeTrait");
  MITK_TEST_CONDITION((mitk::ImageTypeTrait<itk::FixedArray<short, 2>, 3>::IsVectorImage == false), "ImageTypeTrait");

  MITK_TEST_CONDITION(typeid(mitk::ImageTypeTrait<itk::VariableLengthVector<short>, 3>::ImageType) == typeid(itk::VectorImage<short, 3>), "ImageTypeTrait");
  MITK_TEST_CONDITION((mitk::ImageTypeTrait<itk::VariableLengthVector<short>, 3>::IsVectorImage == true), "ImageTypeTrait");

  MITK_TEST_CONDITION(typeid(mitk::ImageTypeTrait<itk::Image<int, 3> >::ImageType) == typeid(itk::Image<int, 3>), "ImageTypeTrait");
  MITK_TEST_CONDITION((mitk::ImageTypeTrait<itk::Image<int, 3> >::IsVectorImage == false), "ImageTypeTrait");

  MITK_TEST_CONDITION(typeid(mitk::ImageTypeTrait<itk::Image<itk::FixedArray<short, 2>, 3> >::ImageType) == typeid(itk::Image<itk::FixedArray<short, 2>, 3>), "ImageTypeTrait");
  MITK_TEST_CONDITION((mitk::ImageTypeTrait<itk::Image<itk::FixedArray<short, 2>, 3> >::IsVectorImage == false), "ImageTypeTrait");

  MITK_TEST_CONDITION(typeid(mitk::ImageTypeTrait<itk::VectorImage<short, 3> >::ImageType) == typeid(itk::VectorImage<short, 3>), "ImageTypeTrait");
  MITK_TEST_CONDITION((mitk::ImageTypeTrait<itk::VectorImage<short, 3> >::IsVectorImage == true), "ImageTypeTrait");

  // test CastableTo

  MITK_TEST_END();
}


