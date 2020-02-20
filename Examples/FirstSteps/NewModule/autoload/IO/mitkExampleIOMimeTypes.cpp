/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExampleIOMimeTypes.h"
#include "mitkIOMimeTypes.h"
#include <itksys/SystemTools.hxx>
#include <mitkLogMacros.h>

namespace mitk
{
  std::vector<CustomMimeType *> ExampleIOMimeTypes::Get()
  {
    std::vector<CustomMimeType *> mimeTypes;

    // order matters here (descending rank for mime types)

    mimeTypes.push_back(EXAMPLE_MIMETYPE().Clone());
    mimeTypes.push_back(EXAMPLE_TWO_MIMETYPE().Clone());

    return mimeTypes;
  }

  // Mime Types

  ExampleIOMimeTypes::ExampleDataStructureMimeType::ExampleDataStructureMimeType()
    : CustomMimeType(EXAMPLE_MIMETYPE_NAME())
  {
    std::string category = "Example and Tutorial Data";
    this->SetCategory(category);
    this->SetComment("Example data structure containing just a string");

    this->AddExtension("example");
    this->AddExtension("txt");
  }

  bool ExampleIOMimeTypes::ExampleDataStructureMimeType::AppliesTo(const std::string &path) const
  {
    bool canRead(CustomMimeType::AppliesTo(path));

    // fix for bug 18572
    // Currently this function is called for writing as well as reading, in that case
    // the image information can of course not be read
    // This is a bug, this function should only be called for reading.
    if (!itksys::SystemTools::FileExists(path.c_str()))
    {
      return canRead;
    }
    // end fix for bug 18572

    std::string ext = this->GetExtension(path);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext == ".txt")
    {
      // we could test something here
      if (false)
      {
        MITK_WARN << "Can not read .txt file as ExampleDataStructure";
        canRead = false;
      }
    }

    return canRead;
  }

  ExampleIOMimeTypes::ExampleDataStructureMimeType *ExampleIOMimeTypes::ExampleDataStructureMimeType::Clone() const
  {
    return new ExampleDataStructureMimeType(*this);
  }

  ExampleIOMimeTypes::ExampleDataStructureMimeType ExampleIOMimeTypes::EXAMPLE_MIMETYPE()
  {
    return ExampleDataStructureMimeType();
  }

  CustomMimeType ExampleIOMimeTypes::EXAMPLE_TWO_MIMETYPE()
  {
    CustomMimeType mimeType(EXAMPLE_TWO_MIMETYPE_NAME());
    std::string category = "Example and Tutorial Data";
    mimeType.SetComment("Simpler Example Data MimeType");
    mimeType.SetCategory(category);
    mimeType.AddExtension("example2");
    return mimeType;
  }

  // Names
  std::string ExampleIOMimeTypes::EXAMPLE_MIMETYPE_NAME()
  {
    // create a unique and sensible name for this mime type
    static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".examples.example-data-structure";
    return name;
  }

  std::string ExampleIOMimeTypes::EXAMPLE_TWO_MIMETYPE_NAME()
  {
    // create a unique and sensible name for this mime type
    static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".examples.example-data-structure-the-second";
    return name;
  }
}
