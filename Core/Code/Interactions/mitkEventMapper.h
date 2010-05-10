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


#ifndef EVENTMAPPER_H_HEADER_INCLUDED
#define EVENTMAPPER_H_HEADER_INCLUDED

#include "mitkEvent.h"
#include "mitkCommon.h"
#include "mitkEventDescription.h"
#include <vtkXMLParser.h>


namespace mitk {
  struct ltstr
  {
    bool operator()(const char* s1, const char* s2) const
    {
      return strcmp(s1, s2) < 0;
    }
  };
  typedef std::string msgString;
  //for friendship wor to set the stateevent after calculating
  class GlobalInteraction;
  class StateMachine;
  class StateEvent;

  //##Documentation
  //## @brief Maps an Event to its description
  //##
  //## EventMapping:
  //## This class mapps the Events, usually given by the OS or here by QT, to a MITK internal EventId.
  //## It loads all information from the xml-file (possible, understandable Events with the mitkEventID).
  //## If an event appears, the method MapEvent is called with the event params.
  //## This Method looks up the event params, and tries to find an mitkEventId to it.
  //## If yes, then sends the event and the found ID to the globalStateMachine, which handles all
  //## further operations of that event.
  //## For Undo-Mechanism a statechanging StateMachine::HandleEvent is connected to an ObjectEventID and an GroupEventId.
  //## That way a fine an raw Undo is possible (fine for ObjectID by ObjectID, raw for GroupID for GroupID)
  //## Here the ObjectEventID gets increased,
  //## not the GroupEventId(must get increased by a StateMachine, that has the information when a new Group of operation starts)
  //## @ingroup Interaction
  class MITK_CORE_EXPORT EventMapper : public vtkXMLParser
  {
  public:
    static EventMapper *New();
    vtkTypeMacro(EventMapper,vtkXMLParser);

    typedef std::vector<mitk::EventDescription> EventDescriptionVec;
    typedef std::vector<mitk::EventDescription>::iterator EventDescriptionVecIter;

    typedef std::map<const char*, int, ltstr> ConstMap;
    typedef std::map<const char*, int, ltstr>::iterator ConstMapIter;

    //##Documentation
    //## searches the Event in m_EventDescription
    //## and if included transmits the event to globalInteraction
    //## the optional parameter should be used in a conference to avoid a
    //## feedback
    static bool MapEvent(Event* event, int mitkPostedEventID=0 );
    
    //##Documentation
    //## maps the Event in m_EventDescription with the ID
    //## and if found returns the Id,
    //## if not found it returns -1
    static StateEvent* RefreshStateEvent(StateEvent* stateEvent);

    //##Documentation
    //## loads an XML-File containing Events into m_EventDescriptions
    //## also involved: EventMapper::startEvent(...)
    bool LoadBehavior(std::string fileName);

    //##Documentation
    //## loads Events into m_EventDescriptions from xml string
    //## also involved: EventMapper::startEvent(...)
    bool LoadBehaviorString(std::string xmlString);

    //##Documentation
    //## Try to load standard behavior file "StateMachine.xml"
    //##
    //## Search strategy:
    //## \li try environment variable "MITKCONF" (path to "StateMachine.xml")
    //## \li try "./StateMachine.xml"
    //## \li try via source directory (using MITKROOT from cmake-created
    //## mitkConfig.h) "MITKROOT/Interactions/mitkBaseInteraction/StateMachine.xml"
    bool LoadStandardBehavior();

    //##Documentation
    //## reads a Tag from an XML-file
    //## adds Events to m_EventDescription

    std::string GetStyleName();

    //friendship because of SetStateEvent for computing WorldCoordinates
    friend class mitk::GlobalInteraction;

  protected:
    EventMapper();
    ~EventMapper();

    //##Documentation
    //##@brief method only for GlobalInteraction to change the Event (from DiplayPositionEvent to PositionEvent)
    static void SetStateEvent(Event* event);

  private:

    //##Documentation
    //## @brief method used in XLM-Reading; gets called when a start-tag is read
    void  StartElement (const char *elementName, const char **atts);

    //##Documentation
    //## @brief reads an XML-String-Attribute
    std::string ReadXMLStringAttribut( std::string name, const char** atts);

    //##Documentation
    //## @brief reads an XML-Integer-Attribute
    int ReadXMLIntegerAttribut( std::string name, const char** atts );

    //##Documentation
    //## @brief converts the strings given by the XML-Behaviour-File to int
    inline int convertConstString2ConstInt(std::string input);
//    static std::string Convert2String(int input);
//    static std::string Convert2String(double input);
//    static std::string Convert2String(float input);

    //##Documentation
    //## @brief maps the strings to int for convertion from XML-Behaviour-File
    ConstMap m_EventConstMap;

    //##Documentation
    //## @brief stores the information for the connection between QT-Events and the internal EventId.
    //## gets this information from xml-File
    static EventDescriptionVec m_EventDescriptions;

    static StateEvent m_StateEvent;

    //##Documentation
    //## @brief stores the name of the Event-Style loaded
    static std::string m_StyleName;

    static const std::string STYLE;
    static const std::string NAME;
    static const std::string ID;
    static const std::string TYPE;
    static const std::string BUTTON;
    static const std::string BUTTONSTATE;
    static const std::string KEY;
    static const std::string EVENTS;
    static const std::string EVENT;

  };
} // namespace mitk

#endif /* EVENTMAPPER_H_HEADER_INCLUDED_C187864A */


