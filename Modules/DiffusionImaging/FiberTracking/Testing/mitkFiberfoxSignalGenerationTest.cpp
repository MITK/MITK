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


#include "mitkTestFixture.h"

class mitkFiberfoxSignalGenerationTestSuite : public mitk::TestFixture
{

    CPPUNIT_TEST_SUITE(mitkFiberfoxSignalGenerationTestSuite);
    //MITK_TEST(Test0);
    //MITK_TEST(Test1);
    MITK_TEST(Test2);
    MITK_TEST(Test3);
    MITK_TEST(Test4);
    MITK_TEST(Test5);
    //MITK_TEST(Test6);
    MITK_TEST(Test7);
    MITK_TEST(Test8);
    CPPUNIT_TEST_SUITE_END();

    typedef itk::VectorImage< short, 3>   ItkDwiType;

private:

public:

    /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
    FiberBundle::Pointer m_FiberBundle;
    std::vector< FiberfoxParameters<double> > m_Parameters;
    std::vector< mitk::Image::Pointer > m_RefImages;

    void setUp() override
    {
        m_FiberBundle = dynamic_cast<FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/Fiberfox/Signalgen.fib"))[0].GetPointer());

        {
        FiberfoxParameters<double> parameters;
        parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param1.ffp"));
        m_Parameters.push_back(parameters);
        m_RefImages.push_back(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param1.dwi")));
        }

        {
        FiberfoxParameters<double> parameters;
        parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param2.ffp"));
        m_Parameters.push_back(parameters);
        m_RefImages.push_back(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param2.dwi")));
        }

        {
        FiberfoxParameters<double> parameters;
        parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param3.ffp"));
        m_Parameters.push_back(parameters);
        m_RefImages.push_back(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param3.dwi")));
        }

        {
        FiberfoxParameters<double> parameters;
        parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param4.ffp"));
        m_Parameters.push_back(parameters);
        m_RefImages.push_back(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param4.dwi")));
        }

        {
        FiberfoxParameters<double> parameters;
        parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param5.ffp"));
        m_Parameters.push_back(parameters);
        m_RefImages.push_back(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param5.dwi")));
        }

        {
        FiberfoxParameters<double> parameters;
        parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param6.ffp"));
        m_Parameters.push_back(parameters);
        m_RefImages.push_back(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param6.dwi")));
        }

        {
        FiberfoxParameters<double> parameters;
        parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param7.ffp"));
        m_Parameters.push_back(parameters);
        m_RefImages.push_back(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param7.dwi")));
        }

        {
        FiberfoxParameters<double> parameters;
        parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param8.ffp"));
        m_Parameters.push_back(parameters);
        m_RefImages.push_back(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param8.dwi")));
        }

        {
        FiberfoxParameters<double> parameters;
        parameters.LoadParameters(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param9.ffp"));
        m_Parameters.push_back(parameters);
        m_RefImages.push_back(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/Fiberfox/params/param9.dwi")));
        }
    }

    void tearDown() override
    {

    }

    bool CompareDwi(itk::VectorImage< short, 3 >* dwi1, itk::VectorImage< short, 3 >* dwi2)
    {
        typedef itk::VectorImage< short, 3 > DwiImageType;
        try{
            itk::ImageRegionIterator< DwiImageType > it1(dwi1, dwi1->GetLargestPossibleRegion());
            itk::ImageRegionIterator< DwiImageType > it2(dwi2, dwi2->GetLargestPossibleRegion());
            while(!it1.IsAtEnd())
            {
                if (it1.Get()!=it2.Get())
                {
                    MITK_INFO << it1.GetIndex() << ":" << it1.Get();
                    MITK_INFO << it2.GetIndex() << ":" << it2.Get();
                    return false;
                }
                ++it1;
                ++it2;
            }
        }
        catch(...)
        {
            return false;
        }
        return true;
    }

    void StartSimulation(FiberfoxParameters<double> parameters, mitk::Image::Pointer refImage, string out)
    {
        itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
        tractsToDwiFilter->SetUseConstantRandSeed(true);
        tractsToDwiFilter->SetParameters(parameters);
        tractsToDwiFilter->SetFiberBundle(m_FiberBundle);
        tractsToDwiFilter->Update();

        mitk::Image::Pointer testImage = mitk::GrabItkImageMemory( tractsToDwiFilter->GetOutput() );
        testImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( parameters.m_SignalGen.GetGradientDirections() ) );
        testImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( parameters.m_SignalGen.m_Bvalue ) );

        mitk::DiffusionPropertyHelper propertyHelper( testImage );
        propertyHelper.InitializeImage();

        if (refImage.IsNotNull())
        {
            if( static_cast<mitk::GradientDirectionsProperty*>( refImage->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer().IsNotNull() )
            {
                ItkDwiType::Pointer itkTestImagePointer = ItkDwiType::New();
                mitk::CastToItkImage(testImage, itkTestImagePointer);
                ItkDwiType::Pointer itkRefImagePointer = ItkDwiType::New();
                mitk::CastToItkImage(refImage, itkRefImagePointer);

                bool cond = CompareDwi(itkTestImagePointer, itkRefImagePointer);
                if (!cond)
                {
                    MITK_INFO << "Saving test image to " << mitk::IOUtil::GetTempPath();
                    mitk::IOUtil::SaveBaseData(testImage, mitk::IOUtil::GetTempPath()+out);
                }
                CPPUNIT_ASSERT_MESSAGE("Simulated images should be equal", cond);
            }
        }
    }

    void Test0()
    {
        StartSimulation(m_Parameters.at(0), m_RefImages.at(0), "param1.dwi");
    }

    void Test1()
    {
        StartSimulation(m_Parameters.at(1), m_RefImages.at(1), "param2.dwi");
    }

    void Test2()
    {
        StartSimulation(m_Parameters.at(2), m_RefImages.at(2), "param3.dwi");
    }

    void Test3()
    {
        StartSimulation(m_Parameters.at(3), m_RefImages.at(3), "param4.dwi");
    }

    void Test4()
    {
        StartSimulation(m_Parameters.at(4), m_RefImages.at(4), "param5.dwi");
    }

    void Test5()
    {
        StartSimulation(m_Parameters.at(5), m_RefImages.at(5), "param6.dwi");
    }

    void Test6()
    {
        StartSimulation(m_Parameters.at(6), m_RefImages.at(6), "param7.dwi");
    }

    void Test7()
    {
        StartSimulation(m_Parameters.at(7), m_RefImages.at(7), "param8.dwi");
    }

    void Test8()
    {
        StartSimulation(m_Parameters.at(8), m_RefImages.at(8), "param9.dwi");
    }

};

MITK_TEST_SUITE_REGISTRATION(mitkFiberfoxSignalGeneration)
