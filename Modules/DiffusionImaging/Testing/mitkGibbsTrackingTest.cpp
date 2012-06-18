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

#include <mitkDiffusionImagingObjectFactory.h>
#include <mitkFiberBundleX.h>
#include <mitkFiberBundleXReader.h>
#include <mitkQBallImage.h>
#include <mitkNrrdQBallImageReader.h>
#include <mitkImage.h>
#include <mitkItkImageFileReader.h>
#include <itkGibbsTrackingFilter.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkBaseData.h>
#include <itksys/SystemTools.hxx>
#include <mitkTestingConfig.h>
#include <math.h>
#include <tinyxml.h>

using namespace mitk;

/**Documentation
 *  Test for gibbs tracking filter
 */
int mitkGibbsTrackingTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkGibbsTrackingTest");

  MITK_TEST_CONDITION_REQUIRED(argc>4,"check for input data")

  NrrdQBallImageReader::Pointer qbiReader = mitk::NrrdQBallImageReader::New();
//  QBallImage::Pointer mitkQballImage;
//  MITK_TEST_CONDITION_REQUIRED(qbiReader.IsNotNull(),"qball reader instantiation")

  ItkImageFileReader::Pointer imageReader = mitk::ItkImageFileReader::New();
  Image::Pointer mitkMaskImage;
  MITK_TEST_CONDITION_REQUIRED(imageReader.IsNotNull(),"image reader instantiation")

  mitk::FiberBundleXReader::Pointer fibReader = mitk::FiberBundleXReader::New();
  mitk::FiberBundleX::Pointer fib1, fib2;
  MITK_TEST_CONDITION_REQUIRED(fibReader.IsNotNull(),"fiber bundle reader instantiation")

  try{
    RegisterDiffusionImagingObjectFactory();

    // test if fib1 can be read
    const std::string s1="", s2="";
    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::BaseDataIO::LoadBaseDataFromFile( argv[1], s1, s2, false );
    fib1 = dynamic_cast<mitk::FiberBundleX*>(fibInfile.at(0).GetPointer());
    MITK_TEST_CONDITION_REQUIRED(fib1.IsNotNull(),"check if reader 1 returned null")



    typedef itk::Vector<float, QBALL_ODFSIZE> OdfVectorType;
    typedef itk::Image<OdfVectorType,3> OdfVectorImgType;
    typedef itk::Image<float,3> MaskImgType;
    typedef itk::GibbsTrackingFilter<OdfVectorImgType> GibbsTrackingFilterType;

    MITK_TEST_CONDITION_REQUIRED(fib1->Equals(fib2),"check if equals method is working");

  }
  catch(...)
  {
    //this means that a wrong exception (i.e. no itk:Exception) has been thrown
    std::cout << "Wrong exception (i.e. no itk:Exception) caught during write [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  // always end with this!
  MITK_TEST_END();
}
