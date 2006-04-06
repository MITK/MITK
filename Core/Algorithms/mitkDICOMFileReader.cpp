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
#include "mitkPicFileReader.h"

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
      image, image_size, color, ipTrue);


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
    Vector3D spacing;
    spacing.Fill(1.0);
      
    ipPicTSV_t *tsv;
    tsv = ipPicQueryTag( pic, "SOURCE HEADER" );
    bool ok=false;

    if (tsv)
    {
      void *data;
      ipUInt4_t len;
      ipFloat8_t xSpacing = 1;
      ipFloat8_t ySpacing = 1;
      ipFloat8_t zSpacing = 1;

      //0x0018 0x602c
      if( dicomFindElement( (unsigned char *) tsv->value, 0x0018, 0x602c, &data, &len ) )
      {
        ok=true;
        sscanf( (char *) data, "%lf", &xSpacing );
      }
      
      if( dicomFindElement( (unsigned char *) tsv->value, 0x0018, 0x602e, &data, &len ) )
      {
        ok=true;
        sscanf( (char *) data, "%lf", &ySpacing );
      }
      
      if( dicomFindElement( (unsigned char *) tsv->value, 0x3001, 0x1003, &data, &len ) )
      {
        ok=true;
        sscanf( (char *) data, "%lf", &zSpacing );
      }
     
      FillVector3D(spacing, xSpacing, ySpacing, zSpacing);
      itkGenericOutputMacro( "spacing:  " << spacing << " mm");
      
    }


    output->Initialize(pic);
    if (ok)
      output->GetSlicedGeometry()->SetSpacing( spacing );
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
    PicFileReader::ConvertHandedness(pic);
    output->SetPicChannel(pic);
  }
}

bool mitk::DICOMFileReader::CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern) 
{
  if(  filename == "" )
    return false;

  bool extensionFound = false;
  std::string::size_type DICOMPos = filename.rfind(".ipdcm");
  if ((DICOMPos != std::string::npos) && (DICOMPos == filename.length() - 6))
    extensionFound = true;

  DICOMPos = filename.rfind(".IPDCM");
  if ((DICOMPos != std::string::npos) && (DICOMPos == filename.length() - 6))
    extensionFound = true;

  if( !extensionFound )
    return false;

  return true;
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
