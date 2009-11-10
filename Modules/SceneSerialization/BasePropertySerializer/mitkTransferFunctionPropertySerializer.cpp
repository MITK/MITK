/*=========================================================================
 
Program:   Medical Imaging & Transferfunctioneraction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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
    LOG_ERROR << "cant serialize transfer function";
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
    LOG_ERROR << "Could not write scene to " << filename << "\nTinyXML reports '" << document.ErrorDesc() << "'";
    return false;
  }
  return true;
}

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(TransferFunctionPropertySerializer);