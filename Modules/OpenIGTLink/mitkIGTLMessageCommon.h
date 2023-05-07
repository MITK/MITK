/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkIGTLMessageCommon_h
#define mitkIGTLMessageCommon_h

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

      ~IGTLMessageCommon() override;

    private:

    };


} // namespace mitk
#endif
