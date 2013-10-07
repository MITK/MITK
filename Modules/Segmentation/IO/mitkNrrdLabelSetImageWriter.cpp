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

#ifndef __mitkNrrdLabelSetImageWriter__cpp
#define __mitkNrrdLabelSetImageWriter__cpp

#include "mitkNrrdLabelSetImageWriter.h"
#include "mitkImageCast.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
//#include "itkNiftiImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"

#include <iostream>
#include <fstream>

mitk::NrrdLabelSetImageWriter::NrrdLabelSetImageWriter()
  : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredInputs( 1 );
}

mitk::NrrdLabelSetImageWriter::~NrrdLabelSetImageWriter()
{}

void mitk::NrrdLabelSetImageWriter::GenerateData()
{
  m_Success = false;
  InputType* input = this->GetInput();
  if (input == NULL)
  {
    MITK_WARN << "Input to NrrdLabelSetImageWriter is NULL.";
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
      MITK_ERROR << "Could not set locale " << currLocale;
      mitkThrow() << "Could not set locale.";
    }
  }

//  typedef itk::Image<TPixelType,3> ImageType;
//  typedef itk::VectorImage<TPixelType,3> VectorImageType;

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
  ext = itksys::SystemTools::LowerCase(ext);
  if (ext == ".lset")
  {
    typedef itk::ImageFileWriter<VectorImageType> WriterType;

    VectorImageType::Pointer vectorImage = input->GetVectorImage(true); // force update

    char keybuffer[512];
    char valbuffer[512];

    sprintf( valbuffer, "LSET");
    itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string("modality"),std::string(valbuffer));

    sprintf( valbuffer, input->GetName().c_str() );
    itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string("name"),std::string(valbuffer));

    sprintf( valbuffer, input->GetLastModificationTime().c_str() );
    itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string("last modification time"),std::string(valbuffer));

    sprintf( valbuffer, "%1d", input->GetTotalNumberOfLabels());
    itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string("number of labels"),std::string(valbuffer));

    int idx = 0;
    for (int layer=0; layer<input->GetNumberOfLayers(); layer++)
    {
      for (int label=0; label<input->GetNumberOfLabels(layer); label++)
      {
        sprintf( keybuffer, "label_%03d_name", idx );
        sprintf( valbuffer, "%s", input->GetLabelName(layer, label).c_str());

        itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string(keybuffer), std::string(valbuffer));

        sprintf( keybuffer, "label_%03d_props", idx );
        float rgba[4];
        const mitk::Color& color = input->GetLabelColor(layer, label);
        rgba[0] = color.GetRed();
        rgba[1] = color.GetGreen();
        rgba[2] = color.GetBlue();
        rgba[3] = input->GetLabelOpacity(layer,label);
        int locked = input->GetLabelLocked(layer,label);
        int visible = input->GetLabelVisible(layer,label);
        sprintf( valbuffer, "%f %f %f %f %d %d %d %d", rgba[0], rgba[1], rgba[2], rgba[3], locked, visible, layer, label);

        itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string(keybuffer), std::string(valbuffer));
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
      MITK_ERROR << "Could not write segmentation. See error log for details.";
      mitkThrow() << e.GetDescription();
    }
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

void mitk::NrrdLabelSetImageWriter::SetInput( InputType* diffVolumes )
{
  this->ProcessObject::SetNthInput( 0, diffVolumes );
}

mitk::LabelSetImage* mitk::NrrdLabelSetImageWriter::GetInput()
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

std::vector<std::string> mitk::NrrdLabelSetImageWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".lset");
  return possibleFileExtensions;
}

#endif //__mitkNrrdLabelSetImageWriter__cpp
