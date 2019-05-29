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

#include "mitkImage.h"
#include <mitkImageCast.h>
#include "mitkBaseData.h"
#include <mitkDiffusionPropertyHelper.h>

#include <itkTensorReconstructionWithEigenvalueCorrectionFilter.h>
#include <itkDiffusionTensor3DReconstructionImageFilter.h>
#include <itkDiffusionTensor3D.h>
#include <itkImageFileWriter.h>
#include <itkNrrdImageIO.h>
#include "mitkCommandLineParser.h"
#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>
#include <mitkLocaleSwitch.h>

/**
 * Convert files from one ending to the other
 */
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input image", "input raw dwi", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output image", "output image", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("b0_threshold", "", mitkCommandLineParser::Int, "b0 threshold", "baseline image intensity threshold", 0);
  parser.addArgument("correct_negative_eigenv", "", mitkCommandLineParser::Bool, "Correct negative eigenvalues", "correct negative eigenvalues", us::Any(false));

  parser.setCategory("Signal Modelling");
  parser.setTitle("Tensor Reconstruction");
  parser.setDescription("");
  parser.setContributor("MIC");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inFileName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string outfilename = us::any_cast<std::string>(parsedArgs["o"]);
  if (!itksys::SystemTools::GetFilenamePath(outfilename).empty())
    outfilename = itksys::SystemTools::GetFilenamePath(outfilename)+"/"+itksys::SystemTools::GetFilenameWithoutExtension(outfilename);
  else
    outfilename = itksys::SystemTools::GetFilenameWithoutExtension(outfilename);
  outfilename += ".dti";

  int threshold = 0;
  if (parsedArgs.count("b0_threshold"))
    threshold = us::any_cast<int>(parsedArgs["b0_threshold"]);

  bool correct_negative_eigenv = false;
  if (parsedArgs.count("correct_negative_eigenv"))
    correct_negative_eigenv = us::any_cast<bool>(parsedArgs["correct_negative_eigenv"]);

  try
  {
    mitk::Image::Pointer dwi = mitk::IOUtil::Load<mitk::Image>(inFileName);

    mitk::DiffusionPropertyHelper::ImageType::Pointer itkVectorImagePointer = mitk::DiffusionPropertyHelper::ImageType::New();
    mitk::CastToItkImage(dwi, itkVectorImagePointer);

    if (correct_negative_eigenv)
    {
      typedef itk::TensorReconstructionWithEigenvalueCorrectionFilter< short, float > TensorReconstructionImageFilterType;
      TensorReconstructionImageFilterType::Pointer filter = TensorReconstructionImageFilterType::New();
      filter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(dwi));
      filter->SetGradientImage( mitk::DiffusionPropertyHelper::GetGradientContainer(dwi), itkVectorImagePointer);
      filter->SetB0Threshold(threshold);
      filter->Update();

      // Save tensor image
      itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
      io->SetFileType( itk::ImageIOBase::Binary );
      io->UseCompressionOn();

      mitk::LocaleSwitch localeSwitch("C");
      itk::ImageFileWriter< itk::Image< itk::DiffusionTensor3D< float >, 3 > >::Pointer writer = itk::ImageFileWriter< itk::Image< itk::DiffusionTensor3D< float >, 3 > >::New();
      writer->SetInput(filter->GetOutput());
      writer->SetFileName(outfilename);
      writer->SetImageIO(io);
      writer->UseCompressionOn();
      writer->Update();
    }
    else {
      typedef itk::DiffusionTensor3DReconstructionImageFilter< short, short, float > TensorReconstructionImageFilterType;
      TensorReconstructionImageFilterType::Pointer filter = TensorReconstructionImageFilterType::New();
      filter->SetGradientImage( mitk::DiffusionPropertyHelper::GetGradientContainer(dwi), itkVectorImagePointer );
      filter->SetBValue( mitk::DiffusionPropertyHelper::GetReferenceBValue( dwi ));
      filter->SetThreshold(threshold);
      filter->Update();

      // Save tensor image
      itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
      io->SetFileType( itk::ImageIOBase::Binary );
      io->UseCompressionOn();

      mitk::LocaleSwitch localeSwitch("C");
      itk::ImageFileWriter< itk::Image< itk::DiffusionTensor3D< float >, 3 > >::Pointer writer = itk::ImageFileWriter< itk::Image< itk::DiffusionTensor3D< float >, 3 > >::New();
      writer->SetInput(filter->GetOutput());
      writer->SetFileName(outfilename);
      writer->SetImageIO(io);
      writer->UseCompressionOn();
      writer->Update();
    }

  }
  catch ( itk::ExceptionObject &err)
  {
    std::cout << "Exception: " << err;
  }
  catch ( std::exception err)
  {
    std::cout << "Exception: " << err.what();
  }
  catch ( ... )
  {
    std::cout << "Exception!";
  }
  return EXIT_SUCCESS;

}
