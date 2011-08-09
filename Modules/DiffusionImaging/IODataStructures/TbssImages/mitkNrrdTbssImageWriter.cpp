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

#ifndef __mitkNrrdTbssImageWriter__cpp
#define __mitkNrrdTbssImageWriter__cpp

#include "mitkNrrdTbssImageWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
//#include "itkNiftiImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"

#include <iostream>
#include <fstream>

template<typename TPixelType>
mitk::NrrdTbssImageWriter<TPixelType>::NrrdTbssImageWriter()
  : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredInputs( 1 );
}

template<typename TPixelType>
mitk::NrrdTbssImageWriter<TPixelType>::~NrrdTbssImageWriter()
{}

template<typename TPixelType>
void mitk::NrrdTbssImageWriter<TPixelType>::GenerateData()
{
  m_Success = false;
  InputType* input = this->GetInput();
  if (input == NULL)
  {
    itkWarningMacro(<<"Sorry, input to NrrdTbssImageWriter is NULL!");
    return;
  }
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }



  itk::Image<char,3>::Pointer img = input->GetImage();


  char keybuffer[512];
  char valbuffer[512];

  if(input->GetTbssType() == mitk::TbssImage<char>::ROI)
  {

    itk::EncapsulateMetaData<std::string>(input->GetImage()->GetMetaDataDictionary(), "tbss", "ROI");


    mitk::TbssImage<char>::RoiType roi = input->GetRoi();

    mitk::TbssImage<char>::RoiType::iterator it = roi.begin();

    int i=0;
    while(it != roi.end())
    {
      itk::Index<3> ix = *it;

      sprintf( keybuffer, "ROI_index_%04d", i );
      sprintf( valbuffer, "%1d %1d %1d", ix[0],ix[1],ix[2]);

      std::cout << valbuffer << std::endl;

      itk::EncapsulateMetaData< std::string >(input->GetImage()->GetMetaDataDictionary(),std::string(keybuffer),std::string(valbuffer));
      it++;
      ++i;
    }



  }



  typedef itk::Image<TPixelType,3> ImageType;


  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  io->SetFileType( itk::ImageIOBase::Binary );
  io->UseCompressionOn();


  typedef itk::ImageFileWriter<ImageType> WriterType;
  typename WriterType::Pointer nrrdWriter = WriterType::New();
  nrrdWriter->UseInputMetaDataDictionaryOn();
  nrrdWriter->SetInput( img );
  nrrdWriter->SetImageIO(io);
  nrrdWriter->SetFileName(m_FileName);
 // nrrdWriter->UseCompressionOn();
  nrrdWriter->SetImageIO(io);
  try
  {
    nrrdWriter->Update();
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e << std::endl;
  }



  /*


  //itk::MetaDataDictionary dic = input->GetImage()->GetMetaDataDictionary();

  vnl_matrix_fixed<double,3,3> measurementFrame = input->GetMeasurementFrame();
  if (measurementFrame(0,0) || measurementFrame(0,1) || measurementFrame(0,2) ||
      measurementFrame(1,0) || measurementFrame(1,1) || measurementFrame(1,2) ||
      measurementFrame(2,0) || measurementFrame(2,1) || measurementFrame(2,2))
  {
    sprintf( valbuffer, " ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) \n", measurementFrame(0,0), measurementFrame(0,1), measurementFrame(0,2), measurementFrame(1,0), measurementFrame(1,1), measurementFrame(1,2), measurementFrame(2,0), measurementFrame(2,1), measurementFrame(2,2));
    itk::EncapsulateMetaData<std::string>(input->GetVectorImage()->GetMetaDataDictionary(),std::string("measurement frame"),std::string(valbuffer));
  }

  sprintf( valbuffer, "DWMRI");
  itk::EncapsulateMetaData<std::string>(input->GetVectorImage()->GetMetaDataDictionary(),std::string("modality"),std::string(valbuffer));

  if(input->GetOriginalDirections()->Size())
  {
    sprintf( valbuffer, "%1f", input->GetB_Value() );
    itk::EncapsulateMetaData<std::string>(input->GetVectorImage()->GetMetaDataDictionary(),std::string("DWMRI_b-value"),std::string(valbuffer));
  }

  for(unsigned int i=0; i<input->GetOriginalDirections()->Size(); i++)
  {
    sprintf( keybuffer, "DWMRI_gradient_%04d", i );

    /*if(itk::ExposeMetaData<std::string>(input->GetMetaDataDictionary(),
      std::string(keybuffer),tmp))
      continue;*/

   // sprintf( valbuffer, "%1f %1f %1f", input->GetOriginalDirections()->ElementAt(i).get(0),
//             input->GetOriginalDirections()->ElementAt(i).get(1), input->GetOriginalDirections()->ElementAt(i).get(2));

  //  itk::EncapsulateMetaData<std::string>(input->GetVectorImage()->GetMetaDataDictionary(),std::string(keybuffer),std::string(valbuffer));
 // }





  m_Success = true;
}

template<typename TPixelType>
void mitk::NrrdTbssImageWriter<TPixelType>::SetInput( InputType* tbssVol )
{
  this->ProcessObject::SetNthInput( 0, tbssVol );
}

template<typename TPixelType>
mitk::TbssImage<TPixelType>* mitk::NrrdTbssImageWriter<TPixelType>::GetInput()
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
std::vector<std::string> mitk::NrrdTbssImageWriter<TPixelType>::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".tbss");
  return possibleFileExtensions;
}

#endif //__mitkNrrdTbssImageWriter__cpp
