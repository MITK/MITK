/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTransferFunctionPropertySerializer.h"
#include <mitkLexicalCast.h>
#include <mitkLocaleSwitch.h>
#include <tinyxml2.h>

namespace mitk
{
  mitk::TransferFunctionPropertySerializer::TransferFunctionPropertySerializer() {}
  mitk::TransferFunctionPropertySerializer::~TransferFunctionPropertySerializer() {}
  tinyxml2::XMLElement *mitk::TransferFunctionPropertySerializer::Serialize(tinyxml2::XMLDocument &doc)
  {
    if (const auto *prop =
          dynamic_cast<const TransferFunctionProperty *>(mitk::BasePropertySerializer::m_Property.GetPointer()))
    {
      LocaleSwitch localeSwitch("C");

      TransferFunction *transferfunction = prop->GetValue();
      if (!transferfunction)
        return nullptr;

      auto *element = doc.NewElement("TransferFunction");

      // serialize scalar opacity function
      auto *scalarOpacityPointlist = doc.NewElement("ScalarOpacity");

      TransferFunction::ControlPoints scalarOpacityPoints = transferfunction->GetScalarOpacityPoints();
      for (auto iter = scalarOpacityPoints.begin(); iter != scalarOpacityPoints.end(); ++iter)
      {
        auto *pointel = doc.NewElement("point");
        pointel->SetAttribute("x", boost::lexical_cast<std::string>(iter->first).c_str());
        pointel->SetAttribute("y", boost::lexical_cast<std::string>(iter->second).c_str());
        scalarOpacityPointlist->InsertEndChild(pointel);
      }
      element->InsertEndChild(scalarOpacityPointlist);
      // serialize gradient opacity function
      auto *gradientOpacityPointlist = doc.NewElement("GradientOpacity");
      TransferFunction::ControlPoints gradientOpacityPoints = transferfunction->GetGradientOpacityPoints();
      for (auto iter = gradientOpacityPoints.begin(); iter != gradientOpacityPoints.end(); ++iter)
      {
        auto *pointel = doc.NewElement("point");
        pointel->SetAttribute("x", boost::lexical_cast<std::string>(iter->first).c_str());
        pointel->SetAttribute("y", boost::lexical_cast<std::string>(iter->second).c_str());
        gradientOpacityPointlist->InsertEndChild(pointel);
      }
      element->InsertEndChild(gradientOpacityPointlist);

      // serialize color function
      vtkColorTransferFunction *ctf = transferfunction->GetColorTransferFunction();
      if (ctf == nullptr)
        return nullptr;
      auto *pointlist = doc.NewElement("Color");
      for (int i = 0; i < ctf->GetSize(); i++)
      {
        double myVal[6];
        ctf->GetNodeValue(i, myVal);
        auto *pointel = doc.NewElement("point");
        pointel->SetAttribute("x", boost::lexical_cast<std::string>(myVal[0]).c_str());
        pointel->SetAttribute("r", boost::lexical_cast<std::string>(myVal[1]).c_str());
        pointel->SetAttribute("g", boost::lexical_cast<std::string>(myVal[2]).c_str());
        pointel->SetAttribute("b", boost::lexical_cast<std::string>(myVal[3]).c_str());
        pointel->SetAttribute("midpoint", boost::lexical_cast<std::string>(myVal[4]).c_str());
        pointel->SetAttribute("sharpness", boost::lexical_cast<std::string>(myVal[5]).c_str());
        pointlist->InsertEndChild(pointel);
      }
      element->InsertEndChild(pointlist);
      return element;
    }
    else
      return nullptr;
  }

  bool mitk::TransferFunctionPropertySerializer::SerializeTransferFunction(const char *filename,
                                                                           TransferFunction::Pointer tf)
  {
    TransferFunctionPropertySerializer::Pointer tfps = TransferFunctionPropertySerializer::New();
    tfps->SetProperty(TransferFunctionProperty::New(tf));

    tinyxml2::XMLDocument doc;
    auto *s = tfps->Serialize(doc);

    if (!s)
    {
      MITK_ERROR << "cant serialize transfer function";

      return false;
    }

    tinyxml2::XMLDocument doc2;
    doc2.InsertEndChild(doc2.NewDeclaration());

    auto *version = doc2.NewElement("Version");
    version->SetAttribute("TransferfunctionVersion", 1);

    doc2.InsertEndChild(version);
    doc2.InsertEndChild(s->DeepClone(&doc2));

    if (tinyxml2::XML_SUCCESS != doc2.SaveFile(filename))
    {
      MITK_ERROR << "Could not write scene to " << filename << "\nTinyXML reports '" << doc2.ErrorStr() << "'";
      return false;
    }
    return true;
  }

  BaseProperty::Pointer mitk::TransferFunctionPropertySerializer::Deserialize(const tinyxml2::XMLElement *element)
  {
    if (!element)
      return nullptr;

    mitk::LocaleSwitch localeSwitch("C");

    TransferFunction::Pointer tf = TransferFunction::New();

    // deserialize scalar opacity function
    auto *scalarOpacityPointlist = element->FirstChildElement("ScalarOpacity");
    if (scalarOpacityPointlist == nullptr)
    {
      return nullptr;
    }

    tf->ClearScalarOpacityPoints();

    try
    {
      for (auto *pointElement = scalarOpacityPointlist->FirstChildElement("point"); pointElement != nullptr;
           pointElement = pointElement->NextSiblingElement("point"))
      {
        const char* x = pointElement->Attribute("x");
        const char* y = pointElement->Attribute("y");
        if (nullptr == x || nullptr == y)
          return nullptr;
        tf->AddScalarOpacityPoint(boost::lexical_cast<double>(x), boost::lexical_cast<double>(y));
      }

      auto *gradientOpacityPointlist = element->FirstChildElement("GradientOpacity");
      if (gradientOpacityPointlist == nullptr)
      {
        return nullptr;
      }

      tf->ClearGradientOpacityPoints();

      for (auto *pointElement = gradientOpacityPointlist->FirstChildElement("point"); pointElement != nullptr;
           pointElement = pointElement->NextSiblingElement("point"))
      {
        const char* x = pointElement->Attribute("x");
        const char* y = pointElement->Attribute("y");
        if (nullptr == x || nullptr == y)
          return nullptr;
        tf->AddGradientOpacityPoint(boost::lexical_cast<double>(x), boost::lexical_cast<double>(y));
      }

      auto *rgbPointlist = element->FirstChildElement("Color");
      if (rgbPointlist == nullptr)
      {
        return nullptr;
      }
      vtkColorTransferFunction *ctf = tf->GetColorTransferFunction();
      if (ctf == nullptr)
      {
        return nullptr;
      }

      ctf->RemoveAllPoints();

      for (auto *pointElement = rgbPointlist->FirstChildElement("point"); pointElement != nullptr;
           pointElement = pointElement->NextSiblingElement("point"))
      {
        const char* x = pointElement->Attribute("x");
        const char* r = pointElement->Attribute("r");
        const char* g = pointElement->Attribute("g");
        const char* b = pointElement->Attribute("b");
        const char* midpoint = pointElement->Attribute("midpoint");
        const char* sharpness = pointElement->Attribute("sharpness");
        if (nullptr == x || nullptr == r || nullptr == g || nullptr == b || nullptr == midpoint || nullptr == sharpness)
          return nullptr;
        ctf->AddRGBPoint(boost::lexical_cast<double>(x),
                         boost::lexical_cast<double>(r),
                         boost::lexical_cast<double>(g),
                         boost::lexical_cast<double>(b),
                         boost::lexical_cast<double>(midpoint),
                         boost::lexical_cast<double>(sharpness));
      }
    }
    catch (boost::bad_lexical_cast &e)
    {
      MITK_ERROR << "Could not parse string as number: " << e.what();

      return nullptr;
    }

    return TransferFunctionProperty::New(tf).GetPointer();
  }

  mitk::TransferFunction::Pointer mitk::TransferFunctionPropertySerializer::DeserializeTransferFunction(
    const char *filePath)
  {
    tinyxml2::XMLDocument document;

    if (tinyxml2::XML_SUCCESS != document.LoadFile(filePath))
    {
      MITK_ERROR << "Could not open/read/parse " << filePath << "\nTinyXML reports: " << document.ErrorStr()
                 << std::endl;
      return nullptr;
    }

    // find version node --> note version in some variable
    int fileVersion = 1;
    auto *versionObject = document.FirstChildElement("Version");
    if (versionObject)
    {
      if (versionObject->QueryIntAttribute("TransferfunctionVersion", &fileVersion) != tinyxml2::XML_SUCCESS)
      {
        MITK_WARN << "Transferfunction file " << filePath
                  << " does not contain version information! Trying version 1 format.";
      }
    }

    auto *input = document.FirstChildElement("TransferFunction");

    TransferFunctionPropertySerializer::Pointer tfpd = TransferFunctionPropertySerializer::New();
    BaseProperty::Pointer bp = tfpd->Deserialize(input);
    TransferFunctionProperty::Pointer tfp = dynamic_cast<TransferFunctionProperty *>(bp.GetPointer());

    if (tfp.IsNotNull())
    {
      TransferFunction::Pointer tf = tfp->GetValue();
      return tf;
    }
    MITK_WARN << "Can't deserialize transfer function";
    return nullptr;
  }

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(TransferFunctionPropertySerializer);
