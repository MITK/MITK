/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-12-10 18:05:13 +0100 (Mi, 10 Dez 2008) $
Version:   $Revision: 15922 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef __mitkNrrdDiffusionVolumesWriter__cpp
#define __mitkNrrdDiffusionVolumesWriter__cpp

#include "mitkNrrdDiffusionVolumesWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkImageFileWriter.h"

template<typename TPixelType>
mitk::NrrdDiffusionVolumesWriter<TPixelType>::NrrdDiffusionVolumesWriter()
    : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
    this->SetNumberOfRequiredInputs( 1 );
}

template<typename TPixelType>
mitk::NrrdDiffusionVolumesWriter<TPixelType>::~NrrdDiffusionVolumesWriter()
{}

template<typename TPixelType>
void mitk::NrrdDiffusionVolumesWriter<TPixelType>::GenerateData()
{
    m_Success = false;
    InputType* input = this->GetInput();
    if (input == NULL)
    {
        itkWarningMacro(<<"Sorry, input to NrrdDiffusionVolumesWriter is NULL!");    
        return;
    }
    if ( m_FileName == "" )
    {
        itkWarningMacro( << "Sorry, filename has not been set!" );
        return ;
    }
    
    char keybuffer[512];
    char valbuffer[512];
    std::string tmp;

    itk::VectorImage<short,3>::Pointer img = input->GetImage();
    img->GetMetaDataDictionary();

    //itk::MetaDataDictionary dic = input->GetImage()->GetMetaDataDictionary();
    sprintf( valbuffer, "(%1f,%1f,%1f) (%1f,%1f,%1f) (%1f,%1f,%1f)", 1.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,1.0f);
    itk::EncapsulateMetaData<std::string>(input->GetImage()->GetMetaDataDictionary(),std::string("measurement frame"),std::string(valbuffer));

    sprintf( valbuffer, "DWMRI");
    itk::EncapsulateMetaData<std::string>(input->GetImage()->GetMetaDataDictionary(),std::string("modality"),std::string(valbuffer));

    if(input->GetDirections()->Size())
    {
      sprintf( valbuffer, "%1f", input->GetB_Value() );
      itk::EncapsulateMetaData<std::string>(input->GetImage()->GetMetaDataDictionary(),std::string("DWMRI_b-value"),std::string(valbuffer));
    }

    for(int i=0; i<input->GetDirections()->Size(); i++)
    {
      sprintf( keybuffer, "DWMRI_gradient_%04d", i );

      /*if(itk::ExposeMetaData<std::string>(input->GetMetaDataDictionary(),
      std::string(keybuffer),tmp))
      continue;*/

      sprintf( valbuffer, "%1f %1f %1f", input->GetDirections()->ElementAt(i).get(0),
        input->GetDirections()->ElementAt(i).get(1), input->GetDirections()->ElementAt(i).get(2));

      itk::EncapsulateMetaData<std::string>(input->GetImage()->GetMetaDataDictionary(),std::string(keybuffer),std::string(valbuffer));
    }

    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    //io->SetNrrdVectorType( nrrdKindList );
    io->SetFileType( itk::ImageIOBase::Binary );

    typedef itk::VectorImage<TPixelType,3> ImageType;
    typedef itk::ImageFileWriter<ImageType> WriterType;
    typename WriterType::Pointer nrrdWriter = WriterType::New();
    nrrdWriter->UseInputMetaDataDictionaryOn();
    nrrdWriter->SetInput( input->GetImage() );
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

    m_Success = true;
}

template<typename TPixelType>
void mitk::NrrdDiffusionVolumesWriter<TPixelType>::SetInput( InputType* diffVolumes )
{
    this->ProcessObject::SetNthInput( 0, diffVolumes );
}

template<typename TPixelType>
mitk::DiffusionVolumes<TPixelType>* mitk::NrrdDiffusionVolumesWriter<TPixelType>::GetInput()
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

template<typename TPixelType>
std::vector<std::string> mitk::NrrdDiffusionVolumesWriter<TPixelType>::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".dwi");
  possibleFileExtensions.push_back(".hdwi");
  return possibleFileExtensions;
}

#endif //__mitkNrrdDiffusionVolumesWriter__cpp