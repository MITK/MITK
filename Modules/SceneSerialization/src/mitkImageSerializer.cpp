/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageSerializer.h"
#include "mitkIOUtil.h"
#include "mitkImage.h"
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
  const auto *image = dynamic_cast<const Image *>(m_Data.GetPointer());
  if (!image)
  {
    MITK_ERROR << " Object at " << (const void *)this->m_Data << " is not an mitk::Image. Cannot serialize as image.";
    return "";
  }

  std::string filename(this->GetUniqueFilenameInWorkingDirectory());
  std::cout << "creating file " << filename << " in " << m_WorkingDirectory << std::endl;
  filename += "_";
  filename += m_FilenameHint;

  std::string fullname(m_WorkingDirectory);
  fullname += Poco::Path::separator();
  fullname += filename + ".nrrd";

  try
  {
    IOUtil::Save(image, fullname);
  }
  catch (std::exception &e)
  {
    MITK_ERROR << " Error serializing object at " << (const void *)this->m_Data << " to " << fullname << ": "
               << e.what();
    return "";
  }
  return Poco::Path(fullname).getFileName();
}
