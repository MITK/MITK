/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExampleDataStructureWriterService.h"

// itk includes
#include "itksys/SystemTools.hxx"

// mitk includes
#include "mitkExampleIOMimeTypes.h"

mitk::ExampleDataStructureWriterService::ExampleDataStructureWriterService()
  : AbstractFileWriter(mitk::ExampleDataStructure::GetStaticNameOfClass(),
                       CustomMimeType(mitk::ExampleIOMimeTypes::EXAMPLE_MIMETYPE()),
                       "Default writer for the example data structure")
{
  RegisterService();
}

mitk::ExampleDataStructureWriterService::ExampleDataStructureWriterService(
  const mitk::ExampleDataStructureWriterService &other)
  : AbstractFileWriter(other)
{
}

mitk::ExampleDataStructureWriterService::~ExampleDataStructureWriterService()
{
}

mitk::ExampleDataStructureWriterService *mitk::ExampleDataStructureWriterService::Clone() const
{
  return new ExampleDataStructureWriterService(*this);
}

void mitk::ExampleDataStructureWriterService::Write()
{
  MITK_INFO << "Writing ExampleDataStructure";
  InputType::ConstPointer input = dynamic_cast<const InputType *>(this->GetInput());
  if (input.IsNull())
  {
    MITK_ERROR << "Sorry, input to ExampleDataStructureWriterService is nullptr!";
    return;
  }
  if (this->GetOutputLocation().empty())
  {
    MITK_ERROR << "Sorry, filename has not been set!";
    return;
  }

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(this->GetOutputLocation());
  ext = itksys::SystemTools::LowerCase(ext);

  // default extension is .txt
  if (ext == "")
  {
    ext = ".txt";
    this->SetOutputLocation(this->GetOutputLocation() + ext);
  }

  try
  {
    std::ofstream file(this->GetOutputLocation());

    if (file.is_open())
    {
      file << input->GetData();
    }
    else
    {
      mitkThrow() << "Could not open file " << this->GetOutputLocation() << " for writing.";
    }

    MITK_INFO << "Example Data Structure has been written";
  }

  catch (const mitk::Exception& e)
  {
    MITK_ERROR << e.GetDescription();
  }
  catch (...)
  {
    MITK_ERROR << "Unknown error occurred while trying to write file.";
  }
}
