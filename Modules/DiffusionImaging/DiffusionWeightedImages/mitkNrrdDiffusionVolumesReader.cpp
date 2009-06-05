/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkImageSeriesReader.txx,v $
  Language:  C++
  Date:      $Date: 2007-08-17 20:16:25 $
  Version:   $Revision: 1.31 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkNrrdDiffusionVolumesReader_cpp
#define __mitkNrrdDiffusionVolumesReader_cpp

#include "mitkNrrdDiffusionVolumesReader.h"

#include "itkImageFileReader.h"
#include "itkMetaDataObject.h"

namespace mitk
{

template <class TImageType>
void NrrdDiffusionVolumesReader<TImageType>
::GenerateData()
{
  typedef itk::ImageFileReader<TImageType> FileReaderType;
  typename FileReaderType::Pointer reader = FileReaderType::New();
  reader->SetFileName(this->m_FileName);
  reader->Update();

  this->SetNthOutput(0, reader->GetOutput() );
  typename TImageType::Pointer img = reader->GetOutput();

  itk::MetaDataDictionary imgMetaDictionary = img->GetMetaDataDictionary();    
  std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string metaString;

  GradientDirectionType vect3d;
  m_DiffusionVectors = GradientDirectionContainerType::New();

  int numberOfImages = 0;
  int numberOfGradientImages = 0;
  bool readb0 = false;

  for (; itKey != imgMetaKeys.end(); itKey ++)
  {
    double x,y,z;

    itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
    if (itKey->find("DWMRI_gradient") != std::string::npos)
    { 
      std::cout << *itKey << " ---> " << metaString << std::endl;      
      sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
      vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
      m_DiffusionVectors->InsertElement( numberOfImages, vect3d );
      ++numberOfImages;
      // If the direction is 0.0, this is a reference image
      if (vect3d[0] == 0.0 &&
        vect3d[1] == 0.0 &&
        vect3d[2] == 0.0)
      {
        continue;
      }
      ++numberOfGradientImages;;
    }
    else if (itKey->find("DWMRI_b-value") != std::string::npos)
    {
      std::cout << *itKey << " ---> " << metaString << std::endl;      
      readb0 = true;
      m_B_Value = atof(metaString.c_str());
    }
  }
  std::cout << "Number of gradient images: "
    << numberOfGradientImages
    << " and Number of reference images: "
    << numberOfImages - numberOfGradientImages
    << std::endl;
  if(!readb0)
  {
    std::cerr << "BValue not specified in header file" << std::endl;
  }

}

} //namespace MITK

#endif
