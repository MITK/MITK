#ifndef STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD
#define STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD

#include "mitkCommon.h"
#include "State.h"
#include "mitkInteractionConst.h"
#include <qxml.h>
#include <iostream>


//##ModelId=3E775728016C
typedef std::map<std::string,mitk::State*> StartStateMap;
//##ModelId=3E775728017C
typedef std::map<std::string,mitk::State *>::iterator StartStateMapIter;

//##ModelId=3E775728014D
typedef std::set<int> HistorySet;
//##ModelId=3E775728015C
typedef std::set<int>::iterator HistorySetIter;


namespace mitk {

//##ModelId=3E5A39550068
//##Documentation
//## a xml file including the states and the transitions is read.
//## according to the structur all States are generated.
//## Then all transitions are generated and the pointers are set
//## (Transition::setNextState(State* tState))
	class StateMachineFactory : public QXmlDefaultHandler
{
  public:
    //##ModelId=3E68B2C600BD
	  StateMachineFactory();

    //##ModelId=3E5B4144024F
	//##Documentation
	//## returns NULL if no entry with string type is found
	  static State* GetStartState(std::string type);

    //##ModelId=3E5B41730261
	//##Documentation
	//##loads the xml file filename and generates the necessary instances
	  static bool LoadBehavior(std::string fileName);

    //##ModelId=3E6773790098
	  bool startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& atts );

    //##ModelId=3E6907B40180
	  bool endElement( const QString&, const QString&, const QString & qName );
		  

  private:
    //##ModelId=3E5B428F010B
	//##Documentation
	//## sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
	  static bool ConnectStates(StateMap *states);

    //##ModelId=3E77572A010E
	//##Documentation
	//## recusive method, that parses this brand of 
	//## the stateMachine and returns if correct
	  static bool parse(StateMap *states, StateMapIter thisState, HistorySet *history);

	//##ModelId=3E5B423003DF
	  static StartStateMap m_StartStates;

    //##ModelId=3E68C269032E
	  StateMap m_AllStates;

    //##ModelId=3E6773290108
	  State* m_AktState;

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
};

} // namespace mitk


#endif /* STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD */