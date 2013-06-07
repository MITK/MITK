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


#include <mitkLegacyImageWriterService.h>
#include <usGetModuleContext.h>
#include <mitkIOUtil.h>
#include <mitkDataNode.h>
#include <mitkCoreObjectFactory.h>
#include <mitkImageWriter.h>

//#include <mitkBaseData.h>


mitk::LegacyImageWriterService::LegacyImageWriterService(std::string basedataType, std::string extension, std::string description)
{
  if (extension == "") mitkThrow() << "LegacyFileWriterWrapper cannot be initialized without FileExtension." ;
  m_BasedataType = basedataType;
  m_Extension = extension;
  m_Description = description;
  m_Priority = 0; // Default priority for legacy Writer
  RegisterMicroservice(mitk::GetModuleContext());
}

mitk::LegacyImageWriterService::~LegacyImageWriterService()
{

}

////////////////////// Writing /////////////////////////

void mitk::LegacyImageWriterService::Write(const itk::SmartPointer<BaseData> data, const std::string& path)
{
  mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();

  std::string name = path;
  name.erase(path.find_last_of('.'), path.size());

  imageWriter->SetFileName(name.c_str());
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(data);
  imageWriter->SetInput(node);
  imageWriter->SetExtension(m_Extension); // Call structure guarantees that m_Extension is user selected Extension
  imageWriter->Update();
}
