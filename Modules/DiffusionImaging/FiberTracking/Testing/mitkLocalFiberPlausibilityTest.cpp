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

#include <mitkBaseDataIOFactory.h>
#include <mitkBaseData.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkFiberTrackingObjectFactory.h>
#include <itkEvaluateDirectionImagesFilter.h>
#include <itkTractsToVectorImageFilter.h>
#include <usAny.h>
#include <itkImageFileWriter.h>
#include <mitkIOUtil.h>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <itksys/SystemTools.hxx>
#include <mitkTestingMacros.h>
#include <mitkCompareImageDataFilter.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

int mitkLocalFiberPlausibilityTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkLocalFiberPlausibilityTest");
    MITK_TEST_CONDITION_REQUIRED(argc==11,"check for input data")

    string fibFile = argv[1];
    vector< string > referenceImages; referenceImages.push_back(argv[2]); referenceImages.push_back(argv[3]);
    string LDFP_ERROR_IMAGE = argv[4];
    string LDFP_NUM_DIRECTIONS = argv[5];
    string LDFP_VECTOR_FIELD = argv[6];

    float angularThreshold = 25;

    try
    {
        RegisterDiffusionCoreObjectFactory();
        RegisterFiberTrackingObjectFactory();

        typedef itk::Image<unsigned char, 3>                                    ItkUcharImgType;
        typedef itk::Image< itk::Vector< float, 3>, 3 >                         ItkDirectionImage3DType;
        typedef itk::VectorContainer< int, ItkDirectionImage3DType::Pointer >   ItkDirectionImageContainerType;
        typedef itk::EvaluateDirectionImagesFilter< float >                     EvaluationFilterType;

        // load fiber bundle
        mitk::FiberBundleX::Pointer inputTractogram = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(fibFile)->GetData());

        // load reference directions
        ItkDirectionImageContainerType::Pointer referenceImageContainer = ItkDirectionImageContainerType::New();
        for (int i=0; i<referenceImages.size(); i++)
        {
            try
            {
                mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(referenceImages.at(i))->GetData());
                typedef mitk::ImageToItk< ItkDirectionImage3DType > CasterType;
                CasterType::Pointer caster = CasterType::New();
                caster->SetInput(img);
                caster->Update();
                ItkDirectionImage3DType::Pointer itkImg = caster->GetOutput();
                referenceImageContainer->InsertElement(referenceImageContainer->Size(),itkImg);
            }
            catch(...){ MITK_INFO << "could not load: " << referenceImages.at(i); }
        }

        ItkUcharImgType::Pointer itkMaskImage = ItkUcharImgType::New();
        ItkDirectionImage3DType::Pointer dirImg = referenceImageContainer->GetElement(0);
        itkMaskImage->SetSpacing( dirImg->GetSpacing() );
        itkMaskImage->SetOrigin( dirImg->GetOrigin() );
        itkMaskImage->SetDirection( dirImg->GetDirection() );
        itkMaskImage->SetLargestPossibleRegion( dirImg->GetLargestPossibleRegion() );
        itkMaskImage->SetBufferedRegion( dirImg->GetLargestPossibleRegion() );
        itkMaskImage->SetRequestedRegion( dirImg->GetLargestPossibleRegion() );
        itkMaskImage->Allocate();
        itkMaskImage->FillBuffer(1);

        // extract directions from fiber bundle
        itk::TractsToVectorImageFilter<float>::Pointer fOdfFilter = itk::TractsToVectorImageFilter<float>::New();
        fOdfFilter->SetFiberBundle(inputTractogram);
        fOdfFilter->SetMaskImage(itkMaskImage);
        fOdfFilter->SetAngularThreshold(cos(angularThreshold*M_PI/180));
        fOdfFilter->SetNormalizeVectors(true);
        fOdfFilter->SetUseWorkingCopy(false);
        fOdfFilter->Update();
        ItkDirectionImageContainerType::Pointer directionImageContainer = fOdfFilter->GetDirectionImageContainer();

        // evaluate directions
        EvaluationFilterType::Pointer evaluationFilter = EvaluationFilterType::New();
        evaluationFilter->SetImageSet(directionImageContainer);
        evaluationFilter->SetReferenceImageSet(referenceImageContainer);
        evaluationFilter->SetMaskImage(itkMaskImage);
        evaluationFilter->SetIgnoreMissingDirections(false);
        evaluationFilter->Update();

        EvaluationFilterType::OutputImageType::Pointer angularErrorImage = evaluationFilter->GetOutput(0);
        ItkUcharImgType::Pointer numDirImage = fOdfFilter->GetNumDirectionsImage();
        mitk::FiberBundleX::Pointer testDirections = fOdfFilter->GetOutputFiberBundle();

        mitk::Image::Pointer mitkAngularErrorImage = mitk::Image::New();
        mitkAngularErrorImage->InitializeByItk( angularErrorImage.GetPointer() );
        mitkAngularErrorImage->SetVolume( angularErrorImage->GetBufferPointer() );

        mitk::Image::Pointer mitkNumDirImage = mitk::Image::New();
        mitkNumDirImage->InitializeByItk( numDirImage.GetPointer() );
        mitkNumDirImage->SetVolume( numDirImage->GetBufferPointer() );

        mitk::Image::Pointer gtAngularErrorImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(LDFP_ERROR_IMAGE)->GetData());
        mitk::Image::Pointer gtNumTestDirImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(LDFP_NUM_DIRECTIONS)->GetData());
        mitk::FiberBundleX::Pointer gtTestDirections = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(LDFP_VECTOR_FIELD)->GetData());

        MITK_TEST_CONDITION_REQUIRED(mitk::Equal(gtAngularErrorImage, mitkAngularErrorImage, 0.0001, true), "Check if error images are equal.");
        MITK_TEST_CONDITION_REQUIRED(testDirections->Equals(gtTestDirections), "Check if vector fields are equal.");
        MITK_TEST_CONDITION_REQUIRED(mitk::Equal(gtNumTestDirImage, mitkNumDirImage, 0.0001, true), "Check if num direction images are equal.");

        evaluationFilter = EvaluationFilterType::New();
        evaluationFilter->SetImageSet(directionImageContainer);
        evaluationFilter->SetReferenceImageSet(referenceImageContainer);
        //evaluationFilter->SetMaskImage(itkMaskImage);
        evaluationFilter->SetIgnoreMissingDirections(true);
        evaluationFilter->Update();
        angularErrorImage = evaluationFilter->GetOutput(0);
        mitkAngularErrorImage = mitk::Image::New();
        mitkAngularErrorImage->InitializeByItk( angularErrorImage.GetPointer() );
        mitkAngularErrorImage->SetVolume( angularErrorImage->GetBufferPointer() );
        mitk::IOUtil::SaveImage(mitkAngularErrorImage, "test.nrrd");
        gtAngularErrorImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[10])->GetData());
        MITK_TEST_CONDITION_REQUIRED(mitk::Equal(gtAngularErrorImage, mitkAngularErrorImage, 0.0001, true), "Check if error images with ignored missing directions are equal.");


        for (int i=0; i<directionImageContainer->Size(); i++)
        {
            MITK_INFO << "Checking direction image " << i;
            itk::TractsToVectorImageFilter<float>::ItkDirectionImageType::Pointer itkImg = directionImageContainer->GetElement(i);
            mitk::Image::Pointer dirImage = mitk::Image::New();
            dirImage->InitializeByItk( itkImg.GetPointer() );
            dirImage->SetVolume( itkImg->GetBufferPointer() );

            mitk::Image::Pointer refDirImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[7+i])->GetData());
            MITK_TEST_CONDITION_REQUIRED(mitk::Equal(dirImage, refDirImage, 0.0001, true), "Check if direction images are equal.");
        }
    }
    catch (itk::ExceptionObject e)
    {
        MITK_INFO << e;
        return EXIT_FAILURE;
    }
    catch (std::exception e)
    {
        MITK_INFO << e.what();
        return EXIT_FAILURE;
    }
    catch (...)
    {
        MITK_INFO << "ERROR!?!";
        return EXIT_FAILURE;
    }
    MITK_TEST_END();
}
