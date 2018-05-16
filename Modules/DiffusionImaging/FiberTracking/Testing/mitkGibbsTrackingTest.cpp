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
#include <mitkImageCast.h>
#include <mitkOdfImage.h>
#include <itkGibbsTrackingFilter.h>
#include <mitkFiberBundle.h>
#include <mitkIOUtil.h>

using namespace mitk;

/**Documentation
 *  Test for gibbs tracking filter
 */
int mitkGibbsTrackingTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkGibbsTrackingTest");

  MITK_TEST_CONDITION_REQUIRED(argc>4,"check for input data")

  OdfImage::Pointer mitkOdfImage;
  Image::Pointer mitkMaskImage;
  mitk::FiberBundle::Pointer fib1;

  try{

    MITK_INFO << "ODF image: " << argv[1];
    MITK_INFO << "Mask image: " << argv[2];
    MITK_INFO << "Parameter file: " << argv[3];
    MITK_INFO << "Reference bundle: " << argv[4];

    std::vector<mitk::BaseData::Pointer> infile = mitk::IOUtil::Load( argv[1]);
    mitkOdfImage = dynamic_cast<mitk::OdfImage*>(infile.at(0).GetPointer());
    MITK_TEST_CONDITION_REQUIRED(mitkOdfImage.IsNotNull(),"check Odf image")

    infile = mitk::IOUtil::Load( argv[2] );
    mitkMaskImage = dynamic_cast<mitk::Image*>(infile.at(0).GetPointer());
    MITK_TEST_CONDITION_REQUIRED(mitkMaskImage.IsNotNull(),"check mask image")

    infile = mitk::IOUtil::Load( argv[4]);
    fib1 = dynamic_cast<mitk::FiberBundle*>(infile.at(0).GetPointer());
    MITK_TEST_CONDITION_REQUIRED(fib1.IsNotNull(),"check fiber bundle")

    typedef itk::Vector<float, ODF_SAMPLING_SIZE> OdfVectorType;
    typedef itk::Image<OdfVectorType,3> OdfVectorImgType;
    typedef itk::Image<float,3> MaskImgType;
    typedef itk::GibbsTrackingFilter<OdfVectorImgType> GibbsTrackingFilterType;

    OdfVectorImgType::Pointer itk_odf = OdfVectorImgType::New();
    mitk::CastToItkImage(mitkOdfImage, itk_odf);

    MaskImgType::Pointer itk_mask = MaskImgType::New();
    mitk::CastToItkImage(mitkMaskImage, itk_mask);

    GibbsTrackingFilterType::Pointer gibbsTracker = GibbsTrackingFilterType::New();
    gibbsTracker->SetOdfImage(itk_odf.GetPointer());
    gibbsTracker->SetMaskImage(itk_mask);
    gibbsTracker->SetDuplicateImage(false);
    gibbsTracker->SetRandomSeed(1);
    gibbsTracker->SetLoadParameterFile(argv[3]);
    gibbsTracker->Update();

    mitk::FiberBundle::Pointer fib2 = mitk::FiberBundle::New(gibbsTracker->GetFiberBundle());
    MITK_TEST_CONDITION_REQUIRED(fib1->Equals(fib2), "check if gibbs tracking has changed");

    gibbsTracker->SetRandomSeed(0);
    gibbsTracker->Update();
    fib2 = mitk::FiberBundle::New(gibbsTracker->GetFiberBundle());
    MITK_TEST_CONDITION_REQUIRED(!fib1->Equals(fib2), "check if gibbs tracking has changed after wrong seed");
  }
  catch(...)
  {
    return EXIT_FAILURE;
  }

  // always end with this!
  MITK_TEST_END();
}
