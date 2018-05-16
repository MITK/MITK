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
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <itkShToOdfImageFilter.h>
#include <mitkShImage.h>

template<int ShOrder>
mitk::OdfImage::Pointer TemplatedConvertShImage(mitk::ShImage::Pointer mitkImage)
{
  typedef itk::ShToOdfImageFilter< float, ShOrder > ShConverterType;
  typename ShConverterType::InputImageType::Pointer itkvol = ShConverterType::InputImageType::New();
  mitk::CastToItkImage(mitkImage, itkvol);

  typename ShConverterType::Pointer converter = ShConverterType::New();
  converter->SetInput(itkvol);
  converter->Update();

  mitk::OdfImage::Pointer image = mitk::OdfImage::New();
  image->InitializeByItk( converter->GetOutput() );
  image->SetVolume( converter->GetOutput()->GetBufferPointer() );
  return image;
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("ShToOdfImage");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("Calculate discrete ODF image from SH coefficient image");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("in", "i", mitkCommandLineParser::InputFile, "Input:", "input image", us::Any(), false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output:", "output image", us::Any(), false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string imageName = us::any_cast<std::string>(parsedArgs["in"]);
  std::string outImage = us::any_cast<std::string>(parsedArgs["out"]);

  try
  {
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"SH Image"}, {});
    mitk::ShImage::Pointer source = mitk::IOUtil::Load<mitk::ShImage>(imageName, &functor);

    mitk::OdfImage::Pointer out_image = nullptr;
    switch (source->ShOrder())
    {
    case 2:
      out_image = TemplatedConvertShImage<2>(source);
      break;
    case 4:
      out_image = TemplatedConvertShImage<4>(source);
      break;
    case 6:
      out_image = TemplatedConvertShImage<6>(source);
      break;
    case 8:
      out_image = TemplatedConvertShImage<8>(source);
      break;
    case 10:
      out_image = TemplatedConvertShImage<10>(source);
      break;
    case 12:
      out_image = TemplatedConvertShImage<12>(source);
      break;
    }

    if (out_image.IsNotNull())
      mitk::IOUtil::Save(out_image, outImage);
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
