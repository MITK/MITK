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

#ifndef mitkDICOMEnums_h
#define mitkDICOMEnums_h

#include <string>
#include <vector>
#include <utility>

namespace mitk
{
  typedef std::vector<std::string> StringList;
  typedef std::vector<bool> BoolList;

  typedef enum
  {
    SpacingInPatient,  /// distances are mm within a patient
    SpacingAtDetector, /// distances are mm at detector surface
    SpacingUnknown     /// NO spacing information is present, we use (1,1) as default
  } PixelSpacingInterpretation;
}

#endif
