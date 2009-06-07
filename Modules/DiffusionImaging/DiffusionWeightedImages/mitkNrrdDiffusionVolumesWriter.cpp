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
#ifndef __mitkNrrdDiffusionVolumesWriter_cpp
#define __mitkNrrdDiffusionVolumesWriter_cpp

#include "mitkNrrdDiffusionVolumesWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include <itkNrrdImageIO.h>
#include "itkImageFileWriter.h"


template <class TInputImage>
void mitk::NrrdDiffusionVolumesWriter<TInputImage>
  ::Update(void)
{

  char keybuffer[512];
  char valbuffer[512];
  std::string tmp;

  sprintf( valbuffer, "(%1f,%1f,%1f) (%1f,%1f,%1f) (%1f,%1f,%1f)", 1.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,1.0f);
  itk::EncapsulateMetaData<std::string>(m_Input->GetMetaDataDictionary(),std::string("measurement frame"),std::string(valbuffer));

  sprintf( valbuffer, "DWMRI");
  itk::EncapsulateMetaData<std::string>(m_Input->GetMetaDataDictionary(),std::string("modality"),std::string(valbuffer));

  if(m_Directions->Size())
  {
    sprintf( valbuffer, "%1f", m_B_Value );
    itk::EncapsulateMetaData<std::string>(m_Input->GetMetaDataDictionary(),std::string("DWMRI_b-value"),std::string(valbuffer));
  }

  for(int i=0; i<m_Directions->Size(); i++)
  {
    sprintf( keybuffer, "DWMRI_gradient_%04d", i );

    /*if(itk::ExposeMetaData<std::string>(m_Input->GetMetaDataDictionary(),
      std::string(keybuffer),tmp))
      continue;*/

    sprintf( valbuffer, "%1f %1f %1f", m_Directions->ElementAt(i).get(0),
      m_Directions->ElementAt(i).get(1), m_Directions->ElementAt(i).get(2));

    itk::EncapsulateMetaData<std::string>(m_Input->GetMetaDataDictionary(),std::string(keybuffer),std::string(valbuffer));
  }

  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  //io->SetNrrdVectorType( nrrdKindList );
  io->SetFileType( itk::ImageIOBase::Binary );

  typedef itk::ImageFileWriter<TInputImage> WriterType;
  typename WriterType::Pointer nrrdWriter = WriterType::New();
  nrrdWriter->UseInputMetaDataDictionaryOn();
  nrrdWriter->SetInput( m_Input );
  nrrdWriter->SetImageIO(io);
  nrrdWriter->SetFileName(m_FileName);

  try
  {
    nrrdWriter->Update();
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e << std::endl;
  }

}

#endif
