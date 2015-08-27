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