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


#ifndef EVENTDESCRIPTION_H_HEADER_INCLUDED_C188FC4D
#define EVENTDESCRIPTION_H_HEADER_INCLUDED_C188FC4D

#include <MitkExports.h>
#include "mitkEvent.h"
#include <string>

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4251)
#endif

namespace mitk {

  //##Documentation
  //## @brief adds additional Information (Name and EventID) to an Event
  //##
  //## A name and an ID is added to the information of an event, so the event can
  //## be processed futher on.
  //## @ingroup Interaction
  class MITK_CORE_EXPORT EventDescription : public Event
  {
  public:
    EventDescription(int type, int button, int buttonState,int key, std::string name, int id);

    std::string GetName() const;

    int GetId() const;

  private:
    std::string m_Name;

    int m_Id;
  };

} // namespace mitk

#ifdef _MSC_VER
# pragma warning(pop)
#endif


#endif /* EVENTDESCRIPTION_H_HEADER_INCLUDED_C188FC4D */
