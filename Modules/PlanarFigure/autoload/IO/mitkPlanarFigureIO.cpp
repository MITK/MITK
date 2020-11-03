/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarFigureIO.h>

#include "mitkCustomMimeType.h"
#include "mitkIOMimeTypes.h"
#include "mitkExceptionMacro.h"

#include "mitkPlanarAngle.h"
#include "mitkPlanarArrow.h"
#include "mitkPlanarBezierCurve.h"
#include "mitkPlanarCircle.h"
#include "mitkPlanarCross.h"
#include "mitkPlanarDoubleEllipse.h"
#include "mitkPlanarEllipse.h"
#include "mitkPlanarFourPointAngle.h"
#include "mitkPlanarLine.h"
#include "mitkPlanarPolygon.h"
#include "mitkPlanarRectangle.h"
#include "mitkPlanarSubdivisionPolygon.h"
#include "mitkPlaneGeometry.h"

#include "mitkBasePropertySerializer.h"

#include <mitkLocaleSwitch.h>

#include <tinyxml.h>

namespace mitk
{

  PlanarFigureIO::PlanarFigureIO()
    : AbstractFileIO(PlanarFigure::GetStaticNameOfClass())
  {
    std::string category = "MITK PlanarFigure File";
    CustomMimeType customMimeType;
    customMimeType.SetCategory(category);
    customMimeType.AddExtension("pf");
    this->AbstractFileIOWriter::SetMimeType(customMimeType);
    this->AbstractFileIOWriter::SetDescription(category);

    customMimeType.AddExtension("pf");
    customMimeType.AddExtension("PF");
    this->AbstractFileIOReader::SetMimeType(customMimeType);
    this->AbstractFileIOReader::SetDescription(category);

    AbstractFileWriter::SetRanking(10);
    AbstractFileReader::SetRanking(10);
    this->RegisterService();
  }

  IFileIO::ConfidenceLevel PlanarFigureIO::GetWriterConfidenceLevel() const
  {
    if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
      return Unsupported;

    const auto *input = static_cast<const PlanarFigure *>(this->GetInput());
    if (input != nullptr)
      return Supported;
    else
      return Unsupported;
  }

  void PlanarFigureIO::Write()
  {
    this->ValidateOutputLocation();

    mitk::LocaleSwitch localeSwitch("C");

    TiXmlDocument document;
    auto decl = new TiXmlDeclaration("1.0", "", ""); // TODO what to write here? encoding? etc....
    document.LinkEndChild(decl);

    auto version = new TiXmlElement("Version");
    version->SetAttribute("Writer", __FILE__);
    version->SetAttribute("CVSRevision", "$Revision: 17055 $");
    version->SetAttribute("FileVersion", 1);
    document.LinkEndChild(version);

    auto pf = dynamic_cast<const PlanarFigure*>(this->GetInput());
    if (pf == nullptr)
    {
      mitkThrow() << "Try to safe a BaseData instance as PlanarFigure. That is not a planar figure. This should not happen and is a violated precondition. Please check the program logic.";
    }

    auto pfElement = new TiXmlElement("PlanarFigure");
    pfElement->SetAttribute("type", pf->GetNameOfClass());
    document.LinkEndChild(pfElement);

    // Serialize property list of PlanarFigure
    mitk::PropertyList::Pointer propertyList = pf->GetPropertyList();
    mitk::PropertyList::PropertyMap::const_iterator it;
    for (it = propertyList->GetMap()->begin(); it != propertyList->GetMap()->end(); ++it)
    {
      // Create seralizer for this property
      const mitk::BaseProperty* prop = it->second;
      std::string serializerName = std::string(prop->GetNameOfClass()) + "Serializer";
      std::list<itk::LightObject::Pointer> allSerializers =
        itk::ObjectFactoryBase::CreateAllInstance(serializerName.c_str());

      if (allSerializers.size() != 1)
      {
        // No or too many serializer(s) found, skip this property
        continue;
      }

      auto* serializer =
        dynamic_cast<mitk::BasePropertySerializer*>(allSerializers.begin()->GetPointer());
      if (serializer == nullptr)
      {
        // Serializer not valid; skip this property
      }

      auto keyElement = new TiXmlElement("property");
      keyElement->SetAttribute("key", it->first);
      keyElement->SetAttribute("type", prop->GetNameOfClass());

      serializer->SetProperty(prop);
      TiXmlElement* valueElement = nullptr;
      try
      {
        valueElement = serializer->Serialize();
      }
      catch (...)
      {
      }

      if (valueElement == nullptr)
      {
        // Serialization failed; skip this property
        continue;
      }

      // Add value to property element
      keyElement->LinkEndChild(valueElement);

      // Append serialized property to property list
      pfElement->LinkEndChild(keyElement);
    }

    // Serialize control points of PlanarFigure
    auto controlPointsElement = new TiXmlElement("ControlPoints");
    pfElement->LinkEndChild(controlPointsElement);
    for (unsigned int i = 0; i < pf->GetNumberOfControlPoints(); i++)
    {
      auto vElement = new TiXmlElement("Vertex");
      vElement->SetAttribute("id", i);
      vElement->SetDoubleAttribute("x", pf->GetControlPoint(i)[0]);
      vElement->SetDoubleAttribute("y", pf->GetControlPoint(i)[1]);
      controlPointsElement->LinkEndChild(vElement);
    }
    auto geoElement = new TiXmlElement("Geometry");
    const auto* planeGeo = dynamic_cast<const PlaneGeometry*>(pf->GetPlaneGeometry());
    if (planeGeo != nullptr)
    {
      // Write parameters of IndexToWorldTransform of the PlaneGeometry
      typedef mitk::Geometry3D::TransformType TransformType;
      const TransformType* affineGeometry = planeGeo->GetIndexToWorldTransform();
      const TransformType::ParametersType& parameters = affineGeometry->GetParameters();
      auto vElement = new TiXmlElement("transformParam");
      for (unsigned int i = 0; i < affineGeometry->GetNumberOfParameters(); ++i)
      {
        std::stringstream paramName;
        paramName << "param" << i;
        vElement->SetDoubleAttribute(paramName.str().c_str(), parameters.GetElement(i));
      }
      geoElement->LinkEndChild(vElement);

      // Write bounds of the PlaneGeometry
      typedef mitk::Geometry3D::BoundsArrayType BoundsArrayType;
      const BoundsArrayType& bounds = planeGeo->GetBounds();
      vElement = new TiXmlElement("boundsParam");
      for (unsigned int i = 0; i < 6; ++i)
      {
        std::stringstream boundName;
        boundName << "bound" << i;
        vElement->SetDoubleAttribute(boundName.str().c_str(), bounds.GetElement(i));
      }
      geoElement->LinkEndChild(vElement);

      // Write spacing and origin of the PlaneGeometry
      Vector3D spacing = planeGeo->GetSpacing();
      Point3D origin = planeGeo->GetOrigin();
      geoElement->LinkEndChild(this->CreateXMLVectorElement("Spacing", spacing));
      geoElement->LinkEndChild(this->CreateXMLVectorElement("Origin", origin));

      pfElement->LinkEndChild(geoElement);
    }

    if (this->GetOutputStream() != nullptr)
    {
      *(this->GetOutputStream()) << document;
    }
    else
    {
      if (document.SaveFile(this->GetOutputLocation()) == false)
      {
        MITK_ERROR << "Could not write planar figures to " << this->GetOutputLocation() << "\nTinyXML reports '" << document.ErrorDesc()
          << "'";
        throw std::ios_base::failure("Error during writing of planar figure xml file.");
      }
    }
  }

  TiXmlElement* mitk::PlanarFigureIO::CreateXMLVectorElement(const char* name, itk::FixedArray<mitk::ScalarType, 3> v)
  {
    auto vElement = new TiXmlElement(name);
    vElement->SetDoubleAttribute("x", v.GetElement(0));
    vElement->SetDoubleAttribute("y", v.GetElement(1));
    vElement->SetDoubleAttribute("z", v.GetElement(2));
    return vElement;
  }

  IFileIO::ConfidenceLevel PlanarFigureIO::GetReaderConfidenceLevel() const
  {
    if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported)
      return Unsupported;

    return Supported;
    //Remark: The original reader code assumed that al pf files can be read.
    //So no content checking was done. Thus was not implemented while refactoring
    //to services yet. But I think it would make sense.
  }

  std::vector<BaseData::Pointer> PlanarFigureIO::DoRead()
  {
    mitk::LocaleSwitch localeSwitch("C");

    std::vector<BaseData::Pointer> results;

    TiXmlDocument document;

    if (this->GetInputStream() != nullptr)
    {
      std::string s(std::istreambuf_iterator<char>(*(this->GetInputStream())), {});
      document.Parse(s.c_str());
      //Remark: didn't use *(this->GetInputStream()) >> document;
      //because our PlanarFigure files version 1 are illformed (multiple top level elements)
      //and therefor tinyxml does not read them completly when streamed directly.
      //only the first (version element) is read.
    }
    else
    {
      if (!document.LoadFile(this->GetInputLocation()))
      {
        MITK_ERROR << "Could not open/read/parse " << this->GetInputLocation() << ". TinyXML reports: '" << document.ErrorDesc()
          << "'. "
          << "The error occurred in row " << document.ErrorRow() << ", column " << document.ErrorCol() << ".";
        return {};
      }
    }

    int fileVersion = 1;
    TiXmlElement* versionObject = document.FirstChildElement("Version");
    if (versionObject != nullptr)
    {
      if (versionObject->QueryIntAttribute("FileVersion", &fileVersion) != TIXML_SUCCESS)
      {
        MITK_WARN << this->GetInputLocation() << " does not contain version information! Trying version 1 format." << std::endl;
      }
    }
    else
    {
      MITK_WARN << this->GetInputLocation() << " does not contain version information! Trying version 1 format." << std::endl;
    }
    if (fileVersion !=
      1) // add file version selection and version specific file parsing here, if newer file versions are created
    {
      MITK_WARN << "File version > 1 is not supported by this reader.";
      return {};
    }

    /* file version 1 reader code */
    for (TiXmlElement* pfElement = document.FirstChildElement("PlanarFigure"); pfElement != nullptr;
      pfElement = pfElement->NextSiblingElement("PlanarFigure"))
    {
      std::string type = pfElement->Attribute("type");

      mitk::PlanarFigure::Pointer planarFigure = nullptr;
      if (type == "PlanarAngle")
      {
        planarFigure = mitk::PlanarAngle::New();
      }
      else if (type == "PlanarCircle")
      {
        planarFigure = mitk::PlanarCircle::New();
      }
      else if (type == "PlanarEllipse")
      {
        planarFigure = mitk::PlanarEllipse::New();
      }
      else if (type == "PlanarCross")
      {
        planarFigure = mitk::PlanarCross::New();
      }
      else if (type == "PlanarFourPointAngle")
      {
        planarFigure = mitk::PlanarFourPointAngle::New();
      }
      else if (type == "PlanarLine")
      {
        planarFigure = mitk::PlanarLine::New();
      }
      else if (type == "PlanarPolygon")
      {
        planarFigure = mitk::PlanarPolygon::New();
      }
      else if (type == "PlanarSubdivisionPolygon")
      {
        planarFigure = mitk::PlanarSubdivisionPolygon::New();
      }
      else if (type == "PlanarRectangle")
      {
        planarFigure = mitk::PlanarRectangle::New();
      }
      else if (type == "PlanarArrow")
      {
        planarFigure = mitk::PlanarArrow::New();
      }
      else if (type == "PlanarDoubleEllipse")
      {
        planarFigure = mitk::PlanarDoubleEllipse::New();
      }
      else if (type == "PlanarBezierCurve")
      {
        planarFigure = mitk::PlanarBezierCurve::New();
      }
      else
      {
        // unknown type
        MITK_WARN << "encountered unknown planar figure type '" << type << "'. Skipping this element.";
        continue;
      }

      // Read properties of the planar figure
      for (TiXmlElement* propertyElement = pfElement->FirstChildElement("property"); propertyElement != nullptr;
        propertyElement = propertyElement->NextSiblingElement("property"))
      {
        const char* keya = propertyElement->Attribute("key");
        const std::string key(keya ? keya : "");

        const char* typea = propertyElement->Attribute("type");
        const std::string type(typea ? typea : "");

        // hand propertyElement to specific reader
        std::stringstream propertyDeserializerClassName;
        propertyDeserializerClassName << type << "Serializer";

        const std::list<itk::LightObject::Pointer> readers =
          itk::ObjectFactoryBase::CreateAllInstance(propertyDeserializerClassName.str().c_str());
        if (readers.size() < 1)
        {
          MITK_ERROR << "No property reader found for " << type;
        }
        if (readers.size() > 1)
        {
          MITK_WARN << "Multiple property readers found for " << type << ". Using arbitrary first one.";
        }

        for (auto iter = readers.cbegin(); iter != readers.cend(); ++iter)
        {
          if (auto* reader = dynamic_cast<BasePropertySerializer*>(iter->GetPointer()))
          {
            const BaseProperty::Pointer property = reader->Deserialize(propertyElement->FirstChildElement());
            if (property.IsNotNull())
            {
              planarFigure->GetPropertyList()->ReplaceProperty(key, property);
            }
            else
            {
              MITK_ERROR << "There were errors while loading property '" << key << "' of type " << type
                << ". Your data may be corrupted";
            }
            break;
          }
        }
      }

      // If we load a planarFigure, it has definitely been placed correctly.
      // If we do not set this property here, we cannot load old planarFigures
      // without messing up the interaction (PF-Interactor needs this property.
      planarFigure->GetPropertyList()->SetBoolProperty("initiallyplaced", true);

      // Which features (length or circumference etc) a figure has is decided by whether it is closed or not
      // the function SetClosed has to be called in case of PlanarPolygons to ensure they hold the correct feature
      auto* planarPolygon = dynamic_cast<PlanarPolygon*>(planarFigure.GetPointer());
      if (planarPolygon != nullptr)
      {
        bool isClosed = false;
        planarFigure->GetPropertyList()->GetBoolProperty("closed", isClosed);
        planarPolygon->SetClosed(isClosed);
      }

      // Read geometry of containing plane
      TiXmlElement* geoElement = pfElement->FirstChildElement("Geometry");
      if (geoElement != nullptr)
      {
        try
        {
          // Create plane geometry
          mitk::PlaneGeometry::Pointer planeGeo = mitk::PlaneGeometry::New();

          // Extract and set plane transform parameters
          const DoubleList transformList =
            this->GetDoubleAttributeListFromXMLNode(geoElement->FirstChildElement("transformParam"), "param", 12);

          typedef mitk::BaseGeometry::TransformType TransformType;
          TransformType::ParametersType parameters;
          parameters.SetSize(12);

          unsigned int i;
          DoubleList::const_iterator it;
          for (it = transformList.cbegin(), i = 0; it != transformList.cend(); ++it, ++i)
          {
            parameters.SetElement(i, *it);
          }

          typedef mitk::BaseGeometry::TransformType TransformType;
          TransformType::Pointer affineGeometry = TransformType::New();
          affineGeometry->SetParameters(parameters);
          planeGeo->SetIndexToWorldTransform(affineGeometry);

          // Extract and set plane bounds
          const DoubleList boundsList =
            this->GetDoubleAttributeListFromXMLNode(geoElement->FirstChildElement("boundsParam"), "bound", 6);

          typedef mitk::BaseGeometry::BoundsArrayType BoundsArrayType;

          BoundsArrayType bounds;
          for (it = boundsList.cbegin(), i = 0; it != boundsList.cend(); ++it, ++i)
          {
            bounds[i] = *it;
          }

          planeGeo->SetBounds(bounds);

          // Extract and set spacing and origin
          const Vector3D spacing = this->GetVectorFromXMLNode(geoElement->FirstChildElement("Spacing"));
          planeGeo->SetSpacing(spacing);

          const Point3D origin = this->GetPointFromXMLNode(geoElement->FirstChildElement("Origin"));
          planeGeo->SetOrigin(origin);
          planarFigure->SetPlaneGeometry(planeGeo);
        }
        catch (...)
        {
        }
      }
      TiXmlElement* cpElement = pfElement->FirstChildElement("ControlPoints");
      bool first = true;
      if (cpElement != nullptr)
        for (TiXmlElement* vertElement = cpElement->FirstChildElement("Vertex"); vertElement != nullptr;
          vertElement = vertElement->NextSiblingElement("Vertex"))
      {
        int id = 0;
        mitk::Point2D::ValueType x = 0.0;
        mitk::Point2D::ValueType y = 0.0;
        if (vertElement->QueryIntAttribute("id", &id) == TIXML_WRONG_TYPE)
          return{}; // TODO: can we do a better error handling?
        if (vertElement->QueryDoubleAttribute("x", &x) == TIXML_WRONG_TYPE)
          return{}; // TODO: can we do a better error handling?
        if (vertElement->QueryDoubleAttribute("y", &y) == TIXML_WRONG_TYPE)
          return{}; // TODO: can we do a better error handling?
        Point2D p;
        p.SetElement(0, x);
        p.SetElement(1, y);
        if (first == true) // needed to set m_FigurePlaced to true
        {
          planarFigure->PlaceFigure(p);
          first = false;
        }
        planarFigure->SetControlPoint(id, p, true);
      }

      // Calculate feature quantities of this PlanarFigure
      planarFigure->EvaluateFeatures();

      // Make sure that no control point is currently selected
      planarFigure->DeselectControlPoint();

      if (planarFigure.IsNotNull())
      {
        results.emplace_back(planarFigure);
      }
    }

    return results;
  }

  mitk::PlanarFigureIO::DoubleList mitk::PlanarFigureIO::GetDoubleAttributeListFromXMLNode(
    TiXmlElement* e, const char* attributeNameBase, unsigned int count)
  {
    DoubleList list;

    if (e == nullptr)
      throw std::invalid_argument("node invalid"); // TODO: can we do a better error handling?

    for (unsigned int i = 0; i < count; ++i)
    {
      mitk::ScalarType p(-1.0);
      std::stringstream attributeName;
      attributeName << attributeNameBase << i;

      if (e->QueryDoubleAttribute(attributeName.str().c_str(), &p) == TIXML_WRONG_TYPE)
        throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
      list.push_back(p);
    }

    return list;
  }

  mitk::Point3D mitk::PlanarFigureIO::GetPointFromXMLNode(TiXmlElement* e)
  {
    if (e == nullptr)
      throw std::invalid_argument("node invalid"); // TODO: can we do a better error handling?
    mitk::Point3D point;
    mitk::ScalarType p(-1.0);
    if (e->QueryDoubleAttribute("x", &p) == TIXML_WRONG_TYPE)
      throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
    point.SetElement(0, p);
    if (e->QueryDoubleAttribute("y", &p) == TIXML_WRONG_TYPE)
      throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
    point.SetElement(1, p);
    if (e->QueryDoubleAttribute("z", &p) == TIXML_WRONG_TYPE)
      throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
    point.SetElement(2, p);
    return point;
  }

  mitk::Vector3D mitk::PlanarFigureIO::GetVectorFromXMLNode(TiXmlElement* e)
  {
    if (e == nullptr)
      throw std::invalid_argument("node invalid"); // TODO: can we do a better error handling?
    mitk::Vector3D vector;
    mitk::ScalarType p(-1.0);
    if (e->QueryDoubleAttribute("x", &p) == TIXML_WRONG_TYPE)
      throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
    vector.SetElement(0, p);
    if (e->QueryDoubleAttribute("y", &p) == TIXML_WRONG_TYPE)
      throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
    vector.SetElement(1, p);
    if (e->QueryDoubleAttribute("z", &p) == TIXML_WRONG_TYPE)
      throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
    vector.SetElement(2, p);
    return vector;
  }

  PlanarFigureIO *PlanarFigureIO::IOClone() const { return new PlanarFigureIO(*this); }

} // namespace
