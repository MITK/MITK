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

#include "MiniAppManager.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

#include <itkImage.h>
#include <itkImageFileReader.h>

#include <itkExceptionObject.h>
#include <itkImageFileWriter.h>

#include <itkMetaDataObject.h>

#include "itkVectorContainer.h"
#include "vnl/vnl_vector_fixed.h"
#include "itkVectorImage.h"

#include "mitkBaseDataIOFactory.h"
#include "mitkDiffusionImage.h"
#include "mitkBaseData.h"
#include <mitkDiffusionCoreObjectFactory.h>
#include "mitkCoreObjectFactory.h"
#include "mitkCoreExtObjectFactory.h"
#include "mitkImageWriter.h"

#include "itkTensorDerivedMeasurementsFilter.h"
#include "mitkTensorImage.h"

#include "itkImageFileWriter.h"


/**
 * Convert files from one ending to the other
 */
int DiffusionIndices(int argc, char* argv[])
{

  if ( argc!=4 )
  {
    std::cout << std::endl;
    std::cout << "Perform Tensor estimation on an dwi file" << std::endl;
    std::cout << std::endl;
    std::cout << "usage: " << argv[0] << "<in-filename> <out-filename> <diffusion-index>" << std::endl;
    std::cout << std::endl;
    return EXIT_FAILURE;
  }

  mitk::BaseData::Pointer baseData = 0;

  try
  {
    mitk::CoreObjectFactory::GetInstance();

    RegisterDiffusionCoreObjectFactory();
    RegisterCoreExtObjectFactory() ;

    std::string filename = argv[1];
    const std::string s1="", s2="";
    std::vector<mitk::BaseData::Pointer> infile = mitk::BaseDataIO::LoadBaseDataFromFile( filename, s1, s2, false );
    if( infile.empty() )
    {
      MITK_INFO << "File could not be read" ;
    }
    baseData = infile.at(0);
  }
  catch ( itk::ExceptionObject &err)
  {
    MITK_INFO << "Exception during read: " << err;
  }
  catch ( std::exception err)
  {
    MITK_INFO << "Exception during read: " << err.what();
  }
  catch ( ... )
  {
    MITK_INFO << "Exception during read!";
  }


  MITK_INFO << "read file";


  typedef mitk::TensorImage TensorImageType;

  std::string outfilename = argv[2];

  TensorImageType::Pointer vol;

  try
  {
    vol = dynamic_cast<TensorImageType*>(baseData.GetPointer());
  }
  catch ( itk::ExceptionObject &err)
  {
    MITK_INFO << "Exception during Image write: " << err;
  }
  catch ( std::exception err)
  {
    MITK_INFO << "Exception during Image write: " << err.what();
  }
  catch ( ... )
  {
    MITK_INFO << "Exception during Image write";
  }

  if(vol.IsNull())
  {
    return EXIT_FAILURE;
  }



  typedef float TTensorPixelType;


  typedef itk::DiffusionTensor3D< TTensorPixelType >  TensorPixelType;
  typedef itk::Image< TensorPixelType, 3 >            ItkTensorImageType;

  ItkTensorImageType::Pointer itkvol = ItkTensorImageType::New();
  mitk::CastToItkImage<ItkTensorImageType>(vol, itkvol);

  std::string measurementType = argv[3];

  typedef itk::TensorDerivedMeasurementsFilter<TTensorPixelType> MeasurementsType;
  MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
  measurementsCalculator->SetInput(itkvol.GetPointer() );

  if(measurementType=="fa")
    measurementsCalculator->SetMeasure(MeasurementsType::FA);
  else if(measurementType=="ra")
    measurementsCalculator->SetMeasure(MeasurementsType::RA);
  else if(measurementType=="da")
    measurementsCalculator->SetMeasure(MeasurementsType::AD);
  else if(measurementType=="dr")
    measurementsCalculator->SetMeasure(MeasurementsType::RD);
  else if(measurementType=="ca")
    measurementsCalculator->SetMeasure(MeasurementsType::CA);
  else if(measurementType=="l2")
    measurementsCalculator->SetMeasure(MeasurementsType::L2);
  else if(measurementType=="l3")
    measurementsCalculator->SetMeasure(MeasurementsType::L3);
  else if(measurementType=="md")
    measurementsCalculator->SetMeasure(MeasurementsType::MD);
  else
    measurementsCalculator->SetMeasure(MeasurementsType::FA);

  measurementsCalculator->Update();

  MITK_INFO << "fa calculated";

  itk::Image<float, 3>::Pointer outputImage = measurementsCalculator->GetOutput();


  // Save tensor image
  MITK_INFO << "writing image";

  std::string tensorName = outfilename + ".nii.gz";


  MITK_INFO << "use itk filewriter";

  itk::ImageFileWriter< itk::Image<float,3> >::Pointer fileWriter = itk::ImageFileWriter< itk::Image<float,3> >::New();
  fileWriter->SetInput(outputImage);
  fileWriter->SetFileName(tensorName);
  fileWriter->Update();

  return EXIT_SUCCESS;
}

RegisterDiffusionCoreMiniApp(DiffusionIndices);
