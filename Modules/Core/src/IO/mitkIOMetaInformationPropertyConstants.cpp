/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIOMetaInformationPropertyConstants.h"

namespace mitk
{

  PropertyKeyPath IOMetaInformationPropertyConstants::READER_DESCRIPTION()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "description" });
  }

  PropertyKeyPath IOMetaInformationPropertyConstants::READER_VERSION()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "version" });
  }

  PropertyKeyPath IOMetaInformationPropertyConstants::READER_MIME_NAME()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "mime", "name" });
  }

  PropertyKeyPath IOMetaInformationPropertyConstants::READER_MIME_CATEGORY()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "mime", "category" });
  }

  PropertyKeyPath IOMetaInformationPropertyConstants::READER_INPUTLOCATION()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "inputlocation" });
  }

  PropertyKeyPath IOMetaInformationPropertyConstants::READER_OPTION_ROOT()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "option" });
  }

  PropertyKeyPath IOMetaInformationPropertyConstants::READER_OPTIONS_ANY()
  {
    return READER_OPTION_ROOT().AddAnyElement();
  }
}
