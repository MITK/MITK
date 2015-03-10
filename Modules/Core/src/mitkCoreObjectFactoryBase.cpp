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

#include "mitkCoreObjectFactoryBase.h"

void mitk::CoreObjectFactoryBase::CreateFileExtensions(MultimapType fileExtensionsMap, std::string& fileExtensions)
{
  std::map<std::string, std::string> aMap;

  // group the extensions by extension-group
  // e.g. aMap["DICOM files"] = "*.dcm *.DCM *.dc3 *.DC3 *.gdcm"
  for (MultimapType::iterator it = fileExtensionsMap.begin(); it != fileExtensionsMap.end(); ++it)
  {
    std::string aValue = aMap[(*it).second];
    if (aValue.compare("") != 0)
    {
      aValue.append(" ");
    }
    aValue.append((*it).first);
    aMap[(*it).second] = aValue;
  }

  // build the "all" entry (it contains all the extensions)
  // and add it to the string in the first position
  // e.g. "all (*.dcm *.DCM *.dc3 *.DC3 *.gdcm *.ima *.mhd ... *.vti *.hdr *.nrrd *.nhdr );;"
  fileExtensions = "known extensions (";
  std::string lastKey = "";
  for (MultimapType::iterator it = fileExtensionsMap.begin(); it != fileExtensionsMap.end(); ++it)
  {
    std::string aKey = (*it).first;

    if (aKey.compare(lastKey) != 0)
    {
      if (lastKey.compare("") != 0)
      {
        fileExtensions.append(" ");
      }
      fileExtensions.append(aKey);
    }
    lastKey = aKey;
  }
  fileExtensions.append(");;all (*);;");

  // build the entry for each extension-group
  // e.g. "Sets of 2D slices (*.pic *.pic.gz *.bmp *.png *.dcm *.gdcm *.ima *.tiff);;"
  for (std::map<std::string, std::string>::iterator it = aMap.begin(); it != aMap.end(); ++it)
  {
    //cout << "  [" << (*it).first << ", " << (*it).second << "]" << endl;
    std::string aKey = (*it).first;
    if (aKey.compare("") != 0)
    {
      fileExtensions.append((*it).first);
      fileExtensions.append(" (");
      fileExtensions.append((*it).second);
      fileExtensions.append(");;");
    }
  }
}

