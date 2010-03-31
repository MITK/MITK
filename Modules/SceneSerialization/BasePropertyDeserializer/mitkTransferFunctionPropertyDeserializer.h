/*=========================================================================
 
Program:   Medical Imaging & TransferFunctioneraction Toolkit
Language:  C++
Date:      $Date: 2009-10-23 11:12:48 +0200 (Fr, 23 Okt 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkBasePropertyDeserializer.h"
#include "mitkTransferFunctionProperty.h"

namespace mitk
{

  class SceneSerialization_EXPORT TransferFunctionPropertyDeserializer : public BasePropertyDeserializer
  {
    public:
      
      mitkClassMacro( TransferFunctionPropertyDeserializer, BasePropertyDeserializer );
      itkNewMacro(Self);

      virtual BaseProperty::Pointer Deserialize(TiXmlElement* element);
      static TransferFunction::Pointer DeserializeTransferFunction( const char *filePath );


    protected:

      TransferFunctionPropertyDeserializer();
      virtual ~TransferFunctionPropertyDeserializer();
  };

} // namespace





