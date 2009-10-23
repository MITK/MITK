/*=========================================================================
 
Program:   Medical Imaging & TransferFunctioneraction Toolkit
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

#ifndef mitkTransferFunctionPropertyDeserializer_h_included
#define mitkTransferFunctionPropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkTransferFunctionProperty.h"

namespace mitk
{

class SceneSerialization_EXPORT TransferFunctionPropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( TransferFunctionPropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
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

  protected:

    TransferFunctionPropertyDeserializer() {}
    virtual ~TransferFunctionPropertyDeserializer() {}
};

TransferFunction::Pointer SceneSerialization_EXPORT DeserializeTransferFunction( const char *filePath )
{
  TiXmlDocument document( filePath );
  
  if (!document.LoadFile())
  {
    LOG_ERROR << "Could not open/read/parse " << filePath << "\nTinyXML reports: " << document.ErrorDesc() << std::endl;
    return NULL;
  }
      
  // find version node --> note version in some variable
  int fileVersion = 1;
  TiXmlElement* versionObject = document.FirstChildElement("Version");
  if (versionObject)
  {
    if ( versionObject->QueryIntAttribute( "TransferfunctionVersion", &fileVersion ) != TIXML_SUCCESS )
    {
      LOG_WARN << "Transferfunction file " << filePath << " does not contain version information! Trying version 1 format.";
    }
    else
    {
      LOG_INFO << "Transferfunction file " << filePath << " is of version " << fileVersion;
    }
  }
  
  TiXmlElement* input =  document.FirstChildElement("TransferFunction");
  
  TransferFunctionPropertyDeserializer::Pointer tfpd=TransferFunctionPropertyDeserializer::New();
  
  BaseProperty::Pointer bp = tfpd->Deserialize(input);
  
  TransferFunctionProperty::Pointer tfp = dynamic_cast<TransferFunctionProperty*>(bp.GetPointer());
  
  if(tfp.IsNotNull())
  {
    TransferFunction::Pointer tf = tfp->GetValue();
    return tf;
  }

  LOG_WARN << "Can't deserialize transferfunction";
  
  return NULL;
}


} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(TransferFunctionPropertyDeserializer);

#endif

