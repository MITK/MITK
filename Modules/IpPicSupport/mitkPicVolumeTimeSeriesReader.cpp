/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include "mitkPicVolumeTimeSeriesReader.h"
#include "mitkPicFileReader.h"
#include <itkImageFileReader.h>
#include <string>

extern "C" 
{
mitkIpPicDescriptor * MITKipPicGet( char *infile_name, mitkIpPicDescriptor *pic );
mitkIpPicDescriptor * MITKipPicGetTags( char *infile_name, mitkIpPicDescriptor *pic );
}


void mitk::PicVolumeTimeSeriesReader::GenerateOutputInformation()
{
    mitk::Image::Pointer output = this->GetOutput();

    if ( ( output->IsInitialized() ) && ( this->GetMTime() <= m_ReadHeaderTime.GetMTime() ) )
        return ;

    itkDebugMacro( << "Reading file for GenerateOutputInformation()" << m_FileName );

    if ( ! this->GenerateFileList() )
    {
        itkWarningMacro( "Sorry, file list could not be generated!" );
        return ;
    }

    //
    // Read the first file of the image sequence. Assume equal size and spacings for all
    // other volumes. @TODO Integrate support for different sizes and spacings
    //
    char* filename = const_cast<char *> ( m_MatchedFileNames[ 0 ].c_str() );
    mitkIpPicDescriptor * header = mitkIpPicGetHeader( filename, NULL );
    header = MITKipPicGetTags( filename, header );

    if ( header == NULL )
    {
        itk::ImageFileReaderException e( __FILE__, __LINE__ );
        itk::OStringStream msg;
        msg << " Could not read file " << m_FileName.c_str();
        e.SetDescription( msg.str().c_str() );
        throw e;
        return ;
    }
    if ( header->dim != 3 )
    {
        itk::ImageFileReaderException e( __FILE__, __LINE__ , "Only 3D-pic volumes are supported! " );
        throw e;
        return ;
    }

    //
    // modify the header to match the real temporal extent
    //
    header->dim = 4;
    header->n[ 3 ] = m_MatchedFileNames.size();

    output->Initialize( header );

    mitkIpPicFree( header );

    m_ReadHeaderTime.Modified();
}


void mitk::PicVolumeTimeSeriesReader::GenerateData()
{
    mitk::Image::Pointer output = this->GetOutput();

    //
    // Check to see if we can read the file given the name or prefix
    //
    if ( m_FilePrefix == "" || m_FilePattern == "" )
    {
        throw itk::ImageFileReaderException( __FILE__, __LINE__, "Both FilePattern and FilePrefix must be non-empty" );
    }

    if ( m_MatchedFileNames.size() == 0 )
    {
        throw itk::ImageFileReaderException( __FILE__, __LINE__, "Sorry, there are no files to read!" );
    }


    //
    // read 3d volumes and copy them to the 4d volume
    //
    mitkIpPicDescriptor* volume3d = NULL;
    for ( unsigned int t = 0 ; t < m_MatchedFileNames.size() ; ++t )
    {
        char* filename = const_cast< char* >( m_MatchedFileNames[ t ].c_str() );
        MITK_INFO << "Reading file " << filename << "..." << std::endl;
        volume3d = MITKipPicGet( filename, NULL );

        if ( volume3d == NULL )
        {
          ::itk::OStringStream message;
          message << "mitk::ERROR: " << this->GetNameOfClass() << "(" << this << "): "
                  << "File (" << filename << ") of time frame " << t << " could not be read!";
          throw itk::ImageFileReaderException( __FILE__, __LINE__, message.str().c_str() );
        }

        //
        // @TODO do some error checking
        //

        //
        // copy the 3d data volume to the 4d volume
        //

        // \todo use memory of Image as in PicFileReader (or integrate everything into the PicFileReader!)
        PicFileReader::ConvertHandedness(volume3d);
        bool result;
        result = output->SetPicVolume( volume3d, t );
        if(result==false)
        {
          ::itk::OStringStream message;
          message << "mitk::ERROR: " << this->GetNameOfClass() << "(" << this << "): "
                  << "Volume of time frame " << t << " did not match size of other time frames.";
          throw itk::ImageFileReaderException( __FILE__, __LINE__, message.str().c_str() );
        }
        mitkIpPicFree ( volume3d );
    }
}

bool mitk::PicVolumeTimeSeriesReader::CanReadFile(const std::string /*filename*/, const std::string filePrefix, const std::string filePattern) 
{
  if( filePattern == "" && filePrefix == "" )
    return false;

  bool extensionFound = false;
  std::string::size_type PICPos = filePattern.rfind(".pic");
  if ((PICPos != std::string::npos)
      && (PICPos == filePattern.length() - 4))
    {
    extensionFound = true;
    }

  PICPos = filePattern.rfind(".pic.gz");
  if ((PICPos != std::string::npos)
      && (PICPos == filePattern.length() - 7))
    {
    extensionFound = true;
    }

  if( !extensionFound )
    return false;

  return true;
}

mitk::PicVolumeTimeSeriesReader::PicVolumeTimeSeriesReader()
{
    //this->DebugOn();
}

mitk::PicVolumeTimeSeriesReader::~PicVolumeTimeSeriesReader()
{}

