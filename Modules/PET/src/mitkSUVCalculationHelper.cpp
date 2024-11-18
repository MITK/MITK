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

#include "mitkSUVCalculationHelper.h"

#include <algorithm>

#include <dcmtk/dcmdata/dcvrdt.h>

// Mitk
#include <mitkBaseData.h>
#include <mitkDICOMTagPath.h>
#include <mitkDICOMProperty.h>
#include <mitkLogMacros.h>

#define BOOST_DATE_TIME_NO_LIB
//Prevent unnecessary/unwanted auto link in this compilation when activating boost libraries in the MITK superbuild
//It is necessary because BOOST_ALL_DYN_LINK overwrites BOOST_DATE_TIME_NO_LIB
#if defined(BOOST_ALL_DYN_LINK)
#undef BOOST_ALL_DYN_LINK
#endif

#include <boost/date_time/posix_time/posix_time_types.hpp>


std::vector<double> mitk::GetRadionuclideHalfLife(mitk::BaseData* data)
{
  std::vector<double> result;

  DICOMTagPath dcmPath;
  dcmPath.AddAnySelection(0x0054, 0x0016).AddElement(0x0018, 0x1075);

  if (data)
  {
    std::map< std::string, BaseProperty::Pointer> props = GetPropertyByDICOMTagPath(data, dcmPath);

    for (const auto& finding : props)
    {
      double value = ConvertDICOMStrToValue<double>(finding.second->GetValueAsString());
      result.push_back(value);
    }
  }

  return result;
};

std::string mitk::GetRadionuclideNames(mitk::BaseData* data)
{
  std::string result;

  DICOMTagPath dcmPath;
  dcmPath.AddAnySelection(0x0054, 0x0016).AddAnySelection(0x0054, 0x0300).AddElement(0x0008,0x0104);

  if (data)
  {
    std::map< std::string, BaseProperty::Pointer> props = GetPropertyByDICOMTagPath(data, dcmPath);

    for (const auto& finding : props)
    {
      std::string value = finding.second->GetValueAsString();
      value.erase(std::remove(value.begin(), value.end(), '^'), value.end());
      result += value + " ";
    }
  }

  return result;
};

std::vector<double> mitk::GetRadionuclideTotalDose(mitk::BaseData* data)
{
  std::vector<double> result;

  DICOMTagPath dcmPath;
  dcmPath.AddAnySelection(0x0054, 0x0016).AddElement(0x0018, 0x1074);

  if (data)
  {
    std::map< std::string, BaseProperty::Pointer> props = GetPropertyByDICOMTagPath(data, dcmPath);

    for (const auto& finding : props)
    {
      double value = ConvertDICOMStrToValue<double>(finding.second->GetValueAsString());
      result.push_back(value);
    }
  }

  return result;
};

double mitk::GetPatientsWeight(mitk::BaseData* data)
{
  if (!data) mitkThrow() << "Cannot retrieve patient weight from basd data. Passed instane pointer is NULL.";
  
  DICOMTagPath weightPath(0x0010, 0x1030);
  std::map< std::string, BaseProperty::Pointer> props = GetPropertyByDICOMTagPath(data, weightPath);

  if (props.empty()) mitkThrow() << "Cannot retrieve patient weight from basd data. No patient weight DICOM property found.";
  if (props.size()>1) MITK_WARN << "Base data seems to have inconsistant patient weight DICOM property. More then on proerty found. Use the first dicom path:" << props.begin()->first;

  double result = ConvertDICOMStrToValue<double>(props.begin()->second->GetValueAsString());

  return result;
};

bool ConvertDICOMDateTimeString(const std::string& dateString,
  const std::string& timeString,
  OFDateTime& time)
{
  OFString content(timeString.c_str());

  if (!dateString.empty())
  {
    content = OFString(dateString.c_str()).append(content);
  }

  const OFCondition result = DcmDateTime::getOFDateTimeFromString(content, time);

  return result.good();
}

boost::posix_time::ptime ConvertOFDateTimeToPTime(const OFDateTime& time)
{
  const boost::gregorian::date boostDate(
    time.getDate().getYear(), time.getDate().getMonth(), time.getDate().getDay());

  const boost::posix_time::time_duration boostTime =
    boost::posix_time::hours(time.getTime().getHour())
    + boost::posix_time::minutes(time.getTime().getMinute())
    + boost::posix_time::seconds(time.getTime().getIntSecond())
    + boost::posix_time::milliseconds(time.getTime().getMilliSecond());

  boost::posix_time::ptime result(boostDate, boostTime);

  return result;
}

double ComputeMiliSecDuration(const OFDateTime& start, const OFDateTime& stop)
{
  const boost::posix_time::ptime startTime = ConvertOFDateTimeToPTime(start);
  const boost::posix_time::ptime stopTime = ConvertOFDateTimeToPTime(stop);

  ::boost::posix_time::time_duration duration = stopTime - startTime;

  double result = duration.total_milliseconds();

  return result;
}

mitk::DecayTimeMapType mitk::DeduceDecayTime_AcquisitionMinusStartSliceResolved(mitk::BaseData* data)
{

  DecayTimeMapType result;
  
  DICOMTagPath acqDatePath(0x0008, 0x0022);
  DICOMTagPath acqTimePath(0x0008, 0x0032);
  DICOMTagPath startDateTimePath;
  startDateTimePath.AddAnySelection(0x0054, 0x0016).AddElement(0x0018, 0x1078);
  DICOMTagPath startTimePath;
  startTimePath.AddAnySelection(0x0054, 0x0016).AddElement(0x0018, 0x1072);

  if (data)
  {
    std::map< std::string, BaseProperty::Pointer> acqDateProps = GetPropertyByDICOMTagPath(data, acqDatePath);
    std::map< std::string, BaseProperty::Pointer> acqTimeProps = GetPropertyByDICOMTagPath(data, acqTimePath);
    std::map< std::string, BaseProperty::Pointer> startDateTimeProps = GetPropertyByDICOMTagPath(data, startDateTimePath);
    std::map< std::string, BaseProperty::Pointer> startTimeProps = GetPropertyByDICOMTagPath(data, startTimePath);

    if (acqDateProps.empty()) mitkThrow() << "Cannot retrieve acquisition date from base data. No DICOM property found.";
    if (acqTimeProps.empty()) mitkThrow() << "Cannot retrieve acquisition time from base data. No DICOM property found.";
    if (acqDateProps.size()>1) MITK_WARN << "Base data seems to have inconsistant Acquisition Date DICOM property. More then on proerty found. Use the first dicom path:" << acqDateProps.begin()->first;
    if (acqTimeProps.size()>1) MITK_WARN << "Base data seems to have inconsistant Acquisition Time DICOM property. More then on proerty found. Use the first dicom path:" << acqTimeProps.begin()->first;

    if (startDateTimeProps.empty() && startTimeProps.empty()) mitkThrow() << "Cannot retrieve Radiopharmaceutical Start Time or Radiopharmaceutical Start DateTime from base data. No DICOM property found.";

    const mitk::DICOMProperty* acqTimeProp = dynamic_cast<const mitk::DICOMProperty*>(acqTimeProps.begin()->second.GetPointer());
    const mitk::DICOMProperty* acqDateProp = dynamic_cast<const mitk::DICOMProperty*>(acqDateProps.begin()->second.GetPointer());
    const mitk::DICOMProperty* startDateTimeProp = nullptr;
    const mitk::DICOMProperty* startTimeProp = nullptr;

    if (!startDateTimeProps.empty())
    {
      startDateTimeProp = dynamic_cast<const mitk::DICOMProperty*>(startDateTimeProps.begin()->second.GetPointer());
    }
    if (!startTimeProps.empty())
    {
      startTimeProp = dynamic_cast<const mitk::DICOMProperty*>(startTimeProps.begin()->second.GetPointer());
    }

    auto timeVector = acqDateProp->GetAvailableTimeSteps();
    for (auto timestep : timeVector)
    {
      auto sliceVector = acqDateProp->GetAvailableSlices(timestep);

      for (auto sliceIndex : sliceVector)
      {
        OFDateTime acqTime;
        const bool convertResult = ConvertDICOMDateTimeString(acqDateProp->GetValue(timestep, sliceIndex, true, true), acqTimeProp->GetValue(timestep, sliceIndex, true, true), acqTime);

        OFDateTime startTime;
        if (startDateTimeProp)
        {
          mitkThrow() << "Missing implementation to handle Radiopharmaceutical Start Date Time.";
        }
        else
        {
          const bool convertResult = ConvertDICOMDateTimeString(acqDateProp->GetValue(timestep, sliceIndex, true, true), startTimeProp->GetValue(timestep, sliceIndex, true, true), startTime);
        }


        result[timestep][sliceIndex] = ComputeMiliSecDuration(startTime, acqTime)/1000.0;
      }
    }
  }

  return result;

};
