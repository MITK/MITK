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
#include <itkExceptionObject.h>
#include <itkImageFileWriter.h>
#include <mitkOdfImage.h>
#include <itkTensorDerivedMeasurementsFilter.h>
#include <itkDiffusionOdfGeneralizedFaImageFilter.h>
#include <mitkTensorImage.h>
#include "mitkCommandLineParser.h"
#include <boost/algorithm/string.hpp>
#include <itksys/SystemTools.hxx>
#include <itkMultiThreader.h>
#include <mitkIOUtil.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkImage.h>
#include <mitkShImage.h>
#include <mitkOdfImage.h>
#include <mitkTensorImage.h>
#include <mitkDiffusionPropertyHelper.h>
#include <itkAdcImageFilter.h>
#include <mitkDiffusionFunctionCollection.h>
#include <mitkLocaleSwitch.h>

/**
 *
 */
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Diffusion Indices");
  parser.setCategory("Diffusion Related Measures");
  parser.setDescription("Computes requested diffusion related measures");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input image (tensor, ODF or SH-coefficient image)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output image", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("index", "idx", mitkCommandLineParser::String, "Index:", "index (fa, gfa, ra, ad, rd, ca, l2, l3, md, adc)", us::Any(), false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inFileName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string index = us::any_cast<std::string>(parsedArgs["index"]);
  std::string outFileName = us::any_cast<std::string>(parsedArgs["o"]);

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(outFileName);
  if (ext.empty())
    outFileName += ".nii.gz";

  try
  {
    // load input image
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images", "SH Image", "ODF Image", "Tensor Image"}, {});
    auto input = mitk::IOUtil::Load<mitk::Image>(inFileName, &functor);

    bool is_odf = (dynamic_cast<mitk::ShImage*>(input.GetPointer()) || dynamic_cast<mitk::OdfImage*>(input.GetPointer()));
    bool is_dt = dynamic_cast<mitk::TensorImage*>(input.GetPointer());
    bool is_dw = mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(input);

    if (is_odf)
      MITK_INFO << "Input is ODF image";
    else if (is_dt)
      MITK_INFO << "Input is tensor image";
    else if (is_dw)
      MITK_INFO << "Input is dMRI";
    else
    {
      MITK_WARN << "Input is no ODF, SH, tensor or raw dMRI.";
      return EXIT_FAILURE;
    }

    mitk::LocaleSwitch localeSwitch("C");
    if( is_odf && index=="gfa" )
    {
      typedef itk::Vector<float,ODF_SAMPLING_SIZE> OdfVectorType;
      typedef itk::Image<OdfVectorType,3> OdfVectorImgType;

      OdfVectorImgType::Pointer itkvol;
      if (dynamic_cast<mitk::ShImage*>(input.GetPointer()))
        itkvol = mitk::convert::GetItkOdfFromShImage(input);
      else
        itkvol = mitk::convert::GetItkOdfFromOdfImage(input);

      typedef itk::DiffusionOdfGeneralizedFaImageFilter<float,float,ODF_SAMPLING_SIZE> GfaFilterType;
      GfaFilterType::Pointer gfaFilter = GfaFilterType::New();
      gfaFilter->SetInput(itkvol);
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
      gfaFilter->Update();

      itk::ImageFileWriter< itk::Image<float,3> >::Pointer fileWriter = itk::ImageFileWriter< itk::Image<float,3> >::New();
      fileWriter->SetInput(gfaFilter->GetOutput());
      fileWriter->SetFileName(outFileName);
      fileWriter->Update();
    }
    else if( is_dt )
    {
      typedef itk::Image< itk::DiffusionTensor3D<float>, 3 >    ItkTensorImage;
      mitk::TensorImage::Pointer mitkTensorImage = dynamic_cast<mitk::TensorImage*>(input.GetPointer());
      ItkTensorImage::Pointer itk_dti = ItkTensorImage::New();
      mitk::CastToItkImage(mitkTensorImage, itk_dti);

      typedef itk::TensorDerivedMeasurementsFilter<float> MeasurementsType;
      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itk_dti.GetPointer() );

      if(index=="fa")
        measurementsCalculator->SetMeasure(MeasurementsType::FA);
      else if(index=="ra")
        measurementsCalculator->SetMeasure(MeasurementsType::RA);
      else if(index=="ad")
        measurementsCalculator->SetMeasure(MeasurementsType::AD);
      else if(index=="rd")
        measurementsCalculator->SetMeasure(MeasurementsType::RD);
      else if(index=="ca")
        measurementsCalculator->SetMeasure(MeasurementsType::CA);
      else if(index=="l2")
        measurementsCalculator->SetMeasure(MeasurementsType::L2);
      else if(index=="l3")
        measurementsCalculator->SetMeasure(MeasurementsType::L3);
      else if(index=="md")
        measurementsCalculator->SetMeasure(MeasurementsType::MD);
      else
      {
        MITK_WARN << "No valid diffusion index for input image (tensor image) defined";
        return EXIT_FAILURE;
      }

      measurementsCalculator->Update();

      itk::ImageFileWriter< itk::Image<float,3> >::Pointer fileWriter = itk::ImageFileWriter< itk::Image<float,3> >::New();
      fileWriter->SetInput(measurementsCalculator->GetOutput());
      fileWriter->SetFileName(outFileName);
      fileWriter->Update();
    }
    else if(is_dw && (index=="adc" || index=="md"))
    {
      typedef itk::AdcImageFilter< short, double > FilterType;

      auto itkVectorImagePointer = mitk::DiffusionPropertyHelper::GetItkVectorImage(input);
      FilterType::Pointer filter = FilterType::New();
      filter->SetInput( itkVectorImagePointer );

      filter->SetGradientDirections( mitk::DiffusionPropertyHelper::GetGradientContainer(input) );
      filter->SetB_value( static_cast<double>(mitk::DiffusionPropertyHelper::GetReferenceBValue(input)) );
      if (index=="adc")
        filter->SetFitSignal(true);
      else
        filter->SetFitSignal(false);
      filter->Update();

      itk::ImageFileWriter< itk::Image<double,3> >::Pointer fileWriter = itk::ImageFileWriter< itk::Image<double,3> >::New();
      fileWriter->SetInput(filter->GetOutput());
      fileWriter->SetFileName(outFileName);
      fileWriter->Update();
    }
    else
      std::cout << "Diffusion index " << index << " not supported for supplied file type.";
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
