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

#include "mitkTensorImageSerializer.h"
#include "mitkTensorImage.h"
#include "mitkNrrdTensorImageWriter.h"

#include <itksys/SystemTools.hxx>


MITK_REGISTER_SERIALIZER(TensorImageSerializer)


mitk::TensorImageSerializer::TensorImageSerializer()
{
}


mitk::TensorImageSerializer::~TensorImageSerializer()
{
}


std::string mitk::TensorImageSerializer::Serialize()
{
  const TensorImage* image = dynamic_cast<const TensorImage*>( m_Data.GetPointer() );
  if (image == NULL)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::NrrdTensorImage. Cannot serialize as NrrdTensorImage.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".dti";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    NrrdTensorImageWriter writer;
    writer.SetOutputLocation(fullname);
    writer.SetInput(const_cast<TensorImage*>(image));
    writer.Write();
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

