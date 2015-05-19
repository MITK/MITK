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


#ifndef MITKIGTLMESSAGECOMMONH_HEADER_INCLUDED_
#define MITKIGTLMESSAGECOMMONH_HEADER_INCLUDED_

#include <itkDataObject.h>
#include "MitkOpenIGTLinkExports.h"
#include <mitkNumericTypes.h>
#include <mitkCommon.h>

#include "igtlMessageBase.h"
#include "igtlTransformMessage.h"

namespace mitk {

    /**Documentation
    * \brief Helper class for copying OpenIGTLink messages.
    *
    * This class is a helper class for copying OpenIGTLink messages.
    *
    */
  class MITKOPENIGTLINK_EXPORT IGTLMessageCommon  : public itk::DataObject
    {
    public:
      mitkClassMacroItkParent(IGTLMessageCommon, itk::DataObject);
      itkFactorylessNewMacro(Self);
      itkCloneMacro(Self);

      /**
      * \brief Clones the input message and returns a igtl::MessageBase::Pointer
      * to the clone
      */
//      static igtl::MessageBase::Pointer Clone(igtl::TransformMessage::Pointer original);

      /**
      * \brief Clones the input message and returns a igtl::MessageBase::Pointer
      * to the clone
      */
//      static igtl::MessageBase::Pointer Clone(igtl::MessageBase::Pointer original);


    protected:
      IGTLMessageCommon();

      /**
       * Copy constructor internally used.
       */
//      IGTLMessageCommon(const mitk::IGTLMessageCommon& toCopy);

      virtual ~IGTLMessageCommon();

    private:

    };


} // namespace mitk
#endif /* MITKIGTLMESSAGECOMMONH_HEADER_INCLUDED_ */
