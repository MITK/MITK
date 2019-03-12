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

#include <mitkImageCast.h>
#include <mitkOdfImage.h>
#include <mitkTensorImage.h>
#include <mitkFiberBundle.h>
#include <itkGibbsTrackingFilter.h>
#include <itkDiffusionTensor3D.h>
#include <itkShCoefficientImageImporter.h>
#include <itkShToOdfImageFilter.h>
#include <mitkImageToItk.h>
#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"
#include <boost/algorithm/string.hpp>
#include <itkFlipImageFilter.h>
#include <mitkCoreObjectFactory.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkShImage.h>
#include <mitkDiffusionFunctionCollection.h>

/*!
\brief Perform global fiber tractography (Gibbs tractography)
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Gibbs Tracking");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setDescription("Perform global fiber tractography (Gibbs tractography)");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input image (tensor, ODF or SH-coefficient image)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output tractogram", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("parameters", "", mitkCommandLineParser::String, "Parameters:", "parameter file (.gtp)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask", "", mitkCommandLineParser::String, "Mask:", "binary mask image", us::Any(), false, false, false, mitkCommandLineParser::Input);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inFileName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string paramFileName = us::any_cast<std::string>(parsedArgs["parameters"]);
  std::string outFileName = us::any_cast<std::string>(parsedArgs["o"]);

  try
  {
    // instantiate gibbs tracker
    typedef itk::Vector<float, ODF_SAMPLING_SIZE>   OdfVectorType;
    typedef itk::Image<OdfVectorType,3>         ItkOdfImageType;
    typedef itk::GibbsTrackingFilter<ItkOdfImageType> GibbsTrackingFilterType;
    GibbsTrackingFilterType::Pointer gibbsTracker = GibbsTrackingFilterType::New();

    // load input image
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"SH Image"}, {});
    mitk::Image::Pointer mitkImage = mitk::IOUtil::Load<mitk::Image>(inFileName, &functor);

    // try to cast to Odf image
    if( dynamic_cast<mitk::OdfImage*>(mitkImage.GetPointer()) )
    {
      mitk::OdfImage::Pointer mitkOdfImage = dynamic_cast<mitk::OdfImage*>(mitkImage.GetPointer());
      ItkOdfImageType::Pointer itk_odf = ItkOdfImageType::New();
      mitk::CastToItkImage(mitkOdfImage, itk_odf);
      gibbsTracker->SetOdfImage(itk_odf.GetPointer());
    }
    else if( dynamic_cast<mitk::TensorImage*>(mitkImage.GetPointer()) )
    {
      typedef itk::Image< itk::DiffusionTensor3D<float>, 3 >    ItkTensorImage;
      mitk::TensorImage::Pointer mitkTensorImage = dynamic_cast<mitk::TensorImage*>(mitkImage.GetPointer());
      ItkTensorImage::Pointer itk_dti = ItkTensorImage::New();
      mitk::CastToItkImage(mitkTensorImage, itk_dti);
      gibbsTracker->SetTensorImage(itk_dti);
    }
    else if ( dynamic_cast<mitk::ShImage*>(mitkImage.GetPointer()) )
    {
      mitk::Image::Pointer shImage = dynamic_cast<mitk::Image*>(mitkImage.GetPointer());
      gibbsTracker->SetOdfImage(mitk::convert::GetItkOdfFromShImage(shImage));
    }
    else
      return EXIT_FAILURE;

    // global tracking
    if (parsedArgs.count("mask"))
    {
      typedef itk::Image<float,3> MaskImgType;
      mitk::Image::Pointer mitkMaskImage = mitk::IOUtil::Load<mitk::Image>(us::any_cast<std::string>(parsedArgs["mask"]));
      MaskImgType::Pointer itk_mask = MaskImgType::New();
      mitk::CastToItkImage(mitkMaskImage, itk_mask);
      gibbsTracker->SetMaskImage(itk_mask);
    }

    gibbsTracker->SetDuplicateImage(false);
    gibbsTracker->SetLoadParameterFile( paramFileName );
    //        gibbsTracker->SetLutPath( "" );
    gibbsTracker->Update();

    mitk::FiberBundle::Pointer mitkFiberBundle = mitk::FiberBundle::New(gibbsTracker->GetFiberBundle());
    mitkFiberBundle->SetReferenceGeometry(mitkImage->GetGeometry());

    mitk::IOUtil::Save(mitkFiberBundle, outFileName );
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "ERROR!?!";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
