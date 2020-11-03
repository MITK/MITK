/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMEnums.h"

std::string
mitk::PixelSpacingInterpretationToString(const PixelSpacingInterpretation& value)
{
  switch (value)
  {
    case SpacingInPatient:  return "In Patient";
    case SpacingAtDetector: return "At Detector";
    case SpacingUnknown:    return "Unknown spacing";
    default: return "<unknown value of enum PixelSpacingInterpretation>";
  };
}

std::string
mitk::ReaderImplementationLevelToString( const ReaderImplementationLevel& value )
{
  switch (value)
  {
    case SOPClassSupported:       return "SOPClassSupported";
    case SOPClassPartlySupported: return "SOPClassPartlySupported";
    case SOPClassImplemented:     return "SOPClassImplemented";
    case SOPClassUnsupported:     return "SOPClassUnsupported";
    case SOPClassUnknown:         return "SOPClassUnknown";
    default: return "<unknown value of enum ReaderImplementationLevel>";
  };
}
