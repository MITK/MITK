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

#include "mitkPlanarFigureSerializer.h"
#include "mitkPlanarFigure.h"
#include "mitkPlanarFigureWriter.h"

#include <Poco/Path.h>

MITK_REGISTER_SERIALIZER(PlanarFigureSerializer)


mitk::PlanarFigureSerializer::PlanarFigureSerializer()
{
}


mitk::PlanarFigureSerializer::~PlanarFigureSerializer()
{
}


std::string mitk::PlanarFigureSerializer::Serialize()
{
  LOG_INFO << this->GetNameOfClass() 
           << " is asked to serialize an object " << (const void*) this->m_Data
           << " into a directory " << m_WorkingDirectory
           << " using a filename hint " << m_FilenameHint;

  const PlanarFigure* pf = dynamic_cast<const PlanarFigure*>( m_Data.GetPointer() );
  if (pf == NULL)
  {
    LOG_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::PlanarFigure. Cannot serialize as PlanarFigure.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  LOG_INFO << "creating file " << filename << " in " << m_WorkingDirectory << std::endl;
  filename += "_";
  filename += m_FilenameHint;
  filename += ".pf";

  std::string fullname(m_WorkingDirectory);
  fullname += Poco::Path::separator();
  fullname += filename;

  try
  {
    PlanarFigureWriter::Pointer writer = PlanarFigureWriter::New();
    writer->SetFileName(fullname);
    writer->SetInput(const_cast<PlanarFigure*>(pf));
    writer->Write();
  }
  catch (std::exception& e)
  {
    LOG_ERROR << " Error serializing object at " << (const void*) this->m_Data
              << " to " 
              << fullname 
              << ": " 
              << e.what();
    return "";
  }
  return filename;
}
