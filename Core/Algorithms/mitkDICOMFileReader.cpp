/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkDICOMFileReader.h"
#include <itkImageFileReader.h>

extern "C"
{
#include "ipDicom/ipDicom.h"
}

void mitk::DICOMFileReader::GenerateOutputInformation()
{
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized()) && (this->GetMTime() <= m_ReadHeaderTime.GetMTime()))
    return;

  itkDebugMacro(<<"Reading file for GenerateOutputInformation()" << m_FileName);

  // Check to see if we can read the file given the name or prefix
  //
  if ( m_FileName == "" )
  {
    throw itk::ImageFileReaderException(__FILE__, __LINE__, "One of FileName or FilePrefix must be non-empty");
  }

  if( m_FileName != "")
  {
    ipUInt4_t header_size;
    ipUInt4_t image_size;
    ipUInt1_t *header = NULL;
    unsigned char *image = NULL;
    ipBool_t color=ipFalse;
    outputfile_name = m_FileName;
    dicomGetHeaderAndImage( const_cast<char *>(m_FileName.c_str()),
      &header, &header_size,
      &image, &image_size );

    if( !header )
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Could not get header.");;
    if( !image )
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Could not get image.");;
    pic = _dicomToPic( header, header_size,
      image, image_size, color );
    pic->dim--;

    if( pic == NULL)
    {
      itk::ImageFileReaderException e(__FILE__, __LINE__);
      itk::OStringStream msg;
      msg << " Could not read file "
        << outputfile_name.c_str();
      e.SetDescription(msg.str().c_str());
      throw e;
      return;
    }

    output->Initialize(pic);
  }

  m_ReadHeaderTime.Modified();
}

//##ModelId=3E187255016C
void mitk::DICOMFileReader::GenerateData()
{
  mitk::Image::Pointer output = this->GetOutput();

  // Check to see if we can read the file given the name or prefix
  //
  if ( m_FileName == "" && m_FilePrefix == "" )
  {
    throw itk::ImageFileReaderException(__FILE__, __LINE__, "One of FileName or FilePrefix must be non-empty");
  }

  if( m_FileName != "")
  {

    //left to right handed conversion
    if(pic->dim==3)
    {
      ipPicDescriptor* slice=ipPicCopyHeader(pic, NULL);
      slice->dim=2;
      int size=_ipPicSize(slice);
      slice->data=malloc(size);
      unsigned char *p_first=(unsigned char *)pic->data;
      unsigned char *p_last=(unsigned char *)pic->data;
      p_last+=size*(pic->n[2]-1);

      int i, smid=pic->n[2]/2;
      for(i=0; i<smid;++i,p_last-=size, p_first+=size)
      {
        memcpy(slice->data, p_last, size);
        memcpy(p_last, p_first, size);
        memcpy(p_first, slice->data, size);
      }
      ipPicFree(slice);
    }
    output->SetPicVolume(pic);
  }
}

//##ModelId=3E1874D202D0
mitk::DICOMFileReader::DICOMFileReader()
: m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
}

//##ModelId=3E1874E50179
mitk::DICOMFileReader::~DICOMFileReader()
{
}
