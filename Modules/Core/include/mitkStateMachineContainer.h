/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStateMachineContainer_h
#define mitkStateMachineContainer_h

#include <itkObject.h>
#include <iostream>
#include <vtkXMLParser.h>

#include <itkObjectFactory.h>
#include <mitkCommon.h>

#include <mitkStateMachineAction.h>
#include <mitkStateMachineState.h>
#include <mitkStateMachineTransition.h>

namespace us
{
  class Module;
}

namespace mitk
{
  /**
   * \brief Parses XML state machine descriptions and builds the state machine structure.
   *
   * Reads state machine pattern XML files from module resources, creates the
   * corresponding StateMachineState, StateMachineTransition, StateMachineAction, and
   * StateMachineCondition objects, and connects them into a complete state machine graph.
   *
   * \ingroup Interaction
   */
  class StateMachineContainer : public vtkXMLParser
  {
  public:
    static StateMachineContainer *New();
    vtkTypeMacro(StateMachineContainer, vtkXMLParser);

    /** \brief Type that holds all states of one state machine. */
    typedef std::vector<StateMachineState::Pointer> StateMachineCollectionType;

    /**
     * \brief Return the start state of the state machine.
     * \return Smart pointer to the start state, or nullptr if not found.
     */
    StateMachineState::Pointer GetStartState() const;

    /**
     * \brief Load a state machine pattern from an XML resource file.
     *
     * Loads an XML resource file in the given module context.
     * The files have to be placed in the Resources/Interaction folder of their
     * respective module.
     *
     * \param[in] fileName the name of the XML file to load (without path).
     * \param[in] module the module context to load the resource from. If nullptr,
     *            the current module context is used.
     * \return True if the file was parsed successfully without errors.
     * \throw mitk::Exception if the resource file cannot be found.
     */
    bool LoadBehavior(const std::string &fileName, const us::Module *module);

    /** \brief To enable InteractionStateMachine to access states. */
    friend class InteractionStateMachine;

  protected:
    StateMachineContainer();
    ~StateMachineContainer() override;

    /**
     * \brief Handle the start of an XML element during parsing.
     *
     * Overridden from vtkXMLParser.
     */
    void StartElement(const char *elementName, const char **atts) override;

    /**
     * \brief Handle the end of an XML element during parsing.
     *
     * Overridden from vtkXMLParser.
     */
    void EndElement(const char *elementName) override;

  private:
    /**
     * \brief Read a string attribute from the XML element attributes.
     *
     * \param[in] name the attribute name to look for.
     * \param[in] atts the null-terminated array of attribute name-value pairs.
     * \return The attribute value, or an empty string if not found.
     */
    std::string ReadXMLStringAttribut(std::string name, const char **atts);

    /**
     * \brief Read a boolean attribute from the XML element attributes.
     *
     * \param[in] name the attribute name to look for.
     * \param[in] atts the null-terminated array of attribute name-value pairs.
     * \return True if the attribute value is "TRUE" (case-insensitive), false otherwise.
     */
    bool ReadXMLBooleanAttribut(std::string name, const char **atts);

    /**
     * \brief Connect all state transitions by resolving state name references to actual state pointers.
     */
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

#endif
