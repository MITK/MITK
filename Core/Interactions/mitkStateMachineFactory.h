/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD
#define STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD

#include "mitkCommon.h"
#include "mitkState.h"
#include <vtkXMLParser.h> 
#include <iostream>


namespace mitk {

//##ModelId=3E5A39550068
//##Documentation
//## @brief builds up all specifiyed statemachines and hold them for later
//## access
//## @ingroup Interaction
//## According to the XML-File every different statemachine is build up. A new
//## Instance of a new StateMachine grabs a StartState of one certain
//## statemachine. Two instances of one kind of statemachine share that
//## statemachine. 
//## During Build-Process each statemachine is paresed for well formed  Style.
//## No changes are to ber done after the Build-Process.
	class StateMachineFactory : public vtkXMLParser
{
  public:
	//##ModelId=3F0177090046
	typedef std::map<std::string,mitk::State*> StartStateMap;
	//##ModelId=3F0177090056
	typedef std::map<std::string,mitk::State *>::iterator StartStateMapIter;
	//##ModelId=3F0177090075
	typedef std::set<int> HistorySet;
	//##ModelId=3F0177090094
	typedef std::set<int>::iterator HistorySetIter;

    //##ModelId=3E68B2C600BD
	  StateMachineFactory();

    //##ModelId=3E5B4144024F
	//##Documentation
	//## returns NULL if no entry with string type is found
	  static State* GetStartState(const char * type);

    //##ModelId=3E5B41730261
	//##Documentation
	//##loads the xml file filename and generates the necessary instances
	  static bool LoadBehavior(std::string fileName);

    //##ModelId=3E6773790098
    void  StartElement (const char *elementName, const char **atts);

    void  EndElement (const char *elementName);

  private:

  std::string ReadXMLStringAttribut( std::string name, const char** atts);
  float ReadXMLFloatAttribut( std::string name, const char** atts );
  double ReadXMLDoubleAttribut( std::string name, const char** atts );
  int ReadXMLIntegerAttribut( std::string name, const char** atts );
  bool ReadXMLBooleanAttribut( std::string name, const char** atts );
    //##ModelId=3E5B428F010B
	//##Documentation
	//## sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
	  static bool ConnectStates(mitk::State::StateMap *states);

    //##ModelId=3E77572A010E
	//##Documentation
	//## recusive method, that parses this brand of 
	//## the stateMachine and returns if correct
	  static bool parse(mitk::State::StateMap *states, mitk::State::StateMapIter thisState, HistorySet *history);

	//##ModelId=3E5B423003DF
	  static StartStateMap m_StartStates;

    //##ModelId=3E68C269032E
	  mitk::State::StateMap m_AllStates;

    //##ModelId=3E6773290108
	  State* m_AktState;

    Transition* m_AktTransition;

    Action* m_AktAction;
    

    //##ModelId=3E68B2C60040
	  std::string m_AktStateMachineName;

    //##ModelId=3E7757280322
	  static const std::string STYLE;
    //##ModelId=3E775728037F
	  static const std::string NAME;
    //##ModelId=3E77572803DD
	  static const std::string ID;	  
    //##ModelId=3E7757290053
	  static const std::string START_STATE;
    //##ModelId=3E77572900B1
	  static const std::string NEXT_STATE_ID;
    //##ModelId=3E775729010E
	  static const std::string EVENT_ID;
    //##ModelId=3E775729017C
	  static const std::string SIDE_EFFECT_ID;
    //##ModelId=3E7F18FF0131
	  static const std::string ISTRUE;
    //##ModelId=3E7F18FF01FD
	  static const std::string ISFALSE;

    static const std::string STATE_MACHIN;

    static const std::string TRANSITION;    

    static const std::string STATE;

    static const std::string STATE_MACHIN_NAME;

    static const std::string ACTION;

    static const std::string BOOL_PARAMETER;

    static const std::string INT_PARAMETER;

    static const std::string FLOAT_PARAMETER;

    static const std::string DOUBLE_PARAMETER;

    static const std::string STRING_PARAMETER;

    static const std::string VALUE;

};

} // namespace mitk


#endif /* STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD */
