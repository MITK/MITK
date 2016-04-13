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

#include "mitkTransferFunctionPropertySerializer.h"
#include <boost/lexical_cast.hpp>
#include <mitkLocaleSwitch.h>

namespace mitk {

mitk::TransferFunctionPropertySerializer::TransferFunctionPropertySerializer()
{
}

mitk::TransferFunctionPropertySerializer::~TransferFunctionPropertySerializer()
{
}

TiXmlElement* mitk::TransferFunctionPropertySerializer::Serialize()
{
  if (const TransferFunctionProperty* prop = dynamic_cast<const TransferFunctionProperty*>(mitk::BasePropertySerializer::m_Property.GetPointer()))
  {
    LocaleSwitch localeSwitch("C");

    TransferFunction* transferfunction = prop->GetValue();
    if (!transferfunction)
      return nullptr;

    auto  element = new TiXmlElement("TransferFunction");

    // serialize scalar opacity function
    auto  scalarOpacityPointlist = new TiXmlElement( "ScalarOpacity" );

    TransferFunction::ControlPoints scalarOpacityPoints = transferfunction->GetScalarOpacityPoints();
    for ( auto iter = scalarOpacityPoints.begin();
      iter != scalarOpacityPoints.end();
      ++iter )
    {
      auto  pointel = new TiXmlElement("point");
      pointel->SetAttribute("x", boost::lexical_cast<std::string>(iter->first));
      pointel->SetAttribute("y", boost::lexical_cast<std::string>(iter->second));
      scalarOpacityPointlist->LinkEndChild( pointel );
    }
    element->LinkEndChild( scalarOpacityPointlist );
    // serialize gradient opacity function
    auto  gradientOpacityPointlist = new TiXmlElement( "GradientOpacity" );
    TransferFunction::ControlPoints gradientOpacityPoints = transferfunction->GetGradientOpacityPoints();
    for ( auto iter = gradientOpacityPoints.begin();
      iter != gradientOpacityPoints.end();
      ++iter )
    {
      auto  pointel = new TiXmlElement("point");
      pointel->SetAttribute("x", boost::lexical_cast<std::string>(iter->first));
      pointel->SetAttribute("y", boost::lexical_cast<std::string>(iter->second));
      gradientOpacityPointlist->LinkEndChild( pointel );
    }
    element->LinkEndChild( gradientOpacityPointlist );

    // serialize color function
    vtkColorTransferFunction* ctf = transferfunction->GetColorTransferFunction();
    if (ctf == nullptr)
      return nullptr;
    auto  pointlist = new TiXmlElement("Color");
    for (int i = 0; i < ctf->GetSize(); i++ )
    {
      double myVal[6];
      ctf->GetNodeValue(i, myVal);
      auto  pointel = new TiXmlElement("point");
      pointel->SetAttribute("x", boost::lexical_cast<std::string>(myVal[0]));
      pointel->SetAttribute("r", boost::lexical_cast<std::string>(myVal[1]));
      pointel->SetAttribute("g", boost::lexical_cast<std::string>(myVal[2]));
      pointel->SetAttribute("b", boost::lexical_cast<std::string>(myVal[3]));
      pointel->SetAttribute("midpoint", boost::lexical_cast<std::string>(myVal[4]));
      pointel->SetAttribute("sharpness", boost::lexical_cast<std::string>(myVal[5]));
      pointlist->LinkEndChild( pointel );
    }
    element->LinkEndChild( pointlist );
    return element;
  }
  else return nullptr;
}

bool mitk::TransferFunctionPropertySerializer::SerializeTransferFunction( const char * filename, TransferFunction::Pointer tf )
{
  TransferFunctionPropertySerializer::Pointer tfps=TransferFunctionPropertySerializer::New();
  tfps->SetProperty( TransferFunctionProperty::New( tf ) );
  TiXmlElement* s=tfps->Serialize();

  if(!s)
  {
    MITK_ERROR << "cant serialize transfer function";

    return false;
  }

  TiXmlDocument document;
  auto  decl = new TiXmlDeclaration( "1.0", "UTF-8", "" ); // TODO what to write here? encoding? standalone would mean that we provide a DTD somewhere...
  document.LinkEndChild( decl );

  auto  version = new TiXmlElement("Version");
  version->SetAttribute("TransferfunctionVersion",  1 );

  document.LinkEndChild(version);
  document.LinkEndChild(s);

  if ( !document.SaveFile( filename ) )
  {
    MITK_ERROR << "Could not write scene to " << filename << "\nTinyXML reports '" << document.ErrorDesc() << "'";
    return false;
  }
  return true;
}

BaseProperty::Pointer mitk::TransferFunctionPropertySerializer::Deserialize(TiXmlElement* element)
{
  if (!element)
    return nullptr;

  mitk::LocaleSwitch localeSwitch("C");

  TransferFunction::Pointer tf = TransferFunction::New();

  // deserialize scalar opacity function
  TiXmlElement* scalarOpacityPointlist = element->FirstChildElement("ScalarOpacity");
  if (scalarOpacityPointlist == nullptr)
  {
    return nullptr;
  }

  tf->ClearScalarOpacityPoints();

  try
  {
    for( TiXmlElement* pointElement = scalarOpacityPointlist->FirstChildElement("point");
         pointElement != nullptr;
         pointElement = pointElement->NextSiblingElement("point"))
    {
      std::string x;
      std::string y;
      if (pointElement->QueryStringAttribute("x", &x) != TIXML_SUCCESS) return nullptr;
      if (pointElement->QueryStringAttribute("y", &y) != TIXML_SUCCESS) return nullptr;
      tf->AddScalarOpacityPoint(boost::lexical_cast<double>(x), boost::lexical_cast<double>(y));
    }

    TiXmlElement* gradientOpacityPointlist = element->FirstChildElement("GradientOpacity");
    if (gradientOpacityPointlist == nullptr)
    {
      return nullptr;
    }

    tf->ClearGradientOpacityPoints();

    for( TiXmlElement* pointElement = gradientOpacityPointlist->FirstChildElement("point");
         pointElement != nullptr;
         pointElement = pointElement->NextSiblingElement("point"))
    {
      std::string x;
      std::string y;
      if (pointElement->QueryStringAttribute("x", &x) != TIXML_SUCCESS) return nullptr;
      if (pointElement->QueryStringAttribute("y", &y) != TIXML_SUCCESS) return nullptr;
      tf->AddGradientOpacityPoint(boost::lexical_cast<double>(x), boost::lexical_cast<double>(y));
    }

    TiXmlElement* rgbPointlist = element->FirstChildElement("Color");
    if (rgbPointlist == nullptr)
    {
      return nullptr;
    }
    vtkColorTransferFunction* ctf = tf->GetColorTransferFunction();
    if (ctf == nullptr)
    {
      return nullptr;
    }

    ctf->RemoveAllPoints();

    for( TiXmlElement* pointElement = rgbPointlist->FirstChildElement("point");
         pointElement != nullptr;
         pointElement = pointElement->NextSiblingElement("point"))
    {
      std::string x;
      std::string r,g,b, midpoint, sharpness;
      if (pointElement->QueryStringAttribute("x", &x) != TIXML_SUCCESS) return nullptr;
      if (pointElement->QueryStringAttribute("r", &r) != TIXML_SUCCESS) return nullptr;
      if (pointElement->QueryStringAttribute("g", &g) != TIXML_SUCCESS) return nullptr;
      if (pointElement->QueryStringAttribute("b", &b) != TIXML_SUCCESS) return nullptr;
      if (pointElement->QueryStringAttribute("midpoint", &midpoint) != TIXML_SUCCESS) return nullptr;
      if (pointElement->QueryStringAttribute("sharpness", &sharpness) != TIXML_SUCCESS) return nullptr;
      ctf->AddRGBPoint(boost::lexical_cast<double>(x),
                       boost::lexical_cast<double>(r), boost::lexical_cast<double>(g), boost::lexical_cast<double>(b),
                       boost::lexical_cast<double>(midpoint),
                       boost::lexical_cast<double>(sharpness));
    }
  }
  catch ( boost::bad_lexical_cast& e )
  {
    MITK_ERROR << "Could not parse string as number: " << e.what();

    return nullptr;
  }

  return TransferFunctionProperty::New(tf).GetPointer();
}

mitk::TransferFunction::Pointer mitk::TransferFunctionPropertySerializer::DeserializeTransferFunction( const char *filePath )
{
  TiXmlDocument document( filePath );

  if (!document.LoadFile())
  {
    MITK_ERROR << "Could not open/read/parse " << filePath << "\nTinyXML reports: " << document.ErrorDesc() << std::endl;
    return nullptr;
  }

  // find version node --> note version in some variable
  int fileVersion = 1;
  TiXmlElement* versionObject = document.FirstChildElement("Version");
  if (versionObject)
  {
    if ( versionObject->QueryIntAttribute( "TransferfunctionVersion", &fileVersion ) != TIXML_SUCCESS )
    {
      MITK_WARN << "Transferfunction file " << filePath << " does not contain version information! Trying version 1 format.";
    }
  }

  TiXmlElement* input =  document.FirstChildElement("TransferFunction");

  TransferFunctionPropertySerializer::Pointer tfpd = TransferFunctionPropertySerializer::New();
  BaseProperty::Pointer bp = tfpd->Deserialize(input);
  TransferFunctionProperty::Pointer tfp = dynamic_cast<TransferFunctionProperty*>(bp.GetPointer());

  if(tfp.IsNotNull())
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
