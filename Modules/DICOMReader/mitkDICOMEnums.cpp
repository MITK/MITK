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
