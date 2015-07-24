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

#ifndef MITKEXAMPLEIOMIMETYPES_H
#define MITKEXAMPLEIOMIMETYPES_H

#include "mitkCustomMimeType.h"

#include <string>

namespace mitk {

class ExampleIOMimeTypes
{
public:

  // Deriving your own MimeType will probably be overkill in most situations.
  class ExampleDataStructureMimeType : public CustomMimeType
  {
  public:
    ExampleDataStructureMimeType();
    virtual bool AppliesTo(const std::string &path) const override;
    virtual ExampleDataStructureMimeType* Clone() const override;
  };

  static ExampleDataStructureMimeType EXAMPLE_MIMETYPE();
  static std::string EXAMPLE_MIMETYPE_NAME();

  // Simpler method of creating a new MimeType
  static CustomMimeType EXAMPLE_TWO_MIMETYPE();
  static std::string EXAMPLE_TWO_MIMETYPE_NAME();

  // Get all example Mime Types
  static std::vector<CustomMimeType*> Get();

private:

  // purposely not implemented
  ExampleIOMimeTypes();
  ExampleIOMimeTypes(const ExampleIOMimeTypes&);
};

}

#endif // MITKEXAMPLEIOMIMETYPES_H
