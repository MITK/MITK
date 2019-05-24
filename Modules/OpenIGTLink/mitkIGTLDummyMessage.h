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

#ifndef MITKIGTLDUMMYMESSAGE_H
#define MITKIGTLDUMMYMESSAGE_H

#include "MitkOpenIGTLinkExports.h"

#include "igtlObject.h"
#include "igtlStringMessage.h"
#include "mitkIGTLMessageCloneHandler.h"

namespace mitk
{

/**
 * \class IGTLDummyMessage
 * \brief This class is a dummy message to show how to implement a new message type
*/
class MITKOPENIGTLINK_EXPORT IGTLDummyMessage : public igtl::StringMessage
{
public:
  typedef IGTLDummyMessage                     Self;
  typedef StringMessage                        Superclass;
  typedef igtl::SmartPointer<Self>             Pointer;
  typedef igtl::SmartPointer<const Self>       ConstPointer;

  igtlTypeMacro( mitk::IGTLDummyMessage, igtl::StringMessage )
  igtlNewMacro( mitk::IGTLDummyMessage )

public:
  /**
   * Set dummy string
   */
  void SetDummyString( const std::string& dummyString );

  /**
   * Get dummy string
   */
  std::string GetDummyString();

  /**
   * Returns a clone of itself
   */
  igtl::MessageBase::Pointer Clone() ;

protected:
  IGTLDummyMessage();
  ~IGTLDummyMessage() override;

  std::string m_dummyString;
};

mitkIGTMessageCloneClassMacro(IGTLDummyMessage, DummyMsgCloneHandler)

} // namespace mitk

#endif
