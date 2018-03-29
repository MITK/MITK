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


#include <mitkIOUtil.h>

#include "mitkImage.h"
#include <mitkImageCast.h>
#include "mitkITKImageImport.h"
#include <iostream>
#include <fstream>
#include <mitkTensorImage.h>
#include <mitkImage.h>
#include <mitkDiffusionPropertyHelper.h>

#include "itkTensorDerivedMeasurementsFilter.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"
#include "mitkCommandLineParser.h"

#include <itkImageFileWriter.h>
#include <itkNrrdImageIO.h>
#include <itkDiffusionTensor3D.h>

typedef short DiffusionPixelType;
typedef double TTensorPixelType;

static void ExtractMapsAndSave(mitk::TensorImage::Pointer tensorImage, std::string filename, std::string postfix = "")
{

  mitk::Image* image = dynamic_cast<mitk::Image*> (tensorImage.GetPointer());

  typedef itk::DiffusionTensor3D< TTensorPixelType >  TensorPixelType;
  typedef itk::Image< TensorPixelType, 3 > TensorImageType;

  TensorImageType::Pointer itkvol = TensorImageType::New();
  mitk::CastToItkImage(image, itkvol);

  typedef itk::TensorDerivedMeasurementsFilter<TTensorPixelType> MeasurementsType;

  MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
  measurementsCalculator->SetInput(itkvol.GetPointer() );

  mitk::Image::Pointer map = mitk::Image::New();
  // FA
  measurementsCalculator->SetMeasure(MeasurementsType::FA);
  measurementsCalculator->Update();
  map->InitializeByItk( measurementsCalculator->GetOutput() );
  map->SetVolume( measurementsCalculator->GetOutput()->GetBufferPointer() );
  mitk::IOUtil::Save(map, filename + "_FA" + postfix + ".nrrd");

  // MD
  measurementsCalculator->SetMeasure(MeasurementsType::MD);
  measurementsCalculator->Update();
  map->InitializeByItk( measurementsCalculator->GetOutput() );
  map->SetVolume( measurementsCalculator->GetOutput()->GetBufferPointer() );
  mitk::IOUtil::Save(map, filename + "_MD" + postfix + ".nrrd");

  // AD
  measurementsCalculator->SetMeasure(MeasurementsType::AD);
  measurementsCalculator->Update();
  map->InitializeByItk( measurementsCalculator->GetOutput() );
  map->SetVolume( measurementsCalculator->GetOutput()->GetBufferPointer() );
  mitk::IOUtil::Save(map, filename + "_AD" + postfix + ".nrrd");


  // CA
  measurementsCalculator->SetMeasure(MeasurementsType::CA);
  measurementsCalculator->Update();
  map->InitializeByItk( measurementsCalculator->GetOutput() );
  map->SetVolume( measurementsCalculator->GetOutput()->GetBufferPointer() );
  mitk::IOUtil::Save(map, filename + "_CA" + postfix + ".nrrd");

  // RA
  measurementsCalculator->SetMeasure(MeasurementsType::RA);
  measurementsCalculator->Update();
  map->InitializeByItk( measurementsCalculator->GetOutput() );
  map->SetVolume( measurementsCalculator->GetOutput()->GetBufferPointer() );
  mitk::IOUtil::Save(map, filename + "_RA" + postfix + ".nrrd");

  // RD
  measurementsCalculator->SetMeasure(MeasurementsType::RD);
  measurementsCalculator->Update();
  map->InitializeByItk( measurementsCalculator->GetOutput() );
  map->SetVolume( measurementsCalculator->GetOutput()->GetBufferPointer() );
  mitk::IOUtil::Save(map, filename + "_RD" + postfix + ".nrrd");

}


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("help", "h", mitkCommandLineParser::String, "Help", "Show this help text");
  parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input file", "input dwi file", us::Any(),false);
  parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output folder", "output folder and base name, e.g. /tmp/outPatient1 ", us::Any(),false);

  parser.setTitle("Tensor Derived Maps Extraction");
  parser.setCategory("Diffusion Related Measures");
  parser.setDescription("");
  parser.setContributor("MIC");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }


  std::string inputFile = us::any_cast<std::string>(parsedArgs["input"]);
  std::string baseFileName = us::any_cast<std::string>(parsedArgs["out"]);

  std::string dtiFileName = "_dti.dti";

  mitk::Image::Pointer diffusionImage = mitk::IOUtil::Load<mitk::Image>(inputFile);

  if (diffusionImage.IsNull() || !mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(diffusionImage)) // does nullptr pointer check make sense after static cast ?
  {
    MITK_ERROR << "Invalid Input Image. Must be DWI. Aborting.";
    return -1;
  }

  typedef itk::DiffusionTensor3DReconstructionImageFilter< DiffusionPixelType, DiffusionPixelType, TTensorPixelType > TensorReconstructionImageFilterType;
  TensorReconstructionImageFilterType::Pointer tensorReconstructionFilter = TensorReconstructionImageFilterType::New();

  mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer gradientContainerCopy = mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::New();
  for( mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::ConstIterator it = mitk::DiffusionPropertyHelper::GetGradientContainer(diffusionImage)->Begin(); it != mitk::DiffusionPropertyHelper::GetGradientContainer(diffusionImage)->End(); it++)
  {
    gradientContainerCopy->push_back(it.Value());
  }

  mitk::DiffusionPropertyHelper::ImageType::Pointer itkVectorImagePointer = mitk::DiffusionPropertyHelper::ImageType::New();
  mitk::CastToItkImage(diffusionImage, itkVectorImagePointer);

  tensorReconstructionFilter->SetGradientImage( gradientContainerCopy, itkVectorImagePointer );
  tensorReconstructionFilter->SetBValue( mitk::DiffusionPropertyHelper::GetReferenceBValue( diffusionImage ) );
  tensorReconstructionFilter->SetThreshold(50);
  tensorReconstructionFilter->Update();

  typedef itk::Image<itk::DiffusionTensor3D<TTensorPixelType>, 3> TensorImageType;
  TensorImageType::Pointer tensorImage = tensorReconstructionFilter->GetOutput();
  tensorImage->SetDirection( itkVectorImagePointer->GetDirection() );

  mitk::TensorImage::Pointer tensorImageMitk = mitk::TensorImage::New();
  tensorImageMitk->InitializeByItk(tensorImage.GetPointer());
  tensorImageMitk->SetVolume( tensorImage->GetBufferPointer() );



  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  io->SetFileType( itk::ImageIOBase::Binary );
  io->UseCompressionOn();

  itk::ImageFileWriter< itk::Image< itk::DiffusionTensor3D< double >, 3 > >::Pointer writer = itk::ImageFileWriter< itk::Image< itk::DiffusionTensor3D< double >, 3 > >::New();
  writer->SetInput(tensorReconstructionFilter->GetOutput());
  writer->SetFileName(baseFileName + dtiFileName);
  writer->SetImageIO(io);
  writer->UseCompressionOn();
  writer->Update();

  ExtractMapsAndSave(tensorImageMitk,baseFileName);

  return EXIT_SUCCESS;

}
