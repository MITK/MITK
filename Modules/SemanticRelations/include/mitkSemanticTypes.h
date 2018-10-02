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

#ifndef MITKSEMANTICTYPES_H
#define MITKSEMANTICTYPES_H

// c++
#include <set>
#include <tuple>

namespace mitk
{
  namespace SemanticTypes
  {
    using ID = std::string;
    using CaseID = std::string; // an ID of the current case (e.g. the DICOM PatientID)
    using InformationType = std::string;

    /*
    * @brief The concept of a control point.
    */
    struct ControlPoint
    {
      ID UID;
      int year = 0;
      int month = 0;
      int day = 0;

      bool operator<(const ControlPoint& other) const
      {
        return std::tie(year, month, day) < std::tie(other.year, other.month, other.day);
      }
      bool operator>(const ControlPoint& other) const
      {
        return std::tie(year, month, day) > std::tie(other.year, other.month, other.day);
      }
      bool operator==(const ControlPoint& other) const
      {
        return (!operator<(other) && !operator>(other));
      }
      bool operator!=(const ControlPoint& other) const
      {
        return (operator<(other) || operator>(other));
      }
      bool operator<=(const ControlPoint& other) const
      {
        return (operator<(other) || operator==(other));
      }
      bool operator>=(const ControlPoint& other) const
      {
        return (operator>(other) || operator==(other));
      }
    };

    /*
    * @brief The concept of a lesion class.
    */
    struct LesionClass
    {
      ID UID;
      std::string classType = "";
    };

    /*
    * @brief The concept of a lesion.
    */
    struct Lesion
    {
      ID UID;
      std::string name = "";
      LesionClass lesionClass;
    };

  } // namespace SemanticTypes
} // namespace mitk

#endif // MITKSEMANTICTYPES_H
