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

#include "mitkFiberBundleXSerializer.h"
#include "mitkFiberBundleX.h"
#include "mitkFiberBundleXWriter.h"

#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>


MITK_REGISTER_SERIALIZER(FiberBundleXSerializer)


mitk::FiberBundleXSerializer::FiberBundleXSerializer()
{
}


mitk::FiberBundleXSerializer::~FiberBundleXSerializer()
{
}


std::string mitk::FiberBundleXSerializer::Serialize()
{
  const FiberBundleX* fb = dynamic_cast<const FiberBundleX*>( m_Data.GetPointer() );
  if (fb == NULL)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::FiberBundleX. Cannot serialize as FiberBundleX.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".fib";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    mitk::IOUtil::Save(const_cast<FiberBundleX*>(fb),fullname);
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

