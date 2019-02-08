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

#define BOOST_DATE_TIME_NO_LIB
#if defined(BOOST_ALL_DYN_LINK)
#undef BOOST_ALL_DYN_LINK
#endif

// boost
#include <boost/date_time/gregorian/gregorian.hpp>

// c++
#include <set>
#include <tuple>
#include <vector>

namespace mitk
{
  namespace SemanticTypes
  {
    using ID = std::string;
    using CaseID = std::string;
    using InformationType = std::string;

    /*
    * @brief The concept of a control point.
    */
    struct ControlPoint
    {
      ID UID;
      boost::gregorian::date date;

      ControlPoint()
      {
        date = boost::gregorian::date(boost::gregorian::min_date_time);
      }

      // less comparison to sort containers of control points
      bool operator<(const ControlPoint& other) const
      {
        return date < other.date;
      }

      std::string ToString() const
      {
        std::stringstream controlPointAsString;
        if (date.is_not_a_date())
        {
          return "";
        }

        controlPointAsString << std::to_string(date.year()) << "-"
          << std::setfill('0') << std::setw(2) << std::to_string(date.month()) << "-"
          << std::setfill('0') << std::setw(2) << std::to_string(date.day());

        return controlPointAsString.str();
      }

      void SetDateFromString(const std::string& dateAsString)
      {
        date = boost::gregorian::from_undelimited_string(dateAsString);
      }

      int DistanceInDays(const ControlPoint& other) const
      {
        boost::gregorian::date_duration duration = date - other.date;
        return std::abs(duration.days());
      }
    };

    /**
    * @brief The concept of an examination period.
    *     An examination period holds a vector of control point UIDs.
    *     The semantic relation storage stores the UIDs such that
    *     the represented control points are in chronological order.
    */
    struct ExaminationPeriod
    {
      ID UID;
      std::string name = "";
      std::vector<ID> controlPointUIDs;
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

    using IDVector = std::vector<ID>;
    using LesionVector = std::vector<Lesion>;
    using LesionClassVector = std::vector<LesionClass>;
    using ControlPointVector = std::vector<ControlPoint>;
    using ExaminationPeriodVector = std::vector<ExaminationPeriod>;
    using InformationTypeVector = std::vector<InformationType>;

  } // namespace SemanticTypes
} // namespace mitk

#endif // MITKSEMANTICTYPES_H
