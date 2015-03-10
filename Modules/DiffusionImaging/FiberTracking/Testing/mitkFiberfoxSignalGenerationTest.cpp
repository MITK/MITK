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
                return false;
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
          mitk::IOUtil::SaveBaseData(testImage, mitk::IOUtil::GetTempPath()+"testImage.nrrd");
          mitk::IOUtil::SaveBaseData(refImage, mitk::IOUtil::GetTempPath()+"refImage.nrrd");
        }
        MITK_TEST_CONDITION_REQUIRED(cond, message);
      }
    }
}

int mitkFiberfoxSignalGenerationTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkFiberfoxSignalGenerationTest");

    MITK_TEST_CONDITION_REQUIRED(argc>=19,"check for input data");

    // input fiber bundle
    FiberBundle::Pointer fiberBundle = dynamic_cast<FiberBundle*>(mitk::IOUtil::Load(argv[1])[0].GetPointer());

    // reference diffusion weighted images
    mitk::Image::Pointer stickBall = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[2])->GetData());
    mitk::Image::Pointer stickAstrosticks = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[3])->GetData());
    mitk::Image::Pointer stickDot = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[4])->GetData());
    mitk::Image::Pointer tensorBall = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[5])->GetData());
    mitk::Image::Pointer stickTensorBall = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[6])->GetData());
    mitk::Image::Pointer stickTensorBallAstrosticks = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[7])->GetData());
    mitk::Image::Pointer gibbsringing = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[8])->GetData());
    mitk::Image::Pointer ghost = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[9])->GetData());
    mitk::Image::Pointer aliasing = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[10])->GetData());
    mitk::Image::Pointer eddy = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[11])->GetData());
    mitk::Image::Pointer linearmotion = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[12])->GetData());
    mitk::Image::Pointer randommotion = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[13])->GetData());
    mitk::Image::Pointer spikes = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[14])->GetData());
    mitk::Image::Pointer riciannoise = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[15])->GetData());
    mitk::Image::Pointer chisquarenoise = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[16])->GetData());
    mitk::Image::Pointer distortions = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[17])->GetData());
    mitk::Image::Pointer mitkFMap = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[18])->GetData());
    typedef itk::Image<double, 3> ItkDoubleImgType;
    ItkDoubleImgType::Pointer fMap = ItkDoubleImgType::New();
    mitk::CastToItkImage(mitkFMap, fMap);

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
  mitk::CastToItkImage(stickBall, itkVectorImagePointer);

    FiberfoxParameters<double> parameters;
    parameters.m_SignalGen.m_SimulateKspaceAcquisition = true;
    parameters.m_SignalGen.m_SignalScale = 10000;
    parameters.m_SignalGen.m_ImageRegion = itkVectorImagePointer->GetLargestPossibleRegion();
    parameters.m_SignalGen.m_ImageSpacing = itkVectorImagePointer->GetSpacing();
    parameters.m_SignalGen.m_ImageOrigin = itkVectorImagePointer->GetOrigin();
    parameters.m_SignalGen.m_ImageDirection = itkVectorImagePointer->GetDirection();
    parameters.m_SignalGen.m_Bvalue = static_cast<mitk::FloatProperty*>(stickBall->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue();
    parameters.m_SignalGen.SetGradienDirections( static_cast<mitk::GradientDirectionsProperty*>( stickBall->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() );

    // intra and inter axonal compartments
    mitk::StickModel<double> stickModel;
    stickModel.SetBvalue(parameters.m_SignalGen.m_Bvalue);
    stickModel.SetT2(110);
    stickModel.SetDiffusivity(0.001);
    stickModel.SetGradientList(parameters.m_SignalGen.GetGradientDirections());

    mitk::TensorModel<double> tensorModel;
    tensorModel.SetT2(110);
    stickModel.SetBvalue(parameters.m_SignalGen.m_Bvalue);
    tensorModel.SetDiffusivity1(0.001);
    tensorModel.SetDiffusivity2(0.00025);
    tensorModel.SetDiffusivity3(0.00025);
    tensorModel.SetGradientList(parameters.m_SignalGen.GetGradientDirections());

    // extra axonal compartment models
    mitk::BallModel<double> ballModel;
    ballModel.SetT2(80);
    ballModel.SetBvalue(parameters.m_SignalGen.m_Bvalue);
    ballModel.SetDiffusivity(0.001);
    ballModel.SetGradientList(parameters.m_SignalGen.GetGradientDirections());

    mitk::AstroStickModel<double> astrosticksModel;
    astrosticksModel.SetT2(80);
    astrosticksModel.SetBvalue(parameters.m_SignalGen.m_Bvalue);
    astrosticksModel.SetDiffusivity(0.001);
    astrosticksModel.SetRandomizeSticks(true);
    astrosticksModel.SetSeed(0);
    astrosticksModel.SetGradientList(parameters.m_SignalGen.GetGradientDirections());

    mitk::DotModel<double> dotModel;
    dotModel.SetT2(80);
    dotModel.SetGradientList(parameters.m_SignalGen.GetGradientDirections());

    // noise models
    mitk::RicianNoiseModel<double>* ricianNoiseModel = new mitk::RicianNoiseModel<double>();
    ricianNoiseModel->SetNoiseVariance(1000000);
    ricianNoiseModel->SetSeed(0);

    // Rician noise
    mitk::ChiSquareNoiseModel<double>* chiSquareNoiseModel = new mitk::ChiSquareNoiseModel<double>();
    chiSquareNoiseModel->SetNoiseVariance(1000000);
    chiSquareNoiseModel->SetSeed(0);

    try{
        // Stick-Ball
        parameters.m_FiberModelList.push_back(&stickModel);
        parameters.m_NonFiberModelList.push_back(&ballModel);
        StartSimulation(parameters, fiberBundle, stickBall, argv[2]);

        // Srick-Astrosticks
        parameters.m_NonFiberModelList.clear();
        parameters.m_NonFiberModelList.push_back(&astrosticksModel);
        StartSimulation(parameters, fiberBundle, stickAstrosticks, argv[3]);

        // Stick-Dot
        parameters.m_NonFiberModelList.clear();
        parameters.m_NonFiberModelList.push_back(&dotModel);
        StartSimulation(parameters, fiberBundle, stickDot, argv[4]);

        // Tensor-Ball
        parameters.m_FiberModelList.clear();
        parameters.m_FiberModelList.push_back(&tensorModel);
        parameters.m_NonFiberModelList.clear();
        parameters.m_NonFiberModelList.push_back(&ballModel);
        StartSimulation(parameters, fiberBundle, tensorBall, argv[5]);

        // Stick-Tensor-Ball
        parameters.m_FiberModelList.clear();
        parameters.m_FiberModelList.push_back(&stickModel);
        parameters.m_FiberModelList.push_back(&tensorModel);
        parameters.m_NonFiberModelList.clear();
        parameters.m_NonFiberModelList.push_back(&ballModel);
        StartSimulation(parameters, fiberBundle, stickTensorBall, argv[6]);

        // Stick-Tensor-Ball-Astrosticks
        parameters.m_NonFiberModelList.push_back(&astrosticksModel);
        StartSimulation(parameters, fiberBundle, stickTensorBallAstrosticks, argv[7]);

        // Gibbs ringing
        parameters.m_FiberModelList.clear();
        parameters.m_FiberModelList.push_back(&stickModel);
        parameters.m_NonFiberModelList.clear();
        parameters.m_NonFiberModelList.push_back(&ballModel);
        parameters.m_SignalGen.m_DoAddGibbsRinging = true;
        StartSimulation(parameters, fiberBundle, gibbsringing, argv[8]);

        // Ghost
        parameters.m_SignalGen.m_DoAddGibbsRinging = false;
        parameters.m_SignalGen.m_KspaceLineOffset = 0.25;
        StartSimulation(parameters, fiberBundle, ghost, argv[9]);

        // Aliasing
        parameters.m_SignalGen.m_KspaceLineOffset = 0;
        parameters.m_SignalGen.m_CroppingFactor = 0.4;
        parameters.m_SignalGen.m_SignalScale = 1000;
        StartSimulation(parameters, fiberBundle, aliasing, argv[10]);

        // Eddy currents
        parameters.m_SignalGen.m_CroppingFactor = 1;
        parameters.m_SignalGen.m_SignalScale = 10000;
        parameters.m_SignalGen.m_EddyStrength = 0.05;
        StartSimulation(parameters, fiberBundle, eddy, argv[11]);

        // Motion (linear)
        parameters.m_SignalGen.m_EddyStrength = 0.0;
        parameters.m_SignalGen.m_DoAddMotion = true;
        parameters.m_SignalGen.m_DoRandomizeMotion = false;
        parameters.m_SignalGen.m_Translation[1] = 10;
        parameters.m_SignalGen.m_Rotation[2] = 90;
        StartSimulation(parameters, fiberBundle, linearmotion, argv[12]);

        // Motion (random)
        parameters.m_SignalGen.m_DoRandomizeMotion = true;
        parameters.m_SignalGen.m_Translation[1] = 5;
        parameters.m_SignalGen.m_Rotation[2] = 45;
        StartSimulation(parameters, fiberBundle, randommotion, argv[13]);

        // Spikes
        parameters.m_SignalGen.m_DoAddMotion = false;
        parameters.m_SignalGen.m_Spikes = 5;
        parameters.m_SignalGen.m_SpikeAmplitude = 1;
        StartSimulation(parameters, fiberBundle, spikes, argv[14]);

        // Rician noise
        parameters.m_SignalGen.m_Spikes = 0;
        parameters.m_NoiseModel = ricianNoiseModel;
        StartSimulation(parameters, fiberBundle, riciannoise, argv[15]);
        delete parameters.m_NoiseModel;

        // Chi-square noise
        parameters.m_NoiseModel = chiSquareNoiseModel;
        StartSimulation(parameters, fiberBundle, chisquarenoise, argv[16]);
        delete parameters.m_NoiseModel;

        // Distortions
        parameters.m_NoiseModel = NULL;
        parameters.m_SignalGen.m_FrequencyMap = fMap;
        StartSimulation(parameters, fiberBundle, distortions, argv[17]);
    }
    catch (std::exception &e)
    {
        MITK_TEST_CONDITION_REQUIRED(false, e.what());
    }

    // always end with this!
    MITK_TEST_END();
}
