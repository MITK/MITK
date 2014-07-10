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
#include "mitkPropertyListSerializer.h"

// itk
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"

#include <iostream>
#include <fstream>

#include "tinyxml.h"


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

  sprintf( keybuffer, "modality");
  sprintf( valbuffer, "LSET");
  itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string(keybuffer), std::string(valbuffer));

  sprintf( keybuffer, "layers");
  sprintf( valbuffer, "%1d", input->GetNumberOfLayers());
  itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string(keybuffer), std::string(valbuffer));

  for (int layerIdx=0; layerIdx<input->GetNumberOfLayers(); layerIdx++)
  {
    sprintf( keybuffer, "layer_%03d", layerIdx ); // layer idx
    sprintf( valbuffer, "%1d", input->GetTotalNumberOfLabels()); // number of labels for the layer
    itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string(keybuffer), std::string(valbuffer));

    for (int labelIdx=0; labelIdx<input->GetNumberOfLabels(layerIdx); labelIdx++)
    {
      TiXmlDocument labelAsXml = input->GetLabel(labelIdx,layerIdx)->GetAsTiXmlDocument();
      TiXmlPrinter printer;
      printer.SetIndent("");
      printer.SetLineBreak("");

      labelAsXml.Accept(&printer);

      sprintf( keybuffer, "label_%03d_%03d", layerIdx, labelIdx );
      itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string(keybuffer), printer.Str());
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
