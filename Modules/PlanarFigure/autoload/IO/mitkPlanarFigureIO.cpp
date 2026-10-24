/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigureIO.h"
#include "mitkPlanarFigureIOMimeTypes.h"

#include <mitkAffineTransform3D.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarEllipse.h>
#include <mitkPlaneGeometry.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <iomanip>

namespace
{
  constexpr int Version = 1;

  template <class TPair>
  TPair ReadPair(const nlohmann::json &j, const std::string &what)
  {
    if (!j.is_array() || 2 != j.size())
      mitkThrow() << what << " needs two coordinates.";

    TPair pair;
    pair[0] = j[0].get<mitk::ScalarType>();
    pair[1] = j[1].get<mitk::ScalarType>();
    return pair;
  }

  nlohmann::ordered_json WritePlane(const mitk::PlaneGeometry *plane)
  {
    nlohmann::json transform;
    mitk::ToJSON(transform, plane->GetIndexToWorldTransform());

    const auto &bounds = plane->GetBounds();

    nlohmann::ordered_json j;
    j["Transform"] = transform;
    j["Bounds"] = { bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5] };
    j["ImageGeometry"] = plane->GetImageGeometry();

    return j;
  }

  mitk::PlaneGeometry::Pointer ReadPlane(const nlohmann::json &j)
  {
    auto transform = mitk::AffineTransform3D::New();
    mitk::FromJSON(j.at("Transform"), transform);

    const auto &jBounds = j.at("Bounds");

    if (!jBounds.is_array() || 6 != jBounds.size())
      mitkThrow() << "The bounds of the plane need six values.";

    mitk::BaseGeometry::BoundsArrayType bounds;

    for (unsigned int i = 0; i < 6; ++i)
      bounds[i] = jBounds[i].get<mitk::ScalarType>();

    // PlaneGeometry asserts most of these instead of throwing.
    if (0.0 != bounds[0] || 0.0 != bounds[2] || !(bounds[1] > 0.0) || !(bounds[3] > 0.0) || !(bounds[5] > bounds[4]))
      mitkThrow() << "The bounds of the plane must start at 0 and have an extent.";

    auto plane = mitk::PlaneGeometry::New();
    plane->SetIndexToWorldTransform(transform);
    plane->SetBounds(bounds);
    plane->SetImageGeometry(j.at("ImageGeometry").get<bool>());

    return plane;
  }

  nlohmann::ordered_json WriteFigure(const mitk::PlanarFigure &figure)
  {
    nlohmann::ordered_json j = {
      { "FileFormat", mitk::PlanarFigureIOMimeTypes::FileFormat },
      { "Version", Version },
      { "Type", figure.GetTypeId() }
    };

    j["Plane"] = WritePlane(figure.GetPlaneGeometry());

    auto controlPoints = nlohmann::json::array();

    for (const auto &point : figure.GetControlPoints())
      controlPoints.push_back({ point[0], point[1] });

    j["ControlPoints"] = controlPoints;

    nlohmann::ordered_json parameters;

    if (const auto *doubleEllipse = dynamic_cast<const mitk::PlanarDoubleEllipse *>(&figure); nullptr != doubleEllipse)
    {
      parameters["Thickness"] = doubleEllipse->GetThickness();
    }
    else if (const auto *ellipse = dynamic_cast<const mitk::PlanarEllipse *>(&figure); nullptr != ellipse)
    {
      if (const auto direction = ellipse->GetReferenceDirection(); direction.has_value())
        parameters["ReferenceDirection"] = { (*direction)[0], (*direction)[1] };
    }

    if (!parameters.empty())
      j["Parameters"] = parameters;

    if (figure.IsShapeLocked())
      j["ShapeLocked"] = true;

    return j;
  }

  void ReadParameters(const nlohmann::json &j, mitk::PlanarFigure &figure)
  {
    if (!j.is_object())
      mitkThrow() << "The parameters need to be an object.";

    for (const auto &parameter : j.items())
    {
      const auto &name = parameter.key();
      auto *doubleEllipse = dynamic_cast<mitk::PlanarDoubleEllipse *>(&figure);
      auto *ellipse = dynamic_cast<mitk::PlanarEllipse *>(&figure);

      if ("Thickness" == name && nullptr != doubleEllipse)
      {
        doubleEllipse->SetThickness(parameter.value().get<double>());
      }
      else if ("ReferenceDirection" == name && nullptr != ellipse)
      {
        ellipse->SetReferenceDirection(ReadPair<mitk::Vector2D>(parameter.value(), "The reference direction"));
      }
      else
      {
        mitkThrow() << "A planar figure of type \"" << figure.GetTypeId() << "\" has no parameter \"" << name << "\".";
      }
    }
  }

  mitk::PlanarFigure::Pointer ReadFigure(const nlohmann::json &j)
  {
    if (!j.is_object() || mitk::PlanarFigureIOMimeTypes::FileFormat != j.value("FileFormat", ""))
      mitkThrow() << "Not a planar figure file.";

    if (!j.at("Version").is_number_integer())
      mitkThrow() << "The version of the planar figure file must be an integer.";

    const auto version = j.at("Version").get<int>();

    if (version > Version)
      mitkThrow() << "The planar figure file has version " << version << ", which is newer than the supported version " << Version << ".";

    if (version < 1)
      mitkThrow() << "The planar figure file has the invalid version " << version << ".";

    auto figure = mitk::CreatePlanarFigure(j.at("Type").get<std::string>());
    figure->SetPlaneGeometry(ReadPlane(j.at("Plane")));

    const auto &jControlPoints = j.at("ControlPoints");

    if (!jControlPoints.is_array())
      mitkThrow() << "The control points need to be an array.";

    std::vector<mitk::Point2D> controlPoints;

    for (const auto &jControlPoint : jControlPoints)
      controlPoints.push_back(ReadPair<mitk::Point2D>(jControlPoint, "A control point"));

    figure->SetControlPoints(controlPoints);

    if (j.contains("Parameters"))
      ReadParameters(j["Parameters"], *figure);

    if (j.contains("ShapeLocked"))
      figure->SetShapeLocked(j["ShapeLocked"].get<bool>());

    return figure;
  }
}

mitk::PlanarFigureIO::PlanarFigureIO()
  : AbstractFileIO(PlanarFigure::GetStaticNameOfClass(), PlanarFigureIOMimeTypes::PLANARFIGURE_MIMETYPE(), "MITK Planar Figure")
{
  this->RegisterService();
}

mitk::IFileIO::ConfidenceLevel mitk::PlanarFigureIO::GetWriterConfidenceLevel() const
{
  if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
    return Unsupported;

  return nullptr != dynamic_cast<const PlanarFigure *>(this->GetInput())
    ? Supported
    : Unsupported;
}

std::vector<mitk::BaseData::Pointer> mitk::PlanarFigureIO::DoRead()
{
  auto *stream = this->GetInputStream();
  std::ifstream fileStream;

  if (nullptr == stream)
  {
    const auto filename = this->GetInputLocation();
    fileStream.open(filename);

    if (!fileStream.is_open())
      mitkThrow() << "Could not open file \"" << filename << "\" for reading.";

    stream = &fileStream;
  }

  PlanarFigure::Pointer figure;

  try
  {
    figure = ReadFigure(nlohmann::json::parse(*stream));
  }
  catch (const nlohmann::json::exception &e)
  {
    mitkThrow() << "Invalid planar figure file: " << e.what();
  }

  return { figure.GetPointer() };
}

void mitk::PlanarFigureIO::Write()
{
  const auto *figure = dynamic_cast<const PlanarFigure *>(this->GetInput());

  if (nullptr == figure)
    mitkThrow() << "Invalid input for writing.";

  if (!figure->IsComplete())
    mitkThrow() << "A planar figure cannot be written while it is being placed.";

  auto *stream = this->GetOutputStream();
  std::ofstream fileStream;

  if (nullptr == stream)
  {
    const auto filename = this->GetOutputLocation();

    if (filename.empty())
      mitkThrow() << "Neither an output stream nor an output filename was specified.";

    fileStream.open(filename);

    if (!fileStream.is_open())
      mitkThrow() << "Could not open file \"" << filename << "\" for writing.";

    stream = &fileStream;
  }

  *stream << std::setw(2) << WriteFigure(*figure) << std::endl;
}

mitk::PlanarFigureIO *mitk::PlanarFigureIO::IOClone() const
{
  return new PlanarFigureIO(*this);
}
