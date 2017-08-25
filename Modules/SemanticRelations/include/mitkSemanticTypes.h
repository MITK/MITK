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
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>

namespace mitk
{
  namespace SemanticTypes
  {
    typedef std::string ID;
    typedef std::string CaseID; // an ID of the current case (e.g. the DICOM PatientID)
    typedef std::string InformationType;

    /*
    * @brief The date type to be used for control points.
    */
    struct Date
    {
      ID UID;
      int year = 0;
      int month = 0;
      int day = 0;

      bool operator<(const Date& other) const
      {
        return std::tie(year, month, day) < std::tie(other.year, other.month, other.day);
      }
      bool operator>(const Date& other) const
      {
        return std::tie(year, month, day) > std::tie(other.year, other.month, other.day);
      }
      bool operator==(const Date& other) const
      {
        return (!operator<(other) && !operator>(other));
      }
      bool operator!=(const Date& other) const
      {
        return (operator<(other) || operator>(other));
      }
      bool operator<=(const Date& other) const
      {
        return (operator<(other) || operator==(other));
      }
      bool operator>=(const Date& other) const
      {
        return (operator>(other) || operator==(other));
      }
    };

    /*
    * @brief The concept of a control point.
    */
    struct ControlPoint // alternatively: ExaminationPoint
    {
      ID UID;
      Date startPoint;
      Date endPoint;

      bool operator<(const ControlPoint& other) const
      {
        return startPoint < other.startPoint;
      }
      bool operator!=(const ControlPoint& other) const
      {
        return (startPoint != other.startPoint) || (endPoint != other.endPoint);
      }
      bool operator==(const ControlPoint& other) const
      {
        return (startPoint == other.startPoint) && (endPoint == other.endPoint);
      }
    };

    /*
    * @brief The concept of a lesion.
    */
    struct LesionClass
    {
      ID UID;
      std::string classType = ""; // could be a "focal lesion" or "diffuse lesion" in the BlackSwan context
    };

    /*
    * @brief The concept of a lesion.
    */
    struct Lesion
    {
      ID UID;
      LesionClass lesionClass;
    };

  } // namespace SemanticTypes
} // namespace mitk

#endif // MITKSEMANTICTYPES_H
