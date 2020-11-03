/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKIGTLMESSAGEFACTORYH_HEADER_INCLUDED_
#define MITKIGTLMESSAGEFACTORYH_HEADER_INCLUDED_

#include "MitkOpenIGTLinkExports.h"
#include "mitkCommon.h"

#include "igtlMessageBase.h"
#include "igtlMessageHeader.h"

#include "mitkIGTLMessageCloneHandler.h"

namespace mitk {
  /**
  * \brief Factory class of supported OpenIGTLink message types
  *
  * This class is a factory for the creation of OpenIGTLink messages. It stores
  * pairs of type and pointer to the message new method. Available standard types
  * are already added but you can also add your custom types at runtime.
  *
  */
  class MITKOPENIGTLINK_EXPORT IGTLMessageFactory : public itk::Object
  {
  public:
    mitkClassMacroItkParent(IGTLMessageFactory, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
    * \brief Function pointer for storing New() static methods of
    * igtl::MessageBase classes
    */
    typedef igtl::MessageBase::Pointer (*PointerToMessageBaseNew)();

    /**
    * \brief Add message type name and pointer to IGTL message new function and
    * the clone handler
    * \param messageTypeName The name of the message type
    * \param messageTypeNewPointer Function pointer to the message type new
    * function (e.g. (PointerToMessageBaseNew)&igtl::ImageMessage::New )
    * \param cloneHandler pointer to the message clone object
    */
    void AddMessageType(std::string messageTypeName,
               IGTLMessageFactory::PointerToMessageBaseNew messageTypeNewPointer,
               mitk::IGTLMessageCloneHandler::Pointer cloneHandler);

    /**
    * \brief Add message type name and pointer to IGTL message new function
    * Usage:
    * AddMessageType("IMAGE", (PointerToMessageBaseNew)&igtl::ImageMessage::New);
    * \param messageTypeName The name of the message type
    * \param messageTypeNewPointer Function pointer to the message type new
    * function (e.g. (PointerToMessageBaseNew)&igtl::ImageMessage::New )
    */
    virtual void AddMessageNewMethod(std::string messageTypeName,
              IGTLMessageFactory::PointerToMessageBaseNew messageTypeNewPointer);

    /**
    * \brief Get pointer to message type new function, or nullptr if the message
    * type not registered
    * Usage:
    * igtl::MessageBase::Pointer message = GetMessageTypeNewPointer("IMAGE")();
    */
    virtual IGTLMessageFactory::PointerToMessageBaseNew GetMessageTypeNewPointer(
        std::string messageTypeName);

    /**
    * \brief Creates a new message instance fitting to the given type.
    *
    * If this type is not registered it returns nullptr
    * Usage:
    * igtl::MessageBase::Pointer message = CreateInstance("IMAGE");
    */
    igtl::MessageBase::Pointer CreateInstance(std::string messageTypeName);

    /**
    * \brief Creates a new message according to the given header
    * \param msg The message header that defines the type of the message
    * This method checks the data type and creates a new message according to the
    * type.
    */
    igtl::MessageBase::Pointer CreateInstance(igtl::MessageHeader::Pointer msg);

    /**
    * \brief Adds a clone function for the specified message type
    * \param msgTypeName The name of the message type
    * \param msgCloneHandler Function pointer to the message clone function
    * (e.g. TBD )
    */
    virtual void AddMessageCloneHandler(std::string msgTypeName,
                      mitk::IGTLMessageCloneHandler::Pointer msgCloneHandler);

    /**
    * \brief Get pointer to message type clone function, or nullptr if the message
    * type is not registered
    * Usage:
    * igtl::TransformMessage::Pointer original = igtl::TransformMessage::New();
    * igtl::MessageBase::Pointer message =
    * GetCloneHandler("IMAGE")->Clone(original);
    */
    virtual mitk::IGTLMessageCloneHandler::Pointer
    GetCloneHandler(std::string messageTypeName);

    /**
    * \brief Clones the given message according to the available clone methods
    * \param msg The message that has to be cloned
    * This method checks the data type and clones the message according to this
    * type.
    */
    igtl::MessageBase::Pointer Clone(igtl::MessageBase::Pointer msg);

    /**
    * \brief Returns available get messages
    */
    std::list<std::string> GetAvailableMessageRequestTypes();

  protected:
    IGTLMessageFactory();
    ~IGTLMessageFactory() override;

    /**
     * \brief Map igt message types and the Clone() methods
    */
    std::map<std::string,mitk::IGTLMessageCloneHandler::Pointer> m_CloneHandlers;

    /**
     * \brief Map igt message types and the New() static methods of igtl::MessageBase
     * classes
    */
    std::map<std::string, PointerToMessageBaseNew> m_NewMethods;

  private:
    IGTLMessageFactory(const IGTLMessageFactory&);
  };
}

#endif
