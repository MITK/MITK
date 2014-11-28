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
#include <mitkDiffusionImage.h>

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
  mitk::IOUtil::SaveImage(map, filename + "_dti_FA" + postfix + ".nrrd");

  // MD
  measurementsCalculator->SetMeasure(MeasurementsType::MD);
  measurementsCalculator->Update();
  map->InitializeByItk( measurementsCalculator->GetOutput() );
  map->SetVolume( measurementsCalculator->GetOutput()->GetBufferPointer() );
  mitk::IOUtil::SaveImage(map, filename + "_dti_MD" + postfix + ".nrrd");

  // AD
  measurementsCalculator->SetMeasure(MeasurementsType::AD);
  measurementsCalculator->Update();
  map->InitializeByItk( measurementsCalculator->GetOutput() );
  map->SetVolume( measurementsCalculator->GetOutput()->GetBufferPointer() );
  mitk::IOUtil::SaveImage(map, filename + "_dti_AD" + postfix + ".nrrd");


  // CA
  measurementsCalculator->SetMeasure(MeasurementsType::CA);
  measurementsCalculator->Update();
  map->InitializeByItk( measurementsCalculator->GetOutput() );
  map->SetVolume( measurementsCalculator->GetOutput()->GetBufferPointer() );
  mitk::IOUtil::SaveImage(map, filename + "_dti_CA" + postfix + ".nrrd");

  // RA
  measurementsCalculator->SetMeasure(MeasurementsType::RA);
  measurementsCalculator->Update();
  map->InitializeByItk( measurementsCalculator->GetOutput() );
  map->SetVolume( measurementsCalculator->GetOutput()->GetBufferPointer() );
  mitk::IOUtil::SaveImage(map, filename + "_dti_RA" + postfix + ".nrrd");

  // RD
  measurementsCalculator->SetMeasure(MeasurementsType::RD);
  measurementsCalculator->Update();
  map->InitializeByItk( measurementsCalculator->GetOutput() );
  map->SetVolume( measurementsCalculator->GetOutput()->GetBufferPointer() );
  mitk::IOUtil::SaveImage(map, filename + "_dti_RD" + postfix + ".nrrd");

}


int main(int argc, char* argv[])
{

    std::cout << "TensorDerivedMapsExtraction";
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("help", "h", mitkCommandLineParser::String, "Help", "Show this help text");
  parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input file", "input dwi file", us::Any(),false);
  parser.addArgument("out", "o", mitkCommandLineParser::String, "Output folder", "output folder and base name, e.g. /tmp/outPatient1 ", us::Any(),false);

  parser.setCategory("Diffusion Related Measures");
  parser.setTitle("Tensor Derived Maps Extraction");
  parser.setDescription("");
  parser.setContributor("MBI");

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0 || parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << "\n\nMiniApp Description: \nPerforms tensor reconstruction on DWI file," << endl;
    std::cout << "and computes tensor derived measures." << endl;
    std::cout << "\n\n For out parameter /tmp/outPatient1 it will produce :"<< endl;
    std::cout << " /tmp/outPatient1_dti.dti , /tmp/outPatient1_dti_FA.nrrd, ..."<< endl;
    std::cout << "\n\n Parameters:"<< endl;
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }



  std::string inputFile = us::any_cast<string>(parsedArgs["input"]);
  std::string baseFileName = us::any_cast<string>(parsedArgs["out"]);

  std::string dtiFileName = "_dti.dti";

  std::cout << "BaseFileName: " << baseFileName;


  mitk::Image::Pointer inputImage =  mitk::IOUtil::LoadImage(inputFile);
  mitk::DiffusionImage<short>* diffusionImage =   static_cast<mitk::DiffusionImage<short>*>(inputImage.GetPointer());
  if (diffusionImage == NULL) // does NULL pointer check make sense after static cast ?
  {
    MITK_ERROR << "Invalid Input Image. Must be DWI. Aborting.";
    return -1;
  }

  mitk::DiffusionImage<DiffusionPixelType>* vols = dynamic_cast <mitk::DiffusionImage<DiffusionPixelType>*> (inputImage.GetPointer());

  typedef itk::DiffusionTensor3DReconstructionImageFilter< DiffusionPixelType, DiffusionPixelType, TTensorPixelType > TensorReconstructionImageFilterType;
  TensorReconstructionImageFilterType::Pointer tensorReconstructionFilter = TensorReconstructionImageFilterType::New();

  typedef mitk::DiffusionImage<DiffusionPixelType> DiffusionImageType;
  typedef DiffusionImageType::GradientDirectionContainerType GradientDirectionContainerType;

  GradientDirectionContainerType::Pointer gradientContainerCopy = GradientDirectionContainerType::New();
  for(GradientDirectionContainerType::ConstIterator it = vols->GetDirections()->Begin(); it != vols->GetDirections()->End(); it++)
  {
    gradientContainerCopy->push_back(it.Value());
  }

  tensorReconstructionFilter->SetGradientImage( gradientContainerCopy, vols->GetVectorImage() );
  tensorReconstructionFilter->SetBValue(vols->GetReferenceBValue());
  tensorReconstructionFilter->SetThreshold(50);
  tensorReconstructionFilter->Update();

  typedef itk::Image<itk::DiffusionTensor3D<TTensorPixelType>, 3> TensorImageType;
  TensorImageType::Pointer tensorImage = tensorReconstructionFilter->GetOutput();
  tensorImage->SetDirection( vols->GetVectorImage()->GetDirection() );

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
