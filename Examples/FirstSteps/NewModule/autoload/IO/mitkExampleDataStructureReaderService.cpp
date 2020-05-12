/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExampleDataStructureReaderService.h"

// mitk includes
#include "mitkExampleIOMimeTypes.h"
#include "mitkGeometry3D.h"
#include <mitkCustomMimeType.h>

// itk includes
#include "itksys/SystemTools.hxx"

namespace mitk
{
  ExampleDataStructureReaderService::ExampleDataStructureReaderService(const ExampleDataStructureReaderService &other)
    : mitk::AbstractFileReader(other)
  {
  }

  ExampleDataStructureReaderService::ExampleDataStructureReaderService()
    : mitk::AbstractFileReader(CustomMimeType(mitk::ExampleIOMimeTypes::EXAMPLE_MIMETYPE()),
                               "Default reader for the example data structure")
  {
    m_ServiceReg = this->RegisterService();
  }

  ExampleDataStructureReaderService::~ExampleDataStructureReaderService() {}
  std::vector<itk::SmartPointer<BaseData>> ExampleDataStructureReaderService::DoRead()
  {
    std::vector<itk::SmartPointer<mitk::BaseData>> result;
    std::string location = GetInputLocation();

    std::string ext = itksys::SystemTools::GetFilenameLastExtension(location);
    ext = itksys::SystemTools::LowerCase(ext);

    if (location == "")
    {
      MITK_ERROR << "No file name specified.";
    }
    try
    {
      std::ifstream file(location);
      std::string content("");
      std::string line("");
      if (file.is_open())
      {
        while (getline(file, line))
        {
          content += line;
          content += "\n";
        }
      }
      else
      {
        mitkThrow() << "Could not open file " << this->GetInputLocation() << " for reading.";
      }

      mitk::ExampleDataStructure::Pointer outputData = mitk::ExampleDataStructure::New();
      outputData->SetData(content);
      result.push_back(outputData.GetPointer());
      MITK_INFO << "Example file read";
    }

    catch (const mitk::Exception& e)
    {
      MITK_ERROR << e.GetDescription();
    }
    catch (...)
    {
      MITK_ERROR << "Unknown error occurred while trying to read file.";
    }

    return result;
  }

} // namespace MITK

mitk::ExampleDataStructureReaderService *mitk::ExampleDataStructureReaderService::Clone() const
{
  return new ExampleDataStructureReaderService(*this);
}
