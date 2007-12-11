/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef EVENTDESCRIPTION_H_HEADER_INCLUDED_C188FC4D
#define EVENTDESCRIPTION_H_HEADER_INCLUDED_C188FC4D

#include "mitkCommon.h"
#include "mitkEvent.h"
#include <string>

namespace mitk {

  //##ModelId=3E5A3910009B
  //##Documentation
  //## @brief adds additional Information (Name and EventID) to an Event
  //##
  //## A name and an ID is added to the information of an event, so the event can
  //## be processed futher on.
  //## @ingroup Interaction
  class EventDescription : public Event
  {
  public:
    //##ModelId=3E5B30A30095
    EventDescription(int type, int button, int buttonState,int key, std::string name, int id);

    //##ModelId=3E5B3103030A
    std::string GetName() const;

    //##ModelId=3E5B3132027C
    int GetId() const;

  private:
    //##ModelId=3E5B2F66020C
    std::string m_Name;

    //##ModelId=3E5B2F660220
    int m_Id;
  };

} // namespace mitk



#endif /* EVENTDESCRIPTION_H_HEADER_INCLUDED_C188FC4D */
