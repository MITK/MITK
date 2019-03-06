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

#include <mitkTestingMacros.h>
#include <mitkIOUtil.h>
#include <mitkFiberBundle.h>
#include <itkTractsToDWIImageFilter.h>
#include <mitkFiberfoxParameters.h>
#include <mitkStickModel.h>
#include <mitkTensorModel.h>
#include <mitkBallModel.h>
#include <mitkDotModel.h>
#include <mitkAstroStickModel.h>
#include <mitkImage.h>
#include <itkTestingComparisonImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <mitkRicianNoiseModel.h>
#include <mitkChiSquareNoiseModel.h>
#include <mitkIOUtil.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkProperties.h>
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>

#include <itkVectorImage.h>
#include <omp.h>

#include "mitkTestFixture.h"

class mitkFiberfoxSignalGenerationTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkFiberfoxSignalGenerationTestSuite);
  MITK_TEST(Test1);
  MITK_TEST(Test2);
  MITK_TEST(Test3);
  MITK_TEST(Test4);
  MITK_TEST(Test5);
  MITK_TEST(Test6);
  MITK_TEST(Test7);
  MITK_TEST(Test8);
  MITK_TEST(Test9);
  CPPUNIT_TEST_SUITE_END();

  typedef itk::VectorImage< short, 3>   ItkDwiType;

private:

public:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
  FiberBundle::Pointer m_FiberBundle;

  void setUp() override
  {
    m_FiberBundle = mitk::IOUtil::Load<FiberBundle>(GetTestDataFilePath("DiffusionImaging/Fiberfox/Signalgen.fib"));
  }

  void tearDown() override
  {

  }

  bool CompareDwi(itk::VectorImage< short, 3 >* dwi1, itk::VectorImage< short, 3 >* dwi2)
  {
    bool out = true;
    typedef itk::VectorImage< short, 3 > DwiImageType;
    try{
      itk::ImageRegionIterator< DwiImageType > it1(dwi1, dwi1->GetLargestPossibleRegion());
      itk::ImageRegionIterator< DwiImageType > it2(dwi2, dwi2->GetLargestPossibleRegion());
      int count = 0;
      while(!it1.IsAtEnd())
      {
        for (unsigned int i=0; i<dwi1->GetVectorLength(); ++i)
        {
          short d = abs(it1.Get()[i]-it2.Get()[i]);

          if (d>1)
          {
            if (count<10)
            {
              MITK_INFO << "**************************************";
              MITK_INFO << "Test value: " << it1.GetIndex() << ":" << it1.Get()[i];
              MITK_INFO << "Ref. value: " << it2.GetIndex() << ":" << it2.Get()[i];
            }
            out = false;
            count++;
          }
        }
        ++it1;
        ++it2;
      }
      if (count>=10)
        MITK_INFO << "Skipping errors.";
      MITK_INFO << "Errors detected: " << count;
    }
    catch(...)
    {
      return false;
    }
    return out;
  }

  void StartSimulation(FiberfoxParameters parameters, mitk::Image::Pointer refImage, std::string out)
  {
    itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
    tractsToDwiFilter->SetUseConstantRandSeed(true);
    tractsToDwiFilter->SetParameters(parameters);
    tractsToDwiFilter->SetFiberBundle(m_FiberBundle);
    tractsToDwiFilter->Update();

    mitk::Image::Pointer testImage = mitk::GrabItkImageMemory( tractsToDwiFilter->GetOutput() );
    mitk::DiffusionPropertyHelper::SetGradientContainer(testImage, parameters.m_SignalGen.GetItkGradientContainer());
    mitk::DiffusionPropertyHelper::SetReferenceBValue(testImage, parameters.m_SignalGen.GetBvalue());

    mitk::DiffusionPropertyHelper::InitializeImage( testImage );

    if (refImage.IsNotNull())
    {
      if(mitk::DiffusionPropertyHelper::GetGradientContainer(refImage).IsNotNull() )
      {
        ItkDwiType::Pointer itkTestImagePointer = ItkDwiType::New();
        mitk::CastToItkImage(testImage, itkTestImagePointer);
        ItkDwiType::Pointer itkRefImagePointer = ItkDwiType::New();
        mitk::CastToItkImage(refImage, itkRefImagePointer);

        bool cond = CompareDwi(itkTestImagePointer, itkRefImagePointer);
        if (!cond)
        {
          MITK_INFO << "Saving test image to " << mitk::IOUtil::GetTempPath()+out;
          mitk::IOUtil::Save(testImage, mitk::IOUtil::GetTempPath()+out);
          mitk::IOUtil::Save(refImage, mitk::IOUtil::GetTempPath()+ "REF_" + out);
        }
        CPPUNIT_ASSERT_MESSAGE("Simulated images should be equal", cond);
      }
    }
  }

  void Test1()
  {
    FiberfoxParameters parameters;
    parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param1.ffp"), true);
    mitk::Image::Pointer refImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param1.dwi"));
    StartSimulation(parameters, refImage, "param1.dwi");
  }

  void Test2()
  {
    FiberfoxParameters parameters;
    parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param2.ffp"), true);
    mitk::Image::Pointer refImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param2.dwi"));
    StartSimulation(parameters, refImage, "param2.dwi");
  }

  void Test3()
  {
    FiberfoxParameters parameters;
    parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param3.ffp"), true);
    mitk::Image::Pointer refImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param3.dwi"));
    StartSimulation(parameters, refImage, "param3.dwi");
  }

  void Test4()
  {
    FiberfoxParameters parameters;
    parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param4.ffp"), true);
    mitk::Image::Pointer refImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param4.dwi"));
    StartSimulation(parameters, refImage, "param4.dwi");
  }

  void Test5()
  {
    FiberfoxParameters parameters;
    parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param5.ffp"), true);
    mitk::Image::Pointer refImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param5.dwi"));
    StartSimulation(parameters, refImage, "param5.dwi");
  }

  void Test6()
  {
    FiberfoxParameters parameters;
    parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param6.ffp"), true);
    mitk::Image::Pointer refImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param6.dwi"));
    StartSimulation(parameters, refImage, "param6.dwi");
  }

  void Test7()
  {
    FiberfoxParameters parameters;
    parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param7.ffp"), true);
    mitk::Image::Pointer refImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param7.dwi"));
    StartSimulation(parameters, refImage, "param7.dwi");
  }

  void Test8()
  {
    FiberfoxParameters parameters;
    parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param8.ffp"), true);
    mitk::Image::Pointer refImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param8.dwi"));
    StartSimulation(parameters, refImage, "param8.dwi");
  }

  void Test9()
  {
    FiberfoxParameters parameters;
    parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param9.ffp"), true);
    mitk::Image::Pointer refImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param9.dwi"));
    StartSimulation(parameters, refImage, "param9.dwi");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkFiberfoxSignalGeneration)
