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

template<int shOrder>
typename itk::ShCoefficientImageImporter< float, shOrder >::OdfImageType::Pointer TemplatedConvertShCoeffs(mitk::Image::Pointer mitkImg)
{
  typedef itk::ShToOdfImageFilter< float, shOrder > ShConverterType;

  typename ShConverterType::InputImageType::Pointer itkvol = ShConverterType::InputImageType::New();
  mitk::CastToItkImage(mitkImg, itkvol);

  typename ShConverterType::Pointer converter = ShConverterType::New();
  converter->SetInput(itkvol);
  converter->Update();

  return converter->GetOutput();
}

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
  parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input image (tensor, ODF or SH-coefficient image)", us::Any(), false);
  parser.addArgument("parameters", "p", mitkCommandLineParser::InputFile, "Parameters:", "parameter file (.gtp)", us::Any(), false);
  parser.addArgument("mask", "m", mitkCommandLineParser::InputFile, "Mask:", "binary mask image");
  parser.addArgument("outFile", "o", mitkCommandLineParser::OutputFile, "Output:", "output fiber bundle (.fib)", us::Any(), false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inFileName = us::any_cast<std::string>(parsedArgs["input"]);
  std::string paramFileName = us::any_cast<std::string>(parsedArgs["parameters"]);
  std::string outFileName = us::any_cast<std::string>(parsedArgs["outFile"]);

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
      mitk::ShImage::Pointer shImage = dynamic_cast<mitk::ShImage*>(mitkImage.GetPointer());

      switch (shImage->ShOrder())
      {
      case 2:
        gibbsTracker->SetOdfImage(TemplatedConvertShCoeffs<2>(mitkImage));
        break;
      case 4:
        gibbsTracker->SetOdfImage(TemplatedConvertShCoeffs<4>(mitkImage));
        break;
      case 6:
        gibbsTracker->SetOdfImage(TemplatedConvertShCoeffs<6>(mitkImage));
        break;
      case 8:
        gibbsTracker->SetOdfImage(TemplatedConvertShCoeffs<8>(mitkImage));
        break;
      case 10:
        gibbsTracker->SetOdfImage(TemplatedConvertShCoeffs<10>(mitkImage));
        break;
      case 12:
        gibbsTracker->SetOdfImage(TemplatedConvertShCoeffs<12>(mitkImage));
        break;
      default:
        std::cout << "SH-order " << shImage->ShOrder() << " not supported";
      }
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
