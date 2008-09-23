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


#ifndef STATEEVENT_H_HEADER_INCLUDED_C188E5BF
#define STATEEVENT_H_HEADER_INCLUDED_C188E5BF

#include "mitkCommon.h"

namespace mitk {

  class Event;
  //##Documentation
  //## @brief Class holding an mitk-event and the mitk-event-number for a statechange
  //##
  //## Holds an event, with which a statechange of a statemachine shall be
  //## done. iD represents the mitk-event-number, event all further necessary information like
  //## the MousePosition or a key.
  //## Not derived from event to hold only one object stateevent, pass it to the statemachines, 
  //## set the next event and reuse this object
  //## @ingroup Interaction
  class MITK_CORE_EXPORT StateEvent
  {
  public:
    StateEvent();
    //##Documentation
    //## @brief Constructor
    //## @param id: mitk internal EventID
    //## @param event: the information about the appeared event
    StateEvent(int id, Event const* event = NULL );

    ~StateEvent();

    //##Documentation
    //## @brief to set the params and reuse an object
    void Set(int id, Event const* event);

    int GetId() const;

    mitk::Event const* GetEvent() const;

  private:
    int m_Id;

    mitk::Event const* m_Event;

  };

} // namespace mitk



#endif /* STATEEVENT_H_HEADER_INCLUDED_C188E5BF */
