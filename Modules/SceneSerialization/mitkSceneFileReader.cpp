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
#include <mitkSceneIO.h>
#include <mitkDataStorage.h>

#include <itkVectorContainer.h>

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
  std::istream* stream = this->GetInputStream();
  if (stream == NULL) MITK_INFO("SceneFileReader") << "Stream Found";
  std::string location = GetInputLocation();
  MITK_INFO("SceneFileReader") << "File Location is " << location;
  std::vector<itk::SmartPointer<mitk::BaseData> > result;

  mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();
  mitk::DataStorage::Pointer storage = sceneIO->LoadScene(location);

  typedef  itk::VectorContainer<unsigned int, mitk::DataNode::Pointer>  VectorContainerType;

  VectorContainerType::ConstPointer nodes = storage->GetAll();
  VectorContainerType::ConstIterator it = nodes->Begin();
  it = nodes->Begin();
  while(it != nodes->End())
    {
      mitk::DataNode::Pointer n = it->Value();
      result.push_back(n->GetData());
      ++it;
    }

  return result;
}

mitk::SceneFileReader* mitk::SceneFileReader::Clone() const
{
  return new SceneFileReader(*this);
}