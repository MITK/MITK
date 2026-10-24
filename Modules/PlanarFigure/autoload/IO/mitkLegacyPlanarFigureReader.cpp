/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLegacyPlanarFigureReader.h"
#include "mitkPlanarFigureIOMimeTypes.h"

#include <mitkLocaleSwitch.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarEllipse.h>
#include <mitkPlaneGeometry.h>

#include <tinyxml2.h>

#include <algorithm>
#include <map>
#include <optional>

namespace
{
  const std::map<std::string, std::string> TypeIds = {
    { "PlanarAngle", "angle" },
    { "PlanarArrow", "arrow" },
    { "PlanarBezierCurve", "bezier_curve" },
    { "PlanarCircle", "circle" },
    { "PlanarCross", "cross" },
    { "PlanarDoubleEllipse", "double_ellipse" },
    { "PlanarEllipse", "ellipse" },
    { "PlanarFourPointAngle", "four_point_angle" },
    { "PlanarLine", "line" },
    { "PlanarPath", "path" },
    { "PlanarPolygon", "polygon" },
    { "PlanarRectangle", "rectangle" },
    { "PlanarSubdivisionPolygon", "subdivision_polygon" }
  };

  bool GetBoolProperty(const tinyxml2::XMLElement *figureElement, const std::string &key, bool defaultValue)
  {
    for (const auto *property = figureElement->FirstChildElement("property"); nullptr != property;
         property = property->NextSiblingElement("property"))
    {
      const auto *propertyKey = property->Attribute("key");

      if (nullptr == propertyKey || key != propertyKey)
        continue;

      const auto *value = property->FirstChildElement("bool");
      auto result = defaultValue;

      if (nullptr == value || tinyxml2::XML_SUCCESS != value->QueryBoolAttribute("value", &result))
        mitkThrow() << "The property \"" << key << "\" of a planar figure is not a valid boolean.";

      return result;
    }

    return defaultValue;
  }

  std::vector<double> ReadValues(const tinyxml2::XMLElement *element, const std::string &name, const std::vector<std::string> &attributes)
  {
    if (nullptr == element)
      mitkThrow() << "The geometry of a planar figure has no element \"" << name << "\".";

    std::vector<double> values;

    for (const auto &attribute : attributes)
    {
      double value = 0.0;

      if (tinyxml2::XML_SUCCESS != element->QueryDoubleAttribute(attribute.c_str(), &value))
        mitkThrow() << "The element \"" << name << "\" of a planar figure geometry has no valid attribute \"" << attribute << "\".";

      values.push_back(value);
    }

    return values;
  }

  std::vector<std::string> NumberedAttributes(const std::string &base, unsigned int count)
  {
    std::vector<std::string> attributes;

    for (unsigned int i = 0; i < count; ++i)
      attributes.push_back(base + std::to_string(i));

    return attributes;
  }

  mitk::PlaneGeometry::Pointer ReadPlane(const tinyxml2::XMLElement *geometry)
  {
    if (nullptr == geometry)
      mitkThrow() << "A planar figure has no geometry.";

    const auto parameters = ReadValues(geometry->FirstChildElement("transformParam"), "transformParam", NumberedAttributes("param", 12));
    const auto boundsValues = ReadValues(geometry->FirstChildElement("boundsParam"), "boundsParam", NumberedAttributes("bound", 6));
    const auto spacingValues = ReadValues(geometry->FirstChildElement("Spacing"), "Spacing", { "x", "y", "z" });
    const auto originValues = ReadValues(geometry->FirstChildElement("Origin"), "Origin", { "x", "y", "z" });

    mitk::AffineTransform3D::ParametersType transformParameters(12);

    for (unsigned int i = 0; i < 12; ++i)
      transformParameters[i] = parameters[i];

    auto transform = mitk::AffineTransform3D::New();
    transform->SetParameters(transformParameters);

    mitk::BaseGeometry::BoundsArrayType bounds;

    for (unsigned int i = 0; i < 6; ++i)
      bounds[i] = boundsValues[i];

    mitk::Vector3D spacing;
    mitk::Point3D origin;

    for (unsigned int i = 0; i < 3; ++i)
    {
      spacing[i] = spacingValues[i];
      origin[i] = originValues[i];
    }

    // PlaneGeometry asserts most of these instead of throwing.
    if (0.0 != bounds[0] || 0.0 != bounds[2] || !(bounds[1] > 0.0) || !(bounds[3] > 0.0) || !(bounds[5] > bounds[4]))
      mitkThrow() << "The bounds of the plane must start at 0 and have an extent.";

    if (!(spacing[0] > 0.0) || !(spacing[1] > 0.0) || !(spacing[2] > 0.0))
      mitkThrow() << "The spacing of the plane must be positive.";

    // Spacing and origin repeat the transform; they are applied in the order of the former reader.
    auto plane = mitk::PlaneGeometry::New();
    plane->SetIndexToWorldTransform(transform);
    plane->SetBounds(bounds);
    plane->SetSpacing(spacing);
    plane->SetOrigin(origin);

    return plane;
  }

  std::vector<mitk::Point2D> ReadControlPoints(const tinyxml2::XMLElement *controlPoints)
  {
    if (nullptr == controlPoints)
      mitkThrow() << "A planar figure has no control points.";

    std::map<int, mitk::Point2D> vertices;

    for (const auto *vertex = controlPoints->FirstChildElement("Vertex"); nullptr != vertex;
         vertex = vertex->NextSiblingElement("Vertex"))
    {
      int id = 0;
      mitk::Point2D point;

      if (tinyxml2::XML_SUCCESS != vertex->QueryIntAttribute("id", &id) ||
          tinyxml2::XML_SUCCESS != vertex->QueryDoubleAttribute("x", &point[0]) ||
          tinyxml2::XML_SUCCESS != vertex->QueryDoubleAttribute("y", &point[1]))
      {
        mitkThrow() << "A control point of a planar figure has no valid id, x or y.";
      }

      if (!vertices.emplace(id, point).second)
        mitkThrow() << "The control points of a planar figure have the id " << id << " twice.";
    }

    std::vector<mitk::Point2D> points;

    for (const auto &[id, point] : vertices)
    {
      if (id != static_cast<int>(points.size()))
        mitkThrow() << "The control points of a planar figure are not numbered from 0 without gaps.";

      points.push_back(point);
    }

    return points;
  }

  void ClampToPlane(const mitk::PlaneGeometry *plane, mitk::Point2D &point)
  {
    const auto &bounds = plane->GetBounds();

    mitk::Point2D index;
    plane->WorldToIndex(point, index);
    index[0] = std::clamp(index[0], bounds[0], bounds[1]);
    index[1] = std::clamp(index[1], bounds[2], bounds[3]);
    plane->IndexToWorld(index, point);
  }

  /* Ellipses stored a fourth point whose direction Fiberfox uses to twist
   * fiber bundles. Loading replayed the constraints of that time: the first
   * axis turned perpendicular to the second one, and the fourth point moved
   * to the larger radius, clamped to the plane. Returns the direction to the
   * fourth point.
   */
  std::optional<mitk::Vector2D> ConvertEllipse(const mitk::PlaneGeometry *plane, std::vector<mitk::Point2D> &points)
  {
    const auto center = points[0];
    const auto firstRadius = center.EuclideanDistanceTo(points[1]);
    const auto secondAxis = points[2] - center;
    const auto secondRadius = secondAxis.GetNorm();

    if (secondRadius > 0.0)
    {
      mitk::Vector2D firstAxis;
      firstAxis[0] = secondAxis[1];
      firstAxis[1] = -secondAxis[0];
      points[1] = center + firstAxis * (firstRadius / secondRadius);
    }

    auto twist = points[3] - center;
    points.pop_back();

    if (twist.GetNorm() <= 0.0)
      return std::nullopt;

    twist.Normalize();
    mitk::Point2D twistPoint = center + twist * std::max(firstRadius, secondRadius);
    ClampToPlane(plane, twistPoint);
    twist = twistPoint - center;

    if (twist.GetNorm() <= 0.0)
      return std::nullopt;

    return twist;
  }

  /* Crosses kept their second line perpendicular to the first one and its
   * crossing within the first line. Its second point lay on the other side of
   * the first line, or on it. Loading replayed these constraints, which also
   * repaired the coordinates of files that stored six significant digits.
   */
  void ConvertCross(std::vector<mitk::Point2D> &points)
  {
    const auto firstLine = points[1] - points[0];
    const auto length = firstLine.GetNorm();

    if (length <= 0.0)
      return;

    const auto direction = firstLine / length;

    mitk::Vector2D normal;
    normal[0] = -direction[1];
    normal[1] = direction[0];

    const auto crossingPoint = points[0] + direction * std::clamp((points[2] - points[0]) * direction, 0.0, length);
    const auto offset2 = (points[2] - points[0]) * normal;
    auto offset3 = (points[3] - points[0]) * normal;

    if (offset2 * offset3 >= 0.0)
      offset3 = 0.0;

    points[2] = crossingPoint + normal * offset2;
    points[3] = crossingPoint + normal * offset3;
  }

  /* Double ellipses stored the end of the first inner axis as a fourth point,
   * opposite of the first outer axis. Loading replayed the constraints of
   * that time: the second axis turned perpendicular to the first one and
   * became no longer than it. Returns the thickness of the ring.
   */
  double ConvertDoubleEllipse(std::vector<mitk::Point2D> &points)
  {
    const auto center = points[0];
    const auto firstAxis = points[1] - center;
    const auto firstRadius = firstAxis.GetNorm();
    const auto secondRadius = std::min(center.EuclideanDistanceTo(points[2]), firstRadius);

    if (firstRadius > 0.0)
    {
      mitk::Vector2D secondAxis;
      secondAxis[0] = firstAxis[1];
      secondAxis[1] = -firstAxis[0];
      points[2] = center + secondAxis * (secondRadius / firstRadius);
    }
    else
    {
      points[2] = center;
    }

    const auto thickness = std::clamp(firstRadius - center.EuclideanDistanceTo(points[3]), 0.0, secondRadius);
    points.pop_back();

    return thickness;
  }

  mitk::PlanarFigure::Pointer ReadFigure(const tinyxml2::XMLElement *element, const std::string &location)
  {
    const auto *typeAttribute = element->Attribute("type");
    const std::string type = nullptr != typeAttribute ? typeAttribute : "";
    const auto typeIdIter = TypeIds.find(type);

    if (TypeIds.end() == typeIdIter)
      mitkThrow() << "Unknown planar figure type \"" << type << "\".";

    auto typeId = typeIdIter->second;
    const auto plane = ReadPlane(element->FirstChildElement("Geometry"));
    auto points = ReadControlPoints(element->FirstChildElement("ControlPoints"));

    std::optional<mitk::Vector2D> referenceDirection;
    std::optional<double> thickness;

    if ("polygon" == typeId)
    {
      if (!GetBoolProperty(element, "closed", true))
        typeId = "path";
    }
    else if ("cross" == typeId)
    {
      if (GetBoolProperty(element, "SingleLineMode", false) || 2 == points.size())
      {
        MITK_WARN << "A PlanarCross with a single line is read as PlanarLine from \"" << location << "\".";
        typeId = "line";
      }
      else if (4 == points.size())
      {
        ConvertCross(points);
      }
    }
    else if ("rectangle" == typeId)
    {
      // Rectangles stored all four corners. Their placement started at corner 1 and ended at corner 3.
      if (4 == points.size())
        points = { points[1], points[3] };
    }
    else if ("circle" == typeId)
    {
      if (1 == points.size())
      {
        MITK_WARN << "The size of a fixed-size PlanarCircle was not stored. It is read with a radius of 0 from \"" << location << "\".";
        points.push_back(points[0]);
      }
    }
    else if ("ellipse" == typeId)
    {
      if (4 == points.size())
        referenceDirection = ConvertEllipse(plane, points);
    }
    else if ("double_ellipse" == typeId)
    {
      if (1 == points.size())
      {
        MITK_WARN << "The size of a fixed-size PlanarDoubleEllipse was not stored. It is read with a radius of 0 from \"" << location << "\".";
        points = { points[0], points[0], points[0] };
      }
      else if (4 == points.size())
      {
        thickness = ConvertDoubleEllipse(points);
      }
    }

    auto figure = mitk::CreatePlanarFigure(typeId);
    figure->SetPlaneGeometry(plane);
    figure->SetControlPoints(points);

    if (referenceDirection.has_value())
      static_cast<mitk::PlanarEllipse *>(figure.GetPointer())->SetReferenceDirection(referenceDirection);

    if (thickness.has_value())
      static_cast<mitk::PlanarDoubleEllipse *>(figure.GetPointer())->SetThickness(*thickness);

    return figure;
  }
}

mitk::LegacyPlanarFigureReader::LegacyPlanarFigureReader()
  : AbstractFileReader(PlanarFigureIOMimeTypes::LEGACY_PLANARFIGURE_MIMETYPE(), "MITK Planar Figure (XML)")
{
  this->RegisterService();
}

std::vector<mitk::BaseData::Pointer> mitk::LegacyPlanarFigureReader::DoRead()
{
  LocaleSwitch localeSwitch("C");

  tinyxml2::XMLDocument document;

  if (auto *stream = this->GetInputStream(); nullptr != stream)
  {
    // The files have several top-level elements, which a streamed document would end after the first.
    const std::string content(std::istreambuf_iterator<char>(*stream), {});
    document.Parse(content.c_str());
  }
  else
  {
    document.LoadFile(this->GetInputLocation().c_str());
  }

  if (document.Error())
    mitkThrow() << "Could not parse planar figure file \"" << this->GetInputLocation() << "\": " << document.ErrorStr();

  int fileVersion = 0;
  const auto *version = document.FirstChildElement("Version");

  if (nullptr == version || tinyxml2::XML_SUCCESS != version->QueryIntAttribute("FileVersion", &fileVersion) || 1 != fileVersion)
    mitkThrow() << "Planar figure file \"" << this->GetInputLocation() << "\" is not of version 1 of the XML format.";

  std::vector<BaseData::Pointer> figures;

  for (const auto *element = document.FirstChildElement("PlanarFigure"); nullptr != element;
       element = element->NextSiblingElement("PlanarFigure"))
  {
    figures.push_back(ReadFigure(element, this->GetInputLocation()).GetPointer());
  }

  if (figures.empty())
    mitkThrow() << "Planar figure file \"" << this->GetInputLocation() << "\" contains no planar figure.";

  return figures;
}

mitk::LegacyPlanarFigureReader *mitk::LegacyPlanarFigureReader::Clone() const
{
  return new LegacyPlanarFigureReader(*this);
}
