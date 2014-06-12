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

#ifndef __mitkLabelSetImageWriter__cpp
#define __mitkLabelSetImageWriter__cpp

#include "mitkLabelSetImageWriter.h"

// itk
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"

#include <iostream>
#include <fstream>

mitk::LabelSetImageWriter::LabelSetImageWriter()
  : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredInputs( 1 );
}

mitk::LabelSetImageWriter::~LabelSetImageWriter()
{}

void mitk::LabelSetImageWriter::GenerateData()
{
  m_Success = false;
  InputType* input = this->GetInput();
  if (input == NULL)
  {
    MITK_WARN << "Input to LabelSetImageWriter is NULL.";
    return;
  }
  if ( m_FileName == "" )
  {
    MITK_WARN << "Filename has not been set.";
    return ;
  }
  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, NULL );

  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      mitkThrow() << "Could not set locale " << currLocale;
    }
  }

  typedef itk::ImageFileWriter<VectorImageType> WriterType;

  VectorImageType::Pointer vectorImage = input->GetVectorImage(true); // force update

  char keybuffer[512];
  char valbuffer[512];

  sprintf( valbuffer, "LSET");
  itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string("modality"), std::string(valbuffer));

  sprintf( valbuffer, input->GetName().c_str() );
  itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string("name"), std::string(valbuffer));

  sprintf( valbuffer, input->GetLastModificationTime().c_str() );
  itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string("last modification time"), std::string(valbuffer));

  sprintf( valbuffer, "%1d", input->GetTotalNumberOfLabels());
  itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string("number of labels"), std::string(valbuffer));

  int idx = 0;
  for (int layerIdx=0; layerIdx<input->GetNumberOfLayers(); layerIdx++)
  {
    for (int labelIdx=0; labelIdx<input->GetNumberOfLabels(layerIdx); labelIdx++)
    {
      sprintf( keybuffer, "label_%03d_name", idx );
      sprintf( valbuffer, "%s", input->GetLabel(labelIdx,layerIdx)->GetName().c_str());

      itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string(keybuffer), std::string(valbuffer));

      sprintf( keybuffer, "label_%03d_props", idx );
      float rgba[4];
      const mitk::Color& color = input->GetLabel(labelIdx, layerIdx)->GetColor();
      rgba[0] = color.GetRed();
      rgba[1] = color.GetGreen();
      rgba[2] = color.GetBlue();
      rgba[3] = input->GetLabel(labelIdx, layerIdx)->GetOpacity();
      int locked = input->GetLabel(labelIdx, layerIdx)->GetLocked();
      int visible = input->GetLabel(labelIdx, layerIdx)->GetVisible();
      sprintf(valbuffer, "%f %f %f %f %d %d %d %d", rgba[0], rgba[1], rgba[2], rgba[3], locked, visible, layerIdx, labelIdx);

      itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(), std::string(keybuffer), std::string(valbuffer));
      ++idx;
    }
  }

  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  io->SetFileType( itk::ImageIOBase::Binary );
  io->UseCompressionOn();

  WriterType::Pointer nrrdWriter = WriterType::New();
  nrrdWriter->UseInputMetaDataDictionaryOn();
  nrrdWriter->SetInput(vectorImage);
  nrrdWriter->SetFileName(m_FileName);
  nrrdWriter->UseCompressionOn();
  nrrdWriter->SetImageIO(io);

  try
  {
    nrrdWriter->Update();
  }
  catch (itk::ExceptionObject e)
  {
    MITK_ERROR << "Could not write segmentation session. See error log for details.";
    mitkThrow() << e.GetDescription();
  }

  try
  {
    setlocale(LC_ALL, currLocale.c_str());
  }
  catch(...)
  {
    mitkThrow() << "Could not reset locale " << currLocale;
  }
  m_Success = true;
}

void mitk::LabelSetImageWriter::SetInput( InputType* diffVolumes )
{
  this->ProcessObject::SetNthInput( 0, diffVolumes );
}

mitk::LabelSetImage* mitk::LabelSetImageWriter::GetInput()
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    return NULL;
  }
  else
  {
    return dynamic_cast<InputType*> ( this->ProcessObject::GetInput( 0 ) );
  }
}

std::vector<std::string> mitk::LabelSetImageWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".lset");
  return possibleFileExtensions;
}

#endif //__mitkLabelSetImageWriter__cpp
