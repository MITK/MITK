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
  class MITK_OPENIGTLINK_EXPORT IGTLMessageCommon  : public itk::DataObject
    {
    public:
      mitkClassMacro(IGTLMessageCommon, itk::DataObject);
      itkFactorylessNewMacro(Self);
      itkCloneMacro(Self);

      /**
      * \brief Copies the content of a transform message original into another one
      * (clone)
      */
      static bool Clone(igtl::TransformMessage::Pointer clone, igtl::TransformMessage::Pointer original);

      /**
      * \brief Copies the content of a transform message original into another one
      * (clone)
      */
      static igtl::MessageBase::Pointer Clone(igtl::TransformMessage::Pointer original);


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
