/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIGTLDummyMessage_h
#define mitkIGTLDummyMessage_h

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
  igtlTypeMacro(mitk::IGTLDummyMessage, igtl::StringMessage);
  igtlNewMacro(mitk::IGTLDummyMessage);

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
  igtl::MessageBase::Pointer Clone() override ;

protected:
  IGTLDummyMessage();
  ~IGTLDummyMessage() override;

  std::string m_dummyString;
};

mitkIGTMessageCloneClassMacro(IGTLDummyMessage, DummyMsgCloneHandler);

} // namespace mitk

#endif
