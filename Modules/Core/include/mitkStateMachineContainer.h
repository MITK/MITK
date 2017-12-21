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

#ifndef STATEMACHINECONTAINER_H_HEADER_INCLUDED_C19AEDDD
#define STATEMACHINECONTAINER_H_HEADER_INCLUDED_C19AEDDD

#include "itkObject.h"
#include <iostream>
#include <vtkXMLParser.h>

#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "mitkStateMachineAction.h"
#include "mitkStateMachineState.h"
#include "mitkStateMachineTransition.h"

namespace us
{
  class Module;
}

namespace mitk
{
  /**
   *@brief
   *
   * @ingroup Interaction
   **/
  class StateMachineContainer : public vtkXMLParser
  {
  public:
    static StateMachineContainer *New();
    vtkTypeMacro(StateMachineContainer, vtkXMLParser);

    /**
    * @brief This type holds all states of one statemachine.
    **/
    typedef std::vector<StateMachineState::Pointer> StateMachineCollectionType;

    /**
    * @brief Returns the StartState of the StateMachine.
    **/
    StateMachineState::Pointer GetStartState() const;

    /**
    * @brief Loads XML resource
    *
    * Loads a XML resource file in the given module context.
    * The files have to be placed in the Resources/Interaction folder of their respective module.
    **/
    bool LoadBehavior(const std::string &fileName, const us::Module *module);

    /**
    * brief To enable StateMachine to access states
    **/
    friend class InteractionStateMachine;

  protected:
    StateMachineContainer();
    ~StateMachineContainer() override;

    /**
    * @brief Derived from XMLReader
    **/
    void StartElement(const char *elementName, const char **atts) override;
    /**
    * @brief Derived from XMLReader
    **/
    void EndElement(const char *elementName) override;

  private:
    /**
    * @brief Derived from XMLReader
    **/
    std::string ReadXMLStringAttribut(std::string name, const char **atts);
    /**
    * @brief Derived from XMLReader
    **/
    bool ReadXMLBooleanAttribut(std::string name, const char **atts);

    /**
    * @brief Sets the pointers in Transition (setNextState(..)) according to the extracted xml-file content
    **/
    void ConnectStates();

    StateMachineState::Pointer m_StartState;
    StateMachineState::Pointer m_CurrState;
    StateMachineTransition::Pointer m_CurrTransition;

    StateMachineCollectionType m_States;
    bool m_StartStateFound;
    bool
      m_errors; // use member, because of inheritance from vtkXMLParser we can't return a success value for parsing the
                // file.
    std::string m_Filename; // store file name for debug purposes.
  };

} // namespace mitk

#endif /* STATEMACHINECONTAINER_H_HEADER_INCLUDED_C19AEDDD */
