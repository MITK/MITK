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
#include "mitkDiffusionCoreObjectFactory.h"
#include "mitkCoreObjectFactory.h"
#include "mitkCoreExtObjectFactory.h"
#include "mitkImageWriter.h"

#include "itkDiffusionTensor3DReconstructionImageFilter.h"
#include "mitkTensorImage.h"


/**
 * Convert files from one ending to the other
 */
int TensorReconstruction(int argc, char* argv[])
{

  if ( argc!=3 )
  {
    std::cout << std::endl;
    std::cout << "Perform Tensor estimation on an dwi file" << std::endl;
    std::cout << std::endl;
    std::cout << "usage: " << argv[0] << "<in-filename> <out-filename> " << std::endl;
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





  typedef short DiffusionPixelType;
  typedef float TensorPixelType;

  typedef mitk::DiffusionImage<DiffusionPixelType> DiffusionImageType;


  std::string outfilename = argv[2];


  DiffusionImageType::Pointer vols;

  try
  {
    vols = dynamic_cast<DiffusionImageType*>(baseData.GetPointer());
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

  if(vols.IsNull())
  {
    return EXIT_FAILURE;
  }



  typedef float TTensorPixelType;

  typedef itk::DiffusionTensor3DReconstructionImageFilter<
          DiffusionPixelType, DiffusionPixelType, TTensorPixelType > TensorReconstructionImageFilterType;
  TensorReconstructionImageFilterType::Pointer filter =
          TensorReconstructionImageFilterType::New();
  filter->SetGradientImage( vols->GetDirections(), vols->GetVectorImage() );
  filter->SetBValue(vols->GetB_Value());
  filter->Update();


  mitk::TensorImage::Pointer image = mitk::TensorImage::New();
  image->InitializeByItk( filter->GetOutput() );
  image->SetVolume( filter->GetOutput()->GetBufferPointer() );


  // Save tensor image
  std::cout << "writing tensor image";

  std::string tensorName = outfilename + ".dti";
  mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
  for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
  {
    if ( (*it)->CanWriteBaseDataType(image.GetPointer()) ) {
      MITK_INFO << "writing";
      (*it)->SetFileName( tensorName.c_str() );
      (*it)->DoWrite( image.GetPointer() );
    }
  }


  return EXIT_SUCCESS;

}
RegisterDiffusionCoreMiniApp(TensorReconstruction);
