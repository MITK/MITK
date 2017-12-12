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

#ifndef mitkMeasurementFramePropertySerializer_h_included
#define mitkMeasurementFramePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkMeasurementFrameProperty.h"

#include <MitkDiffusionCoreExports.h>

namespace mitk
{

class MITKDIFFUSIONCORE_EXPORT MeasurementFramePropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( MeasurementFramePropertySerializer, BasePropertySerializer )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    TiXmlElement* Serialize() override
    {
      if (const MeasurementFrameProperty* prop = dynamic_cast<const MeasurementFrameProperty*>(m_Property.GetPointer()))
      {

        typedef mitk::MeasurementFrameProperty::MeasurementFrameType MeasurementFrameType;
        const MeasurementFrameType & mft = prop->GetMeasurementFrame();

        if(mft.is_zero()) return nullptr;

        auto  element = new TiXmlElement("measurementframe");

        auto  child = new TiXmlElement("entry");
        std::stringstream ss;
        ss << mft;
        child->SetAttribute("value", ss.str());
        element->InsertEndChild(*child);

        return element;
      }
      else return nullptr;
    }

    BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;

      TiXmlElement* entry = element->FirstChildElement( "entry" )->ToElement();

      std::stringstream ss;
      std::string value;

      entry->QueryStringAttribute("value",&value);
      ss << value;

      MeasurementFrameProperty::MeasurementFrameType matrix;
      matrix.read_ascii(ss);

      return MeasurementFrameProperty::New(matrix).GetPointer();
    }

  protected:

    MeasurementFramePropertySerializer() {}
    ~MeasurementFramePropertySerializer() override {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(MeasurementFramePropertySerializer)

#endif
