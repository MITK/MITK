/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkProportionalTimeGeometryToXML.h"

#include "mitkGeometry3DToXML.h"

#include <mitkLexicalCast.h>

#include <tinyxml2.h>

tinyxml2::XMLElement *mitk::ProportionalTimeGeometryToXML::ToXML(tinyxml2::XMLDocument& doc, const ProportionalTimeGeometry *timeGeom)
{
  assert(timeGeom);

  auto *timeGeomElem = doc.NewElement("ProportionalTimeGeometry");
  timeGeomElem->SetAttribute("NumberOfTimeSteps", static_cast<int>(timeGeom->CountTimeSteps()));
  // TinyXML cannot serialize infinity (default value for time step)
  // So we guard this value and the first time point against serialization problems
  // by not writing them. The reader can then tell that absence of those values
  // means "keep the default values"
  if (timeGeom->GetFirstTimePoint() != -std::numeric_limits<TimePointType>::max())
    timeGeomElem->SetAttribute("FirstTimePoint", boost::lexical_cast<std::string>(timeGeom->GetFirstTimePoint()).c_str());
  if (timeGeom->GetStepDuration() != std::numeric_limits<TimePointType>::infinity())
    timeGeomElem->SetAttribute("StepDuration", boost::lexical_cast<std::string>(timeGeom->GetStepDuration()).c_str());

  for (TimeStepType t = 0; t < timeGeom->CountTimeSteps(); ++t)
  {
    // add a node for the geometry of each time step
    const Geometry3D *geom3D(nullptr);
    if ((geom3D = dynamic_cast<const Geometry3D *>(timeGeom->GetGeometryForTimeStep(t).GetPointer())))
    {
      auto *geom3DElement = Geometry3DToXML::ToXML(doc, geom3D);
      geom3DElement->SetAttribute("TimeStep", static_cast<int>(t)); // mark order for us
      timeGeomElem->InsertEndChild(geom3DElement);
    }
    else
    {
      MITK_WARN << "Serializing a ProportionalTimeGeometry that contains something other than Geometry3D!"
                << " (in time step " << t << ")"
                << " File will miss information!";
    }
  }

  return timeGeomElem;
}

mitk::ProportionalTimeGeometry::Pointer mitk::ProportionalTimeGeometryToXML::FromXML(const tinyxml2::XMLElement *timeGeometryElement)
{
  if (!timeGeometryElement)
  {
    MITK_ERROR << "Cannot deserialize ProportionalTimeGeometry from nullptr.";
    return nullptr;
  }

  int numberOfTimeSteps = 0;

  if (tinyxml2::XML_SUCCESS != timeGeometryElement->QueryIntAttribute("NumberOfTimeSteps", &numberOfTimeSteps))
  {
    MITK_WARN << "<ProportionalTimeGeometry> found without NumberOfTimeSteps attribute. Counting...";
  }

  // might be missing!
  TimePointType firstTimePoint;
  const char* firstTimePoint_s = nullptr;
  TimePointType stepDuration;
  const char* stepDuration_s = nullptr;
  try
  {
    firstTimePoint_s = timeGeometryElement->Attribute("FirstTimePoint");
    if (nullptr != firstTimePoint_s)
    {
      firstTimePoint = boost::lexical_cast<double>(firstTimePoint_s);
    }
    else
    {
      firstTimePoint = -std::numeric_limits<TimePointType>::max();
    }

    stepDuration_s = timeGeometryElement->Attribute("StepDuration");
    if (nullptr != stepDuration_s)
    {
      stepDuration = boost::lexical_cast<double>(stepDuration_s);
    }
    else
    {
      stepDuration = std::numeric_limits<TimePointType>::infinity();
    }
  }
  catch ( const boost::bad_lexical_cast &e )
  {
    MITK_ERROR << "Could not parse string as number: " << e.what();
    return nullptr;
  }

  // list of all geometries with their time steps
  std::multimap<TimeStepType, BaseGeometry::Pointer> allReadGeometries;

  int indexForUnlabeledTimeStep(-1);
  for (auto *currentElement = timeGeometryElement->FirstChildElement(); currentElement != nullptr;
       currentElement = currentElement->NextSiblingElement())
  {
    // different geometries could have been inside a ProportionalTimeGeometry.
    // By now, we only support Geometry3D
    std::string tagName = currentElement->Value();
    if (tagName == "Geometry3D")
    {
      Geometry3D::Pointer restoredGeometry = Geometry3DToXML::FromXML(currentElement);
      if (restoredGeometry.IsNotNull())
      {
        int timeStep(-1);
        if (tinyxml2::XML_SUCCESS != currentElement->QueryIntAttribute("TimeStep", &timeStep))
        {
          timeStep = indexForUnlabeledTimeStep--; // decrement index for next one
          MITK_WARN << "Found <Geometry3D> without 'TimeStep' attribute in <ProportionalTimeGeometry>. No guarantees "
                       "on order anymore.";
        }

        if (allReadGeometries.count(static_cast<TimeStepType>(timeStep)) > 0)
        {
          MITK_WARN << "Found <Geometry3D> tags with identical 'TimeStep' attribute in <ProportionalTimeGeometry>. No "
                       "guarantees on order anymore.";
        }

        allReadGeometries.insert(std::make_pair(static_cast<TimeStepType>(timeStep), restoredGeometry.GetPointer()));
      }
    }
    else
    {
      MITK_WARN << "Found unsupported tag <" << tagName << "> inside <ProportionalTimeGeometry>. Ignoring.";
    }
  }

  // now add all BaseGeometries that were read to a new instance
  // of ProportionalTimeGeometry
  ProportionalTimeGeometry::Pointer newTimeGeometry = ProportionalTimeGeometry::New();
  newTimeGeometry->SetFirstTimePoint(firstTimePoint);
  newTimeGeometry->SetStepDuration(stepDuration);
  newTimeGeometry->ReserveSpaceForGeometries(allReadGeometries.size());

  TimeStepType t(0);
  for (const auto &entry : allReadGeometries)
  {
    // We add items with newly assigned time steps.
    // This avoids great confusion when a file contains
    // bogus numbers.
    newTimeGeometry->SetTimeStepGeometry(entry.second, t++);
  }

  // Need to re-calculate global bounding box.
  // This is neither stored in a file, nor done by SetTimeStepGeometry
  newTimeGeometry->UpdateBoundingBox();

  return newTimeGeometry;
}
