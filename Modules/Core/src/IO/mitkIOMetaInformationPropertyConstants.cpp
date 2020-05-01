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

  PropertyKeyPath IOMetaInformationPropertyConsants::READER_DESCRIPTION()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "description" });
  }

  PropertyKeyPath IOMetaInformationPropertyConsants::READER_VERSION()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "version" });
  }

  PropertyKeyPath IOMetaInformationPropertyConsants::READER_MIME_NAME()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "mime", "name" });
  }

  PropertyKeyPath IOMetaInformationPropertyConsants::READER_MIME_CATEGORY()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "mime", "category" });
  }

  PropertyKeyPath IOMetaInformationPropertyConsants::READER_INPUTLOCATION()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "inputlocation" });
  }

  PropertyKeyPath IOMetaInformationPropertyConsants::READER_OPTION_ROOT()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "option" });
  }

  PropertyKeyPath IOMetaInformationPropertyConsants::READER_OPTIONS_ANY()
  {
    return READER_OPTION_ROOT().AddAnyElement();
  }
}
