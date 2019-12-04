/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKEXAMPLEIOMIMETYPES_H
#define MITKEXAMPLEIOMIMETYPES_H

#include "mitkCustomMimeType.h"

#include <string>

namespace mitk
{
  class ExampleIOMimeTypes
  {
  public:
    // Deriving your own MimeType will probably be overkill in most situations.
    class ExampleDataStructureMimeType : public CustomMimeType
    {
    public:
      ExampleDataStructureMimeType();
      bool AppliesTo(const std::string &path) const override;
      ExampleDataStructureMimeType *Clone() const override;
    };

    static ExampleDataStructureMimeType EXAMPLE_MIMETYPE();
    static std::string EXAMPLE_MIMETYPE_NAME();

    // Simpler method of creating a new MimeType
    static CustomMimeType EXAMPLE_TWO_MIMETYPE();
    static std::string EXAMPLE_TWO_MIMETYPE_NAME();

    // Get all example Mime Types
    static std::vector<CustomMimeType *> Get();

  private:
    // purposely not implemented
    ExampleIOMimeTypes();
    ExampleIOMimeTypes(const ExampleIOMimeTypes &);
  };
}

#endif // MITKEXAMPLEIOMIMETYPES_H
