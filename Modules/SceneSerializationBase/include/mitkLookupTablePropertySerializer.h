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

#ifndef mitkLookupTablePropertySerializer_h_included
#define mitkLookupTablePropertySerializer_h_included

#include <MitkSceneSerializationBaseExports.h>
#include "mitkBasePropertySerializer.h"


namespace mitk
{

/**
  \brief Base class for objects that serialize BaseData types.

  The name of sub-classes must be deduced from the class name of the object that should be serialized.
  The serialization assumes that

  \verbatim
  If the class derived from BaseData is called GreenData
  Then the serializer for this class must be called GreenDataSerializer
  \endverbatim
*/
class MITKSCENESERIALIZATIONBASE_EXPORT LookupTablePropertySerializer : public BasePropertySerializer
{
  public:
  
    mitkClassMacro( LookupTablePropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
      \brief Serializes given BaseData object.
      \return the filename of the newly created file.

      This should be overwritten by specific sub-classes.
      */
    virtual TiXmlElement* Serialize() override;
	
    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override;
	
  protected:

    LookupTablePropertySerializer(){};
    virtual ~LookupTablePropertySerializer(){};

};

}
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(LookupTablePropertySerializer);

#endif

