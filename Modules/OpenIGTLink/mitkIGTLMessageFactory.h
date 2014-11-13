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

#ifndef MITKIGTLMESSAGEFACTORYH_HEADER_INCLUDED_
#define MITKIGTLMESSAGEFACTORYH_HEADER_INCLUDED_

#include "MitkOpenIGTLinkExports.h"
#include "mitkCommon.h"

#include "igtlMessageBase.h"
#include "igtlMessageHeader.h"

#include "mitkIGTLMessageCloneHandler.h"

namespace mitk {
  /**Documentation
  * \brief Factory class of supported OpenIGTLink message types
  *
  * This class is a factory class of supported OpenIGTLink message types to
  * localize the message creation code.
  *
  */
  class MITK_OPENIGTLINK_EXPORT IGTLMessageFactory : public itk::Object
  {
  public:
    mitkClassMacro(IGTLMessageFactory, itk::Object)
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);


//    virtual void PrintSelf(ostream& os);

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
    * \param msgClonePointer Function pointer to the message clone function
    * (e.g. TBD )
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
    * \brief Get pointer to message type new function, or NULL if the message
    * type not registered
    * Usage:
    * igtl::MessageBase::Pointer message = GetMessageTypeNewPointer("IMAGE")();
    */
    virtual IGTLMessageFactory::PointerToMessageBaseNew GetMessageTypeNewPointer(
        std::string messageTypeName);

    /**
    * \brief Creates a new message according to the given header
    * \param msg The message header that defines the type of the message
    * This method checks the data type and creates a new message according to the
    * type.
    */
    igtl::MessageBase::Pointer CreateInstance(igtl::MessageHeader::Pointer msg);

    /**
    * \brief Add a clone function for the specified message type
    * \param msgTypeName The name of the message type
    * \param msgClonePointer Function pointer to the message clone function
    * (e.g. TBD )
    */
    virtual void AddMessageCloneHandler(std::string msgTypeName,
                      mitk::IGTLMessageCloneHandler::Pointer msgCloneHandler);

    /**
    * \brief Get pointer to message type clone function, or NULL if the message type is
    * not registered
    * Usage: igtl::TransformMessage::Pointer original = igtl::TransformMessage::New();
    * igtl::MessageBase::Pointer message = GetMessageTypeClonePointer("IMAGE")(original);
    */
    virtual mitk::IGTLMessageCloneHandler::Pointer GetCloneHandler(std::string messageTypeName);

    /**
    * \brief Clones the given message according to the available clone methods
    * \param msg The message that has to be cloned
    * This method checks the data type and clones the message according to this
    * type.
    */
    igtl::MessageBase::Pointer Clone(igtl::MessageBase::Pointer msg);

    /*! Print all supported OpenIGTLink message types */
  //  virtual void PrintAvailableMessageTypes(ostream& os, vtkIndent indent);

    /*! Create a new igtl::MessageBase instance from message type, delete previous igtl::MessageBase if's not NULL */
  //  PlusStatus CreateInstance(const char* aIgtlMessageType, igtl::MessageBase::Pointer& aMessageBase);

    /*!
    Generate and pack IGTL messages from tracked frame
    \param packValidTransformsOnly Control whether or not to pack transform messages if they contain invalid transforms
    \param igtlMessageTypes List of message types to generate for a client
    \param igtMessages Output list for the generated IGTL messages
    \param trackedFrame Input tracked frame data used for IGTL message generation
    \param transformNames List of transform names to send
    \param imageTransformName Image transform name used in the IGTL image message
    \param transformRepository Transform repository used for computing the selected transforms
    */
  //  PlusStatus PackMessages(const std::vector<std::string>& igtlMessageTypes, std::vector<igtl::MessageBase::Pointer>& igtMessages, TrackedFrame& trackedFrame,
  //    std::vector<PlusTransformName>& transformNames, std::vector<PlusIgtlClientInfo::ImageStream>& imageStreams, bool packValidTransformsOnly, vtkTransformRepository* transformRepository=NULL);

    /*!
    * Returns available get messages
    */
    std::list<std::string> GetAvailableMessageRequestTypes();

  protected:
    IGTLMessageFactory();
    virtual ~IGTLMessageFactory();

    /**
     * Map igt message types and the Clone() methods
    */
    std::map<std::string,mitk::IGTLMessageCloneHandler::Pointer> m_CloneHandlers;

    /**
     * Map igt message types and the New() static methods of igtl::MessageBase
     * classes
    */
    std::map<std::string, PointerToMessageBaseNew> m_NewMethods;

  private:
    IGTLMessageFactory(const IGTLMessageFactory&);
  //  void operator=(const IGTLMessageFactory&);

  };
}

#endif
