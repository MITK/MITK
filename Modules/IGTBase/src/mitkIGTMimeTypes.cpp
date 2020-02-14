/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkIGTMimeTypes.h"

mitk::CustomMimeType mitk::IGTMimeTypes::NAVIGATIONDATASETXML_MIMETYPE()
{
  mitk::CustomMimeType mimeType(IOMimeTypes::DEFAULT_BASE_NAME() + ".NavigationDataSet.xml");
  std::string category = "NavigationDataSet";
  mimeType.SetComment("NavigationDataSet (XML)");
  mimeType.SetCategory(category);
  mimeType.AddExtension("xml");
  return mimeType;
}

mitk::CustomMimeType mitk::IGTMimeTypes::NAVIGATIONDATASETCSV_MIMETYPE()
{
  mitk::CustomMimeType mimeType(IOMimeTypes::DEFAULT_BASE_NAME() + ".NavigationDataSet.csv");
  std::string category = "NavigationDataSet";
  mimeType.SetComment("NavigationDataSet (csv)");
  mimeType.SetCategory(category);
  mimeType.AddExtension("csv");
  return mimeType;
}

mitk::CustomMimeType mitk::IGTMimeTypes::USDEVICEINFORMATIONXML_MIMETYPE()
{
  mitk::CustomMimeType mimeType(IOMimeTypes::DEFAULT_BASE_NAME() + ".USDeviceInformation.xml");
  std::string category = "USDeviceInformation";
  mimeType.SetComment("USDeviceInformation (XML)");
  mimeType.SetCategory(category);
  mimeType.AddExtension("xml");
  return mimeType;
}
