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

#include "mitkSurfaceSerializer.h"

#include "mitkSurfaceVtkWriter.h"

#include <vtkXMLPolyDataWriter.h>
#include "Poco/Path.h"

MITK_REGISTER_SERIALIZER(SurfaceSerializer)

mitk::SurfaceSerializer::SurfaceSerializer()
{
}

mitk::SurfaceSerializer::~SurfaceSerializer()
{
}

std::string mitk::SurfaceSerializer::Serialize()
{
  const Surface* surface = dynamic_cast<const Surface*>( m_Data.GetPointer() );
  if (!surface)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::Surface. Cannot serialize as surface.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".vtp";

  std::string fullname(m_WorkingDirectory);
  fullname += Poco::Path::separator();
  fullname += filename;

  try
  {
    SurfaceVtkWriter<vtkXMLPolyDataWriter>::Pointer writer = SurfaceVtkWriter<vtkXMLPolyDataWriter>::New();
    writer->SetFileName( fullname );
    //writer->SetExtension(".vtp");
    writer->SetInput( const_cast<Surface*>( surface ) );
    writer->Write();
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

  return filename;
}

