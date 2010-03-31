/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkImageSerializer.h"

#include "mitkImageWriter.h"
#include <Poco/Path.h>

MITK_REGISTER_SERIALIZER(ImageSerializer)

mitk::ImageSerializer::ImageSerializer()
{
}

mitk::ImageSerializer::~ImageSerializer()
{
}

std::string mitk::ImageSerializer::Serialize()
{
  const Image* image = dynamic_cast<const Image*>( m_Data.GetPointer() );
  if (!image)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::Image. Cannot serialize as image.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
std::cout << "creating file " << filename << " in " << m_WorkingDirectory << std::endl;
  filename += "_";
  filename += m_FilenameHint;

  std::string fullname(m_WorkingDirectory);
  fullname += Poco::Path::separator();
  fullname += filename;

  try
  {
    ImageWriter::Pointer writer = ImageWriter::New();
    writer->SetFileName( fullname );
    writer->SetExtension(".pic");
    writer->SetInput( const_cast<Image*>(image) ); // bad writer design??
    writer->Write();
    fullname = writer->GetFileName();
  }
  catch (std::exception& e)
  {
    MITK_ERROR << " Error serializing object at " << (const void*) this->m_Data
              << " to " 
              << fullname 
              << ": " 
              << e.what();
    return "";
  }
  return Poco::Path(fullname).getFileName();// + ".pic";
}
