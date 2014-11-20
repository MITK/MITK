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
#include <mitkCoreObjectFactory.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

int mitkLocalFiberPlausibilityTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkLocalFiberPlausibilityTest");
    MITK_TEST_CONDITION_REQUIRED(argc==8,"check for input data")

            string fibFile = argv[1];
    vector< string > referenceImages;
    referenceImages.push_back(argv[2]);
    referenceImages.push_back(argv[3]);
    string LDFP_ERROR_IMAGE = argv[4];
    string LDFP_NUM_DIRECTIONS = argv[5];
    string LDFP_VECTOR_FIELD = argv[6];
    string LDFP_ERROR_IMAGE_IGNORE = argv[7];

    float angularThreshold = 25;

    try
    {
        typedef itk::Image<unsigned char, 3>                                    ItkUcharImgType;
        typedef itk::Image< itk::Vector< float, 3>, 3 >                         ItkDirectionImage3DType;
        typedef itk::VectorContainer< unsigned int, ItkDirectionImage3DType::Pointer >   ItkDirectionImageContainerType;
        typedef itk::EvaluateDirectionImagesFilter< float >                     EvaluationFilterType;

        // load fiber bundle
        mitk::FiberBundleX::Pointer inputTractogram = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(fibFile)->GetData());

        // load reference directions
        ItkDirectionImageContainerType::Pointer referenceImageContainer = ItkDirectionImageContainerType::New();
        for (unsigned int i=0; i<referenceImages.size(); i++)
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
        //fOdfFilter->SetMaxNumDirections(1);
        fOdfFilter->SetSizeThreshold(0.0);

        fOdfFilter->SetUseWorkingCopy(false);
        fOdfFilter->SetNumberOfThreads(1);
        fOdfFilter->Update();
        ItkDirectionImageContainerType::Pointer directionImageContainer = fOdfFilter->GetDirectionImageContainer();

        // Get directions and num directions image
        ItkUcharImgType::Pointer numDirImage = fOdfFilter->GetNumDirectionsImage();
        mitk::Image::Pointer mitkNumDirImage = mitk::Image::New();
        mitkNumDirImage->InitializeByItk( numDirImage.GetPointer() );
        mitkNumDirImage->SetVolume( numDirImage->GetBufferPointer() );
        mitk::FiberBundleX::Pointer testDirections = fOdfFilter->GetOutputFiberBundle();

        // evaluate directions with missing directions
        EvaluationFilterType::Pointer evaluationFilter = EvaluationFilterType::New();
        evaluationFilter->SetImageSet(directionImageContainer);
        evaluationFilter->SetReferenceImageSet(referenceImageContainer);
        evaluationFilter->SetMaskImage(itkMaskImage);
        evaluationFilter->SetIgnoreMissingDirections(false);
        evaluationFilter->Update();

        EvaluationFilterType::OutputImageType::Pointer angularErrorImage = evaluationFilter->GetOutput(0);
        mitk::Image::Pointer mitkAngularErrorImage = mitk::Image::New();
        mitkAngularErrorImage->InitializeByItk( angularErrorImage.GetPointer() );
        mitkAngularErrorImage->SetVolume( angularErrorImage->GetBufferPointer() );

        // evaluate directions without missing directions
        evaluationFilter->SetIgnoreMissingDirections(true);
        evaluationFilter->Update();

        EvaluationFilterType::OutputImageType::Pointer angularErrorImageIgnore = evaluationFilter->GetOutput(0);
        mitk::Image::Pointer mitkAngularErrorImageIgnore = mitk::Image::New();
        mitkAngularErrorImageIgnore->InitializeByItk( angularErrorImageIgnore.GetPointer() );
        mitkAngularErrorImageIgnore->SetVolume( angularErrorImageIgnore->GetBufferPointer() );

        mitk::Image::Pointer gtAngularErrorImageIgnore = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(LDFP_ERROR_IMAGE_IGNORE)->GetData());
        mitk::Image::Pointer gtAngularErrorImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(LDFP_ERROR_IMAGE)->GetData());
        mitk::Image::Pointer gtNumTestDirImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(LDFP_NUM_DIRECTIONS)->GetData());
        mitk::FiberBundleX::Pointer gtTestDirections = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(LDFP_VECTOR_FIELD)->GetData());

//        mitk::IOUtil::SaveBaseData(mitkAngularErrorImageIgnore, mitk::IOUtil::GetTempPath()+"mitkAngularErrorImageIgnore.nrrd");
//        mitk::IOUtil::SaveBaseData(mitkAngularErrorImage, mitk::IOUtil::GetTempPath()+"mitkAngularErrorImage.nrrd");
//        mitk::IOUtil::SaveBaseData(mitkNumDirImage, mitk::IOUtil::GetTempPath()+"mitkNumDirImage.nrrd");
//        mitk::IOUtil::SaveBaseData(testDirections, mitk::IOUtil::GetTempPath()+"testDirections.fib");

        MITK_TEST_CONDITION_REQUIRED(mitk::Equal(gtAngularErrorImageIgnore, mitkAngularErrorImageIgnore, 0.01, true), "Check if error images are equal (ignored missing directions).");
        MITK_TEST_CONDITION_REQUIRED(mitk::Equal(gtAngularErrorImage, mitkAngularErrorImage, 0.01, true), "Check if error images are equal.");
        MITK_TEST_CONDITION_REQUIRED(testDirections->Equals(gtTestDirections), "Check if vector fields are equal.");
        MITK_TEST_CONDITION_REQUIRED(mitk::Equal(gtNumTestDirImage, mitkNumDirImage, 0.1, true), "Check if num direction images are equal.");
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
