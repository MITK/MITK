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

#include <mitkSceneFileReader.h>

#include <mitkIOMimeTypes.h>
#include <mitkCustomMimeType.h>

mitk::SceneFileReader::SceneFileReader(const mitk::SceneFileReader& other)
  : mitk::AbstractFileReader(other)
{
}

mitk::SceneFileReader::SceneFileReader()
  : AbstractFileReader()
{
  //this->SetMimeTypePrefix(IOMimeTypes::DEFAULT_BASE_NAME() + ".legacy.");

  std::string category = "MITK Scene Files";
  CustomMimeType customMimeType;
  customMimeType.SetCategory(category);
  customMimeType.AddExtension("mitk");

  this->SetDescription(category);
  this->SetMimeType(customMimeType);

  m_ServiceReg = this->RegisterService();
}

mitk::SceneFileReader::~SceneFileReader()
{
}

/**
* This method must be implemented for each specific reader. Call
* GetInputStream() first and check for a non-null stream to read from.
* If the input stream is \c NULL, use GetInputLocation() to read from a local
* file-system path.
*/
std::vector<itk::SmartPointer<mitk::BaseData> > mitk::SceneFileReader::Read()
{
  MITK_INFO("SceneFileReader") << "SceneFileReader Triggered";
  std::vector<itk::SmartPointer<mitk::BaseData> > vector;
  return vector;
}

mitk::SceneFileReader* mitk::SceneFileReader::Clone() const
{
  return new SceneFileReader(*this);
}