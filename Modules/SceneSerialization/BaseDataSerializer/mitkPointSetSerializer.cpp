/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#include "mitkPointSetSerializer.h"
#include "mitkPointSetWriter.h"
#include "Poco/Path.h"


MITK_REGISTER_SERIALIZER(PointSetSerializer)

mitk::PointSetSerializer::PointSetSerializer()
{
}

mitk::PointSetSerializer::~PointSetSerializer()
{
}

std::string mitk::PointSetSerializer::Serialize()
{
  const PointSet* ps = dynamic_cast<const PointSet *>( m_Data.GetPointer() );
  if (ps == NULL)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::PointSet. Cannot serialize as pointset.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".mps";

  std::string fullname(m_WorkingDirectory);
  fullname += Poco::Path::separator();
  fullname += filename;

  try
  {
    PointSetWriter::Pointer writer = PointSetWriter::New();
    writer->SetFileName( fullname );
    writer->SetInput( const_cast<PointSet*>( ps ) );
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
