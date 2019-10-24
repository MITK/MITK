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

// Testing
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

// std includes
#include <string>

// MITK includes
#include "mitkCorrelationCalculator.h"
#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkImageRegionAccessor.h>

// VTK includes
#include <vtkDebugLeaks.h>

class mitkCorrelationCalculatorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkCorrelationCalculatorTestSuite);

  /// \todo Fix VTK memory leaks. Bug 18097.
  vtkDebugLeaks::SetExitError(0);

  MITK_TEST(CalculateWholeCorrelation);
  MITK_TEST(CalculateParcelCorrelation);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::Image::Pointer m_ParcellationImage;
  mitk::Image::Pointer m_TimeSeriesImage;

public:

  /**
  * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
  */
  void setUp()
  {
    // The setup is somewhat cumbersome, due to the fact, that the mitk-itk conversion
    // functions do not play nicely with 4D images

    //create test images
    itk::Image<int, 4>::Pointer timeSeriesTestImage = itk::Image<int, 4>::New();
    itk::Image<int, 4>::RegionType timeSeriesTestRegion;
    itk::Image<int, 4>::IndexType timeSeriesTestIndex;
    itk::Image<int, 4>::SizeType timeSeriesTestSize;
    timeSeriesTestIndex[0] = 0;
    timeSeriesTestIndex[1] = 0;
    timeSeriesTestIndex[2] = 0;
    timeSeriesTestIndex[3] = 0;

    timeSeriesTestSize[0] = 3;
    timeSeriesTestSize[1] = 3;
    timeSeriesTestSize[2] = 3;
    timeSeriesTestSize[3] = 10;

    timeSeriesTestRegion.SetIndex(timeSeriesTestIndex);
    timeSeriesTestRegion.SetSize(timeSeriesTestSize);

    timeSeriesTestImage->SetRegions(timeSeriesTestRegion);
    timeSeriesTestImage->Allocate();
    timeSeriesTestImage->FillBuffer(0);
    timeSeriesTestImage->Update();

    itk::Image<int, 3>::Pointer parcellationTestImage = itk::Image<int, 3>::New();
    itk::Image<int, 3>::RegionType parcellationTestRegion;
    itk::Image<int, 3>::IndexType parcellationTestIndex;
    itk::Image<int, 3>::SizeType parcellationTestSize;
    parcellationTestIndex[0] = 0;
    parcellationTestIndex[1] = 0;
    parcellationTestIndex[2] = 0;

    parcellationTestSize[0] = 3;
    parcellationTestSize[1] = 3;
    parcellationTestSize[2] = 3;

    parcellationTestRegion.SetIndex(parcellationTestIndex);
    parcellationTestRegion.SetSize(parcellationTestSize);

    parcellationTestImage->SetRegions(parcellationTestRegion);
    parcellationTestImage->Allocate();

    //Fill test images with values

    mitk::Image::Pointer tsTestImage = mitk::Image::New();
    mitk::GrabItkImageMemory( timeSeriesTestImage, tsTestImage.GetPointer() );
    mitk::Image::Pointer pTestImage = mitk::Image::New();
    mitk::GrabItkImageMemory( parcellationTestImage, pTestImage.GetPointer());

    //divide parcellation image into 3 different parcels
    for( int loop(0); loop < 27; ++loop)
    {
      itk::Image<int, 3>::IndexType parcellationIndex;
      parcellationIndex[2] = (loop / 9);
      parcellationIndex[1] = (loop / 3) % 3;
      parcellationIndex[0] = loop % 3;
      if(loop < 9)
      {
        parcellationTestImage->SetPixel(parcellationIndex, 1);
      }
      else if( loop < 15 || (loop > 17 && loop < 21) )
      {
        parcellationTestImage->SetPixel(parcellationIndex, 2);
      }
      else
      {
        parcellationTestImage->SetPixel(parcellationIndex, 3);
      }
    }

    mitk::ImageRegionAccessor writeAccess (tsTestImage);

    //fill time series image with similar time series in each parcel
    for( int loop(0); loop < 270; ++loop)
    {
      itk::Image<int, 4>::IndexType timeSeriesIndex;
      timeSeriesIndex[3] = (loop / 27);
      timeSeriesIndex[2] = (loop / 9) % 3;
      timeSeriesIndex[1] = (loop / 3) % 3;
      timeSeriesIndex[0] = loop % 3;

      itk::Image<int, 3>::IndexType parcellationIndex;
      parcellationIndex[2] = (loop / 9) % 3;
      parcellationIndex[1] = (loop / 3) % 3;
      parcellationIndex[0] = loop % 3;

      if( mitk::Equal(parcellationTestImage->GetPixel(parcellationIndex), 1) )
      {
        *(int*)writeAccess.getPixel(timeSeriesIndex) = 1 + loop;
      }
      else if( mitk::Equal(parcellationTestImage->GetPixel(parcellationIndex), 2) )
      {
        *(int*)writeAccess.getPixel(timeSeriesIndex) = 1 + (loop % 13 - loop % 11);
      }
      else
      {
        *(int*)writeAccess.getPixel(timeSeriesIndex) = 1 + (loop - loop % 2);
      }
    }

    //end create test images

    m_ParcellationImage = pTestImage;
    m_TimeSeriesImage = tsTestImage;
  }

  void tearDown()
  {
    m_ParcellationImage = nullptr;
    m_TimeSeriesImage = nullptr;
  }

  void CalculateWholeCorrelation()
  {
    mitk::CorrelationCalculator<int>::Pointer correlationCalculator = mitk::CorrelationCalculator<int>::New();
    correlationCalculator->SetTimeSeriesImage( m_TimeSeriesImage );
    correlationCalculator->DoWholeCorrelation();

    const vnl_matrix< double >* cM = correlationCalculator->GetCorrelationMatrix();

    bool equal(true);
    // instead of checking the entire 27 x 27 matrix
    // just check 3 values and assume if they are equal entire
    // matrix is likely to be equal
    if( std::abs((*cM)[0][0] - 1) > 0.00001 )
    {
      equal = false;
    }
    if( std::abs((*cM)[2][10] - 0.00828941) > 0.00001 )
    {
      equal = false;
    }
    if( std::abs((*cM)[14][8] - 0.636613) > 0.00001 )
    {
      equal = false;
    }
    CPPUNIT_ASSERT_MESSAGE( "Comparing created and reference correlation matrix sample points.", equal );
  }

  void CalculateParcelCorrelation()
  {
    mitk::CorrelationCalculator<int>::Pointer correlationCalculator = mitk::CorrelationCalculator<int>::New();
    correlationCalculator->SetTimeSeriesImage( m_TimeSeriesImage );
    correlationCalculator->SetParcellationImage( m_ParcellationImage );
    correlationCalculator->DoParcelCorrelation();

    const vnl_matrix< double >* cM = correlationCalculator->GetCorrelationMatrix();

    bool equal(true);
    // diagonal should be 1
    if( std::abs((*cM)[0][0] - 1) > 0.00001 )
    {
      equal = false;
    }
    if( std::abs((*cM)[1][1] - 1) > 0.00001 )
    {
      equal = false;
    }
    if( std::abs((*cM)[2][2] - 1) > 0.00001 )
    {
      equal = false;
    }
    // parcel 0 and parcel 1 should correlate with -0.431111
    if( std::abs((*cM)[1][0] - -0.431111) > 0.00001 )
    {
      equal = false;
    }
    if( std::abs((*cM)[0][1] - -0.431111) > 0.00001 )
    {
      equal = false;
    }
    // parcel 0 and parcel 2 should correlate with 1
    if( std::abs((*cM)[0][2] - 1) > 0.00001 )
    {
      equal = false;
    }
    if( std::abs((*cM)[2][0] - 1) > 0.00001 )
    {
      equal = false;
    }
    // parcel 1 and parcel 2 should correlate with -0.430522
    if( std::abs((*cM)[1][2] - -0.430522) > 0.00001 )
    {
      equal = false;
    }
    if( std::abs((*cM)[2][1] - -0.430522) > 0.00001 )
    {
      equal = false;
    }
    CPPUNIT_ASSERT_MESSAGE( "Comparing created and reference correlation matrix.", equal );

    mitk::ConnectomicsNetwork::Pointer network = correlationCalculator->GetConnectomicsNetwork();

    mitk::ConnectomicsNetwork::Pointer refNetwork = mitk::ConnectomicsNetwork::New();
    mitk::ConnectomicsNetwork::VertexDescriptorType x = refNetwork->AddVertex( 1 );
    mitk::ConnectomicsNetwork::VertexDescriptorType y = refNetwork->AddVertex( 2 );
    mitk::ConnectomicsNetwork::VertexDescriptorType z = refNetwork->AddVertex( 3 );
    refNetwork->AddEdge(x,y);
    refNetwork->AddEdge(x,z);
    refNetwork->AddEdge(y,z);
    CPPUNIT_ASSERT_MESSAGE( "Comparing created and reference network.", mitk::Equal( network.GetPointer(), refNetwork, mitk::eps, true) );

    // check sample parcels for other methods
    correlationCalculator->DoParcelCorrelation( mitk::CorrelationCalculator<int>::UseAverageCorrelation );
    cM = correlationCalculator->GetCorrelationMatrix();
    // parcel 0 and parcel 1 should correlate with -0.0643023
    equal = true;
    if( std::abs((*cM)[1][0] - -0.0643023) > 0.00001 )
    {
      equal = false;
    }
    // parcel 0 and parcel 2 should correlate with 0.99998
    if( std::abs((*cM)[2][0] - 0.99998) > 0.00001 )
    {
      equal = false;
    }
    CPPUNIT_ASSERT_MESSAGE( "Comparing sample parcel correlation for average correlation.", equal );

    correlationCalculator->DoParcelCorrelation( mitk::CorrelationCalculator<int>::UseMaximumCorrelation );
    cM = correlationCalculator->GetCorrelationMatrix();
    // parcel 0 and parcel 1 should correlate with 0.636613
    equal = true;
    if( std::abs((*cM)[1][0] - 0.636613) > 0.00001 )
    {
      equal = false;
    }
    // parcel 0 and parcel 2 should correlate with 0.99998
    if( std::abs((*cM)[2][0] - 0.99998) > 0.00001 )
    {
      equal = false;
    }
    CPPUNIT_ASSERT_MESSAGE( "Comparing sample parcel correlation for maximum correlation.", equal );
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkCorrelationCalculator)
