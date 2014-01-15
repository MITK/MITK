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
#include <mitkFiberTrackingObjectFactory.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkFiberBundleX.h>
#include <itkTractsToDWIImageFilter.h>
#include <mitkFiberfoxParameters.h>
#include <mitkFiberBundleXReader.h>
#include <mitkStickModel.h>
#include <mitkTensorModel.h>
#include <mitkBallModel.h>
#include <mitkDotModel.h>
#include <mitkAstroStickModel.h>
#include <mitkDiffusionImage.h>
#include <mitkNrrdDiffusionImageWriter.h>
#include <itkTestingComparisonImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <mitkRicianNoiseModel.h>
#include <mitkChiSquareNoiseModel.h>

/**Documentation
 *
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

void StartSimulation(FiberfoxParameters parameters, FiberBundleX::Pointer fiberBundle, mitk::DiffusionImage<short>::Pointer refImage, string message)
{
    itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
    tractsToDwiFilter->SetUseConstantRandSeed(true);
    tractsToDwiFilter->SetSimulateEddyCurrents(parameters.m_DoSimulateEddyCurrents);
    tractsToDwiFilter->SetEddyGradientStrength(parameters.m_EddyStrength);
    tractsToDwiFilter->SetAddGibbsRinging(parameters.m_AddGibbsRinging);
    tractsToDwiFilter->SetSimulateRelaxation(parameters.m_DoSimulateRelaxation);
    tractsToDwiFilter->SetImageRegion(parameters.m_ImageRegion);
    tractsToDwiFilter->SetSpacing(parameters.m_ImageSpacing);
    tractsToDwiFilter->SetOrigin(parameters.m_ImageOrigin);
    tractsToDwiFilter->SetDirectionMatrix(parameters.m_ImageDirection);
    tractsToDwiFilter->SetFiberBundle(fiberBundle);
    tractsToDwiFilter->SetFiberModels(parameters.m_FiberModelList);
    tractsToDwiFilter->SetNonFiberModels(parameters.m_NonFiberModelList);
    tractsToDwiFilter->SetNoiseModel(parameters.m_NoiseModel);
    tractsToDwiFilter->SetkOffset(parameters.m_KspaceLineOffset);
    tractsToDwiFilter->SettLine(parameters.m_tLine);
    tractsToDwiFilter->SettInhom(parameters.m_tInhom);
    tractsToDwiFilter->SetTE(parameters.m_tEcho);
    tractsToDwiFilter->SetNumberOfRepetitions(parameters.m_Repetitions);
    tractsToDwiFilter->SetEnforcePureFiberVoxels(parameters.m_DoDisablePartialVolume);
    tractsToDwiFilter->SetInterpolationShrink(parameters.m_InterpolationShrink);
    tractsToDwiFilter->SetFiberRadius(parameters.m_AxonRadius);
    tractsToDwiFilter->SetSignalScale(parameters.m_SignalScale);
    if (parameters.m_InterpolationShrink>0)
        tractsToDwiFilter->SetUseInterpolation(true);
    tractsToDwiFilter->SetTissueMask(parameters.m_MaskImage);
    tractsToDwiFilter->SetFrequencyMap(parameters.m_FrequencyMap);
    tractsToDwiFilter->SetSpikeAmplitude(parameters.m_SpikeAmplitude);
    tractsToDwiFilter->SetSpikes(parameters.m_Spikes);
    tractsToDwiFilter->SetWrap(parameters.m_Wrap);
    tractsToDwiFilter->SetAddMotionArtifact(parameters.m_DoAddMotion);
    tractsToDwiFilter->SetMaxTranslation(parameters.m_Translation);
    tractsToDwiFilter->SetMaxRotation(parameters.m_Rotation);
    tractsToDwiFilter->SetRandomMotion(parameters.m_RandomMotion);
    tractsToDwiFilter->Update();

    mitk::DiffusionImage<short>::Pointer testImage = mitk::DiffusionImage<short>::New();
    testImage->SetVectorImage( tractsToDwiFilter->GetOutput() );
    testImage->SetB_Value(parameters.m_Bvalue);
    testImage->SetDirections(parameters.m_GradientDirections);
    testImage->InitializeFromVectorImage();

    if (refImage.IsNotNull())
    {
        MITK_TEST_CONDITION_REQUIRED(CompareDwi(testImage->GetVectorImage(), refImage->GetVectorImage()), message);
    }
    else
    {
        MITK_INFO << "Saving test image to " << message;
        NrrdDiffusionImageWriter<short>::Pointer writer = NrrdDiffusionImageWriter<short>::New();
        writer->SetFileName(message);
        writer->SetInput(testImage);
        writer->Update();
    }
}

int mitkFiberfoxSignalGenerationTest(int argc, char* argv[])
{
    RegisterDiffusionCoreObjectFactory();

    MITK_TEST_BEGIN("mitkFiberfoxSignalGenerationTest");

    MITK_TEST_CONDITION_REQUIRED(argc>=19,"check for input data");

    // input fiber bundle
    FiberBundleXReader::Pointer fibReader = FiberBundleXReader::New();
    fibReader->SetFileName(argv[1]);
    fibReader->Update();
    FiberBundleX::Pointer fiberBundle = dynamic_cast<FiberBundleX*>(fibReader->GetOutput());

    // reference diffusion weighted images
    mitk::DiffusionImage<short>::Pointer stickBall = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[2])->GetData());
    mitk::DiffusionImage<short>::Pointer stickAstrosticks = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[3])->GetData());
    mitk::DiffusionImage<short>::Pointer stickDot = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[4])->GetData());
    mitk::DiffusionImage<short>::Pointer tensorBall = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[5])->GetData());
    mitk::DiffusionImage<short>::Pointer stickTensorBall = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[6])->GetData());
    mitk::DiffusionImage<short>::Pointer stickTensorBallAstrosticks = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[7])->GetData());
    mitk::DiffusionImage<short>::Pointer gibbsringing = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[8])->GetData());
    mitk::DiffusionImage<short>::Pointer ghost = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[9])->GetData());
    mitk::DiffusionImage<short>::Pointer aliasing = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[10])->GetData());
    mitk::DiffusionImage<short>::Pointer eddy = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[11])->GetData());
    mitk::DiffusionImage<short>::Pointer linearmotion = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[12])->GetData());
    mitk::DiffusionImage<short>::Pointer randommotion = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[13])->GetData());
    mitk::DiffusionImage<short>::Pointer spikes = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[14])->GetData());
    mitk::DiffusionImage<short>::Pointer riciannoise = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[15])->GetData());
    mitk::DiffusionImage<short>::Pointer chisquarenoise = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[16])->GetData());
    mitk::DiffusionImage<short>::Pointer distortions = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(argv[17])->GetData());
    mitk::Image::Pointer mitkFMap = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[18])->GetData());
    typedef itk::Image<double, 3> ItkDoubleImgType;
    ItkDoubleImgType::Pointer fMap = ItkDoubleImgType::New();
    mitk::CastToItkImage<ItkDoubleImgType>(mitkFMap, fMap);

    FiberfoxParameters parameters;
    parameters.m_DoSimulateRelaxation = true;
    parameters.m_SignalScale = 10000;
    parameters.m_ImageRegion = stickBall->GetVectorImage()->GetLargestPossibleRegion();
    parameters.m_ImageSpacing = stickBall->GetVectorImage()->GetSpacing();
    parameters.m_ImageOrigin = stickBall->GetVectorImage()->GetOrigin();
    parameters.m_ImageDirection = stickBall->GetVectorImage()->GetDirection();
    parameters.m_Bvalue = stickBall->GetB_Value();
    mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer dirs = stickBall->GetDirections();
    parameters.m_NumGradients = 0;
    for (unsigned int i=0; i<dirs->Size(); i++)
    {
        DiffusionSignalModel<double>::GradientType g;
        g[0] = dirs->at(i)[0];
        g[1] = dirs->at(i)[1];
        g[2] = dirs->at(i)[2];
        parameters.m_GradientDirections.push_back(g);
        if (dirs->at(i).magnitude()>0.0001)
            parameters.m_NumGradients++;
    }

    // intra and inter axonal compartments
    mitk::StickModel<double> stickModel;
    stickModel.SetBvalue(parameters.m_Bvalue);
    stickModel.SetT2(110);
    stickModel.SetDiffusivity(0.001);
    stickModel.SetGradientList(parameters.m_GradientDirections);

    mitk::TensorModel<double> tensorModel;
    tensorModel.SetT2(110);
    stickModel.SetBvalue(parameters.m_Bvalue);
    tensorModel.SetDiffusivity1(0.001);
    tensorModel.SetDiffusivity2(0.00025);
    tensorModel.SetDiffusivity3(0.00025);
    tensorModel.SetGradientList(parameters.m_GradientDirections);

    // extra axonal compartment models
    mitk::BallModel<double> ballModel;
    ballModel.SetT2(80);
    ballModel.SetBvalue(parameters.m_Bvalue);
    ballModel.SetDiffusivity(0.001);
    ballModel.SetGradientList(parameters.m_GradientDirections);

    mitk::AstroStickModel<double> astrosticksModel;
    astrosticksModel.SetT2(80);
    astrosticksModel.SetBvalue(parameters.m_Bvalue);
    astrosticksModel.SetDiffusivity(0.001);
    astrosticksModel.SetRandomizeSticks(true);
    astrosticksModel.SetSeed(0);
    astrosticksModel.SetGradientList(parameters.m_GradientDirections);

    mitk::DotModel<double> dotModel;
    dotModel.SetT2(80);
    dotModel.SetGradientList(parameters.m_GradientDirections);

    // noise models
    mitk::RicianNoiseModel<double>* ricianNoiseModel = new mitk::RicianNoiseModel<double>();
    ricianNoiseModel->SetNoiseVariance(1000000);
    ricianNoiseModel->SetSeed(0);

    // Rician noise
    mitk::ChiSquareNoiseModel<double>* chiSquareNoiseModel = new mitk::ChiSquareNoiseModel<double>();
    chiSquareNoiseModel->SetDOF(500000);
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
        parameters.m_AddGibbsRinging = true;
        StartSimulation(parameters, fiberBundle, gibbsringing, argv[8]);

        // Ghost
        parameters.m_AddGibbsRinging = false;
        parameters.m_KspaceLineOffset = 0.25;
        StartSimulation(parameters, fiberBundle, ghost, argv[9]);

        // Aliasing
        parameters.m_KspaceLineOffset = 0;
        parameters.m_Wrap = 0.4;
        parameters.m_SignalScale = 1000;
        StartSimulation(parameters, fiberBundle, aliasing, argv[10]);

        // Eddy currents
        parameters.m_Wrap = 1;
        parameters.m_SignalScale = 10000;
        parameters.m_DoSimulateEddyCurrents = true;
        parameters.m_EddyStrength = 0.05;
        StartSimulation(parameters, fiberBundle, eddy, argv[11]);

        // Motion (linear)
        parameters.m_DoSimulateEddyCurrents = false;
        parameters.m_EddyStrength = 0.0;
        parameters.m_DoAddMotion = true;
        parameters.m_RandomMotion = false;
        parameters.m_Translation[1] = 10;
        parameters.m_Rotation[2] = 90;
        StartSimulation(parameters, fiberBundle, linearmotion, argv[12]);

        // Motion (random)
        parameters.m_RandomMotion = true;
        parameters.m_Translation[1] = 5;
        parameters.m_Rotation[2] = 45;
        StartSimulation(parameters, fiberBundle, randommotion, argv[13]);

        // Spikes
        parameters.m_DoAddMotion = false;
        parameters.m_Spikes = 5;
        parameters.m_SpikeAmplitude = 1;
        StartSimulation(parameters, fiberBundle, NULL, "/tmp/spikes.dwi");

        // Rician noise
        parameters.m_Spikes = 0;
        parameters.m_NoiseModel = ricianNoiseModel;
        StartSimulation(parameters, fiberBundle, riciannoise, argv[15]);
        delete parameters.m_NoiseModel;

        // Chi-square noise
        parameters.m_NoiseModel = chiSquareNoiseModel;
        StartSimulation(parameters, fiberBundle, chisquarenoise, argv[16]);
        delete parameters.m_NoiseModel;

        // Distortions
        parameters.m_NoiseModel = NULL;
        parameters.m_FrequencyMap = fMap;
        StartSimulation(parameters, fiberBundle, distortions, argv[17]);
    }
    catch (std::exception &e)
    {
        MITK_TEST_CONDITION_REQUIRED(false, e.what());
    }

    // always end with this!
    MITK_TEST_END();
}
