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

typedef itk::VectorImage< short, 3>   ItkDwiType;

/**Documentation
 * Test the Fiberfox simulation functions (fiberBundle -> diffusion weighted image)
 */
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

void StartSimulation(FiberfoxParameters<double> parameters, FiberBundle::Pointer fiberBundle, mitk::Image::Pointer refImage, string message)
{
    itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
    tractsToDwiFilter->SetUseConstantRandSeed(true);
    tractsToDwiFilter->SetParameters(parameters);
    tractsToDwiFilter->SetFiberBundle(fiberBundle);
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
                MITK_INFO << "Saving test and rference image to " << mitk::IOUtil::GetTempPath();
                mitk::IOUtil::SaveBaseData(testImage, mitk::IOUtil::GetTempPath()+"testImage.dwi");
                mitk::IOUtil::SaveBaseData(refImage, mitk::IOUtil::GetTempPath()+"refImage.dwi");
            }
            MITK_TEST_CONDITION_REQUIRED(cond, message);
        }
    }
}

int mitkFiberfoxSignalGenerationTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkFiberfoxSignalGenerationTest");

    // input fiber bundle
    FiberBundle::Pointer fiberBundle = dynamic_cast<FiberBundle*>(mitk::IOUtil::Load(argv[1])[0].GetPointer());

    for (int i=2; i<argc; i++)
    {
        // Load parameter file
        FiberfoxParameters<double> parameters;
        string file = argv[i];
        MITK_INFO << "Starting test: " << file;
        parameters.LoadParameters(file+".ffp");

        // Load reference diffusion weighted image
        mitk::Image::Pointer mitkRef = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(file+".dwi")->GetData());

        StartSimulation(parameters, fiberBundle, mitkRef, file);
    }

    // always end with this!
    MITK_TEST_END();
}
