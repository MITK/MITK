#ifndef STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD
#define STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD

// #include "mitkCommon.h"
#include "State.h"
#include <qxml.h>

typedef std::map<std::string,mitk::State*> StartStateMap;
typedef std::map<std::string,mitk::State *>::iterator StartStateMapIter;

/*typedef std::map<int,mitk::State *> StateMap;
typedef std::map<int,mitk::Transition> TransitionMap;

typedef std::map<int,mitk::State *>::iterator StateMapIter;
typedef std::map<int,mitk::Transition>::iterator TransMapIter;
*/

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

	  bool startDocument(){
	  
		  return TRUE;
	  }

	  bool endDocument() {
	  
		  return true;
	  }

    //##ModelId=3E6773790098
	  bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts );

    //##ModelId=3E6907B40180
	  bool endElement( const QString&, const QString&, const QString & qName );
		  

  private:
    //##ModelId=3E5B428F010B
	//##Documentation
	//## sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
	  static bool ConnectStates(StateMap *states);

	//##ModelId=3E5B423003DF
	  static StartStateMap m_StartStates;

    //##ModelId=3E68C269032E
	  StateMap m_AllStates;

    //##ModelId=3E6773290108
	  State* m_AktState;

    //##ModelId=3E68B2C60040
	  std::string m_AktStateMachineName;

	  static const std::string STYLE;
	  static const std::string NAME;
	  static const std::string ID;	  
	  static const std::string START_STATE;
	  static const std::string NEXT_STATE_ID;
	  static const std::string EVENT_ID;
	  static const std::string SIDE_EFFECT_ID;
	  
	  static const std::string TYPE;
	  static const std::string BUTTON_NUMBER;
	  static const std::string KEY;
	  
};

} // namespace mitk


#endif /* STATEMACHINEFACTORY_H_HEADER_INCLUDED_C19AEDDD */