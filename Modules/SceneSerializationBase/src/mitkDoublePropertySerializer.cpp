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

#ifndef mitkDoublePropertySerializer_h_included
#define mitkDoublePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"
#include <boost/lexical_cast.hpp>

#include <MitkSceneSerializationBaseExports.h>

#include <mitkLocaleSwitch.h>

namespace mitk
{

class DoublePropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( DoublePropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const DoubleProperty* prop = dynamic_cast<const DoubleProperty*>(m_Property.GetPointer()))
      {
        LocaleSwitch localeSwitch("C");

        auto  element = new TiXmlElement("double");
        element->SetAttribute("value", boost::lexical_cast<std::string>(prop->GetValue()));
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;

      LocaleSwitch localeSwitch("C");

      std::string d;
      if ( element->QueryStringAttribute( "value", &d ) == TIXML_SUCCESS )
      {
        try
        {
          return DoubleProperty::New(boost::lexical_cast<double>(d)).GetPointer();
        }
        catch ( boost::bad_lexical_cast& e )
        {
          MITK_ERROR << "Could not parse string as number: " << e.what();
          return nullptr;
        }
      }
      else
      {
        return nullptr;
      }
    }

  protected:

    DoublePropertySerializer() {}
    virtual ~DoublePropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(DoublePropertySerializer);

#endif

