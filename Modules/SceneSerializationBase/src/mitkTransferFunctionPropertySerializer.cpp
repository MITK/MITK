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
    TransferFunction* transferfunction = prop->GetValue();
    if (!transferfunction)
      return NULL;

    TiXmlElement* element = new TiXmlElement("TransferFunction");

    // serialize scalar opacity function
    TiXmlElement* scalarOpacityPointlist = new TiXmlElement( "ScalarOpacity" );

    TransferFunction::ControlPoints scalarOpacityPoints = transferfunction->GetScalarOpacityPoints();
    for ( TransferFunction::ControlPoints::iterator iter = scalarOpacityPoints.begin();
      iter != scalarOpacityPoints.end();
      ++iter )
    {
      TiXmlElement* pointel = new TiXmlElement("point");
      pointel->SetDoubleAttribute("x", iter->first);
      pointel->SetDoubleAttribute("y", iter->second);
      scalarOpacityPointlist->LinkEndChild( pointel );
    }
    element->LinkEndChild( scalarOpacityPointlist );
    // serialize gradient opacity function
    TiXmlElement* gradientOpacityPointlist = new TiXmlElement( "GradientOpacity" );
    TransferFunction::ControlPoints gradientOpacityPoints = transferfunction->GetGradientOpacityPoints();
    for ( TransferFunction::ControlPoints::iterator iter = gradientOpacityPoints.begin();
      iter != gradientOpacityPoints.end();
      ++iter )
    {
      TiXmlElement* pointel = new TiXmlElement("point");
      pointel->SetDoubleAttribute("x", iter->first);
      pointel->SetDoubleAttribute("y", iter->second);
      gradientOpacityPointlist->LinkEndChild( pointel );
    }
    element->LinkEndChild( gradientOpacityPointlist );

    // serialize color function
    vtkColorTransferFunction* ctf = transferfunction->GetColorTransferFunction();
    if (ctf == NULL)
      return NULL;
    TiXmlElement* pointlist = new TiXmlElement("Color");
    for (int i = 0; i < ctf->GetSize(); i++ )
    {
      double myVal[6];
      ctf->GetNodeValue(i, myVal);
      TiXmlElement* pointel = new TiXmlElement("point");
      pointel->SetDoubleAttribute("x", myVal[0]);
      pointel->SetDoubleAttribute("r", myVal[1]);
      pointel->SetDoubleAttribute("g", myVal[2]);
      pointel->SetDoubleAttribute("b", myVal[3]);
      pointel->SetDoubleAttribute("midpoint", myVal[4]);
      pointel->SetDoubleAttribute("sharpness", myVal[5]);
      pointlist->LinkEndChild( pointel );
    }
    element->LinkEndChild( pointlist );
    return element;
  }
  else return NULL;
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
  TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "" ); // TODO what to write here? encoding? standalone would mean that we provide a DTD somewhere...
  document.LinkEndChild( decl );

  TiXmlElement* version = new TiXmlElement("Version");
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
    return NULL;

  TransferFunction::Pointer tf = TransferFunction::New();

  // deserialize scalar opacity function
  TiXmlElement* scalarOpacityPointlist = element->FirstChildElement("ScalarOpacity");
  if (scalarOpacityPointlist == NULL)
    return NULL;

  tf->ClearScalarOpacityPoints();

  for( TiXmlElement* pointElement = scalarOpacityPointlist->FirstChildElement("point"); pointElement != NULL; pointElement = pointElement->NextSiblingElement("point"))
  {
    double x;
    double y;
    if (pointElement->QueryDoubleAttribute("x", &x) == TIXML_WRONG_TYPE)
      return NULL; // TODO: can we do a better error handling?
    if (pointElement->QueryDoubleAttribute("y", &y) == TIXML_WRONG_TYPE)
      return NULL; // TODO: can we do a better error handling?
    tf->AddScalarOpacityPoint(x, y);
  }

  TiXmlElement* gradientOpacityPointlist = element->FirstChildElement("GradientOpacity");
  if (gradientOpacityPointlist == NULL)
    return NULL;

  tf->ClearGradientOpacityPoints();

  for( TiXmlElement* pointElement = gradientOpacityPointlist->FirstChildElement("point"); pointElement != NULL; pointElement = pointElement->NextSiblingElement("point"))
  {
    double x;
    double y;
    if (pointElement->QueryDoubleAttribute("x", &x) == TIXML_WRONG_TYPE)
      return NULL; // TODO: can we do a better error handling?
    if (pointElement->QueryDoubleAttribute("y", &y) == TIXML_WRONG_TYPE)
      return NULL; // TODO: can we do a better error handling?
    tf->AddGradientOpacityPoint(x, y);
  }

  TiXmlElement* rgbPointlist = element->FirstChildElement("Color");
  if (rgbPointlist == NULL)
    return NULL;
  vtkColorTransferFunction* ctf = tf->GetColorTransferFunction();
  if (ctf == NULL)
    return NULL;

  ctf->RemoveAllPoints();

  for( TiXmlElement* pointElement = rgbPointlist->FirstChildElement("point"); pointElement != NULL; pointElement = pointElement->NextSiblingElement("point"))
  {
    double x;
    double r,g,b, midpoint, sharpness;
    if (pointElement->QueryDoubleAttribute("x", &x) == TIXML_WRONG_TYPE)
      return NULL; // TODO: can we do a better error handling?
    if (pointElement->QueryDoubleAttribute("r", &r) == TIXML_WRONG_TYPE)
      return NULL; // TODO: can we do a better error handling?
    if (pointElement->QueryDoubleAttribute("g", &g) == TIXML_WRONG_TYPE)
      return NULL; // TODO: can we do a better error handling?
    if (pointElement->QueryDoubleAttribute("b", &b) == TIXML_WRONG_TYPE)
      return NULL; // TODO: can we do a better error handling?
    if (pointElement->QueryDoubleAttribute("midpoint", &midpoint) == TIXML_WRONG_TYPE)
      return NULL; // TODO: can we do a better error handling?
    if (pointElement->QueryDoubleAttribute("sharpness", &sharpness) == TIXML_WRONG_TYPE)
      return NULL; // TODO: can we do a better error handling?
    ctf->AddRGBPoint(x, r, g, b, midpoint, sharpness);
  }
  return TransferFunctionProperty::New(tf).GetPointer();
}

mitk::TransferFunction::Pointer mitk::TransferFunctionPropertySerializer::DeserializeTransferFunction( const char *filePath )
{
  TiXmlDocument document( filePath );

  if (!document.LoadFile())
  {
    MITK_ERROR << "Could not open/read/parse " << filePath << "\nTinyXML reports: " << document.ErrorDesc() << std::endl;
    return NULL;
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
  return NULL;
}

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(TransferFunctionPropertySerializer);
