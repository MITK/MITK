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


#include <vtkMitkThickSlicesFilter.h>

#include "mitkImageWriteAccessor.h"
#include "mitkImage.h"

#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

class vtkMitkThickSlicesFilterTestHelper
{
public:

  static mitk::Image::Pointer CreateTestImage( int min, int max )
  {
    mitk::PixelType pixelType( mitk::MakeScalarPixelType<unsigned char>() );
    mitk::Image::Pointer testImage = mitk::Image::New();
    unsigned int* dim = new unsigned int[3];
    dim[0] = 10;
    dim[1] = 10;
    dim[2] = max+1-min;
    testImage->Initialize( pixelType, 3, dim );
    size_t buffer_size = dim[0] * dim[1] * sizeof(unsigned char);

    for( int i=min; i<=max; ++i )
    {
      mitk::ImageWriteAccessor writeAccess( testImage, testImage->GetSliceData( i-min ) );
      memset( writeAccess.GetData(), i, buffer_size );
    }

    return testImage;
  }


  static void EvaluateResult( unsigned char expectedValue, vtkImageData* image, const char* projection )
  {
    MITK_TEST_CONDITION_REQUIRED( image->GetDimensions()[0] == 10
                               && image->GetDimensions()[1] == 10
                               && image->GetDimensions()[2] == 1,
                               "Resulting image has correct size" );

    unsigned char* value = static_cast<unsigned char*>( image->GetScalarPointer(0,0,0) );
    MITK_INFO << "Evaluating projection mode: " << projection;
    MITK_INFO << "expected value: " << static_cast<double>( expectedValue );
    MITK_INFO << "actual value: " << static_cast<double>( value[0] );
    MITK_TEST_CONDITION_REQUIRED( value[0] == expectedValue,
                                 "Resulting image has correct pixel-value" );

  }

};


/**
*  Test for vtkMitkThickSlicesFilter.
*
*/
int vtkMitkThickSlicesFilterTest(int, char* [])
{
  // always start with this!
  MITK_TEST_BEGIN("vtkMitkThickSlicesFilterTest")


  vtkMitkThickSlicesFilter* thickSliceFilter = vtkMitkThickSlicesFilter::New();

  //////////////////////////////////////////////////////////////////////////
  // Image looks like:
  // 000000000
  // 111111111
  // 222222222
  mitk::Image::Pointer testImage1 = vtkMitkThickSlicesFilterTestHelper::CreateTestImage( 0, 2 );
  thickSliceFilter->SetInputData( testImage1->GetVtkImageData() );


  // MaxIP
  thickSliceFilter->SetThickSliceMode( 0 );
  thickSliceFilter->Modified();
  thickSliceFilter->Update();
  vtkMitkThickSlicesFilterTestHelper::EvaluateResult( 2, thickSliceFilter->GetOutput(), "MaxIP" );

  // Sum
  thickSliceFilter->SetThickSliceMode( 1 );
  thickSliceFilter->Modified();
  thickSliceFilter->Update();
  vtkMitkThickSlicesFilterTestHelper::EvaluateResult( 1, thickSliceFilter->GetOutput(), "Sum" );

  // Weighted
  thickSliceFilter->SetThickSliceMode( 2 );
  thickSliceFilter->Modified();
  thickSliceFilter->Update();
  vtkMitkThickSlicesFilterTestHelper::EvaluateResult( 1, thickSliceFilter->GetOutput(), "Weighted" );

  // MinIP
  thickSliceFilter->SetThickSliceMode( 3 );
  thickSliceFilter->Modified();
  thickSliceFilter->Update();
  vtkMitkThickSlicesFilterTestHelper::EvaluateResult( 0, thickSliceFilter->GetOutput(), "MinIP" );

  // Mean
  thickSliceFilter->SetThickSliceMode( 4 );
  thickSliceFilter->Modified();
  thickSliceFilter->Update();
  vtkMitkThickSlicesFilterTestHelper::EvaluateResult( 1, thickSliceFilter->GetOutput(), "Mean" );



  //////////////////////////////////////////////////////////////////////////
  // Image looks like:
  // 333333333
  // 444444444
  // 555555555
  // 666666666
  // 777777777
  // 888888888

  mitk::Image::Pointer testImage2 = vtkMitkThickSlicesFilterTestHelper::CreateTestImage( 3, 8 );
  thickSliceFilter->SetInputData( testImage2->GetVtkImageData() );


  // MaxIP
  thickSliceFilter->SetThickSliceMode( 0 );
  thickSliceFilter->Modified();
  thickSliceFilter->Update();
  vtkMitkThickSlicesFilterTestHelper::EvaluateResult( 8, thickSliceFilter->GetOutput(), "MaxIP" );

  // Sum
  thickSliceFilter->SetThickSliceMode( 1 );
  thickSliceFilter->Modified();
  thickSliceFilter->Update();
  vtkMitkThickSlicesFilterTestHelper::EvaluateResult( 5, thickSliceFilter->GetOutput(), "Sum" );

  // Weighted
  thickSliceFilter->SetThickSliceMode( 2 );
  thickSliceFilter->Modified();
  thickSliceFilter->Update();
  vtkMitkThickSlicesFilterTestHelper::EvaluateResult( 4, thickSliceFilter->GetOutput(), "Weighted" );

  // MinIP
  thickSliceFilter->SetThickSliceMode( 3 );
  thickSliceFilter->Modified();
  thickSliceFilter->Update();
  vtkMitkThickSlicesFilterTestHelper::EvaluateResult( 3, thickSliceFilter->GetOutput(), "MinIP" );

  // Mean
  thickSliceFilter->SetThickSliceMode( 4 );
  thickSliceFilter->Modified();
  thickSliceFilter->Update();
  vtkMitkThickSlicesFilterTestHelper::EvaluateResult( 6, thickSliceFilter->GetOutput(), "Mean" );

  thickSliceFilter->Delete();

  MITK_TEST_END()
}

