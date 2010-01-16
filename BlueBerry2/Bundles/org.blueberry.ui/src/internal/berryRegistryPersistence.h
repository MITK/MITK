/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYREGISTRYPERSISTENCE_H_
#define BERRYREGISTRYPERSISTENCE_H_

#include "berryWorkbenchRegistryConstants.h"
#include "../services/berryIDisposable.h"
#include "../commands/berryICommandService.h"


#include <berryExpression.h>
#include <berryCommand.h>
#include <berryParameterizedCommand.h>
#include <berryIStatus.h>
#include <berryIConfigurationElement.h>

namespace berry {

/**
 * <p>
 * A manager for items parsed from the registry. This attaches a listener to the
 * registry after the first read, and monitors the registry for changes from
 * that point on. When {@link #Dispose()} is called, the listener is detached.
 * </p>
 * <p>
 * This class is only intended for internal use within the
 * <code>org.blueberry.ui</code> plug-in.
 * </p>
 *
 * @since 3.2
 */
class RegistryPersistence : public WorkbenchRegistryConstants , public IDisposable {

public:

  /**
   * Detaches the registry change listener from the registry.
   */
  void Dispose();


protected:

  /**
   * The expression to return when there is an error. Never <code>null</code>.
   */
  static Expression::Pointer ERROR_EXPRESSION;

  /**
   * Adds a warning to be logged at some later point in time. This logs the
   * identifier of the item, as well as an extra attribute.
   *
   * @param warningsToLog
   *            The collection of warnings to be logged; must not be
   *            <code>null</code>.
   * @param message
   *            The mesaage to log; must not be <code>null</code>.
   * @param element
   *            The element from which the warning originates; may be
   *            <code>null</code>.
   * @param id
   *            The identifier of the item for which a warning is being
   *            logged; may be <code>null</code>.
   * @param extraAttributeName
   *            The name of extra attribute to be logged; may be
   *            <code>null</code>.
   * @param extraAttributeValue
   *            The value of the extra attribute to be logged; may be
   *            <code>null</code>.
   */
  static void AddWarning(std::vector<IStatus::Pointer>& warningsToLog,
      const std::string& message, const IConfigurationElement::ConstPointer element,
      const std::string& id = "", const std::string& extraAttributeName = "",
      const std::string& extraAttributeValue = "");

  /**
   * Checks that the class attribute or element exists for this element. This
   * is used for executable extensions that are being read in.
   *
   * @param configurationElement
   *            The configuration element which should contain a class
   *            attribute or a class child element; must not be
   *            <code>null</code>.
   * @param warningsToLog
   *            The list of warnings to be logged; never <code>null</code>.
   * @param message
   *            The message to log if something goes wrong; may be
   *            <code>null</code>.
   * @param id
   *            The identifier of the handle object; may be <code>null</code>.
   * @return <code>true</code> if the class attribute or element exists;
   *         <code>false</code> otherwise.
   */
  static bool CheckClass(
      const IConfigurationElement::ConstPointer configurationElement,
      std::vector<IStatus::Pointer>& warningsToLog,
      const std::string& message,
      const std::string& id);

  /**
   * Checks to see whether the configuration element represents a pulldown
   * action. This involves reading the <code>style</code> and
   * <code>pulldown</code> attributes.
   *
   * @param element
   *            The element to check; must not be <code>null</code>.
   * @return <code>true</code> if the element is a pulldown action;
   *         <code>false</code> otherwise.
   */
  static bool IsPulldown(
      const IConfigurationElement::ConstPointer element);

  /**
   * Logs any warnings in <code>warningsToLog</code>.
   *
   * @param warningsToLog
   *            The warnings to log; may be <code>null</code>.
   * @param message
   *            The message to include in the log entry; must not be
   *            <code>null</code>.
   */
  static void LogWarnings(const std::vector<IStatus::Pointer>& warningsToLog,
      const std::string& message);

  /**
   * Reads a boolean attribute from an element.
   *
   * @param configurationElement
   *            The configuration element from which to read the attribute;
   *            must not be <code>null</code>.
   * @param attribute
   *            The attribute to read; must not be <code>null</code>.
   * @param defaultValue
   *            The default boolean value.
   * @return The attribute's value; may be <code>null</code> if none.
   */
  static bool ReadBoolean(
     const IConfigurationElement::ConstPointer configurationElement,
     const std::string& attribute,
     bool defaultValue);

  /**
   * Reads an optional attribute from an element.
   *
   * @param configurationElement
   *            The configuration element from which to read the attribute;
   *            must not be <code>null</code>.
   * @param attribute
   *            The attribute to read; must not be <code>null</code>.
   * @param value
   *            The variable where the value will be filled in.
   * @return <code>true</code> if the attribute contains a non-empty value, <code>false</code> else.
   */
  static bool ReadOptional(
      const IConfigurationElement::ConstPointer configurationElement,
      const std::string& attribute, std::string& value);

  /**
   * Reads the parameterized command from a parent configuration element. This
   * is used to read the parameter sub-elements from a key element, as well as
   * the command id. Each parameter is guaranteed to be valid. If invalid
   * parameters are found, then a warning status will be appended to the
   * <code>warningsToLog</code> list. The command id is required, or a
   * warning will be logged.
   *
   * @param configurationElement
   *            The configuration element from which the parameters should be
   *            read; must not be <code>null</code>.
   * @param commandService
   *            The service providing commands for the workbench; must not be
   *            <code>null</code>.
   * @param warningsToLog
   *            The list of warnings found during parsing. Warnings found will
   *            parsing the parameters will be appended to this list. This
   *            value must not be <code>null</code>.
   * @param message
   *            The message to print if the command identifier is not present;
   *            must not be <code>null</code>.
   * @return The array of parameters found for this configuration element;
   *         <code>null</code> if none can be found.
   */
  static ParameterizedCommand::Pointer ReadParameterizedCommand(
      const IConfigurationElement::Pointer configurationElement,
      ICommandService::Pointer commandService,
      std::vector<IStatus::Pointer>& warningsToLog,
      const std::string& message,
      const std::string& id);

  /**
   * Reads the parameters from a parent configuration element. This is used to
   * read the parameter sub-elements from a key element. Each parameter is
   * guaranteed to be valid. If invalid parameters are found, then a warning
   * status will be appended to the <code>warningsToLog</code> list.
   *
   * @param configurationElement
   *            The configuration element from which the parameters should be
   *            read; must not be <code>null</code>.
   * @param warningsToLog
   *            The list of warnings found during parsing. Warnings found will
   *            parsing the parameters will be appended to this list. This
   *            value must not be <code>null</code>.
   * @param command
   *            The command around which the parameterization should be
   *            created; must not be <code>null</code>.
   * @return The array of parameters found for this configuration element;
   *         <code>null</code> if none can be found.
   */
  static ParameterizedCommand::Pointer ReadParameters(
      const IConfigurationElement::Pointer configurationElement,
      std::vector<IStatus::Pointer>& warningsToLog,
      Command::Pointer command);


  /**
   * Reads a required attribute from the configuration element. This logs the
   * identifier of the item if this required element cannot be found.
   *
   * @param configurationElement
   *            The configuration element from which to read; must not be
   *            <code>null</code>.
   * @param attribute
   *            The attribute to read; must not be <code>null</code>.
   * @param value
   *            The required attribute value.
   * @param warningsToLog
   *            The list of warnings; must not be <code>null</code>.
   * @param message
   *            The warning message to use if the attribute is missing; must
   *            not be <code>null</code>.
   * @param id
   *            The identifier of the element for which this is a required
   *            attribute; may be empty.
   * @return <code>true</code> if the attribute was found, <code>false</code> else.
   */
  static bool ReadRequired(
      const IConfigurationElement::ConstPointer configurationElement,
      const std::string& attribute,
      std::string& value,
      std::vector<IStatus::Pointer>& warningsToLog,
      const std::string& message,
      const std::string& id = "");

  /**
   * Reads a <code>when</code> child element from the given configuration
   * element. Warnings will be appended to <code>warningsToLog</code>.
   *
   * @param parentElement
   *            The configuration element which might have a <code>when</code>
   *            element as a child; never <code>null</code>.
   * @param whenElementName
   *            The name of the when element to find; never <code>null</code>.
   * @param id
   *            The identifier of the menu element whose <code>when</code>
   *            expression is being read; never <code>null</code>.
   * @param warningsToLog
   *            The list of warnings while parsing the extension point; never
   *            <code>null</code>.
   * @return The <code>when</code> expression for the
   *         <code>configurationElement</code>, if any; otherwise,
   *         <code>null</code>.
   */
  static Expression::Pointer ReadWhenElement(
      const IConfigurationElement::Pointer parentElement,
      const std::string& whenElementName,
      const std::string& id,
      std::vector<IStatus::Pointer>& warningsToLog);

  /**
   * Constructs a new instance of {@link RegistryPersistence}. A registry
   * change listener is created.
   */
  RegistryPersistence();

   /**
   * Checks whether the registry change could affect this persistence class.
   *
   * @param event
   *            The event indicating the registry change; must not be
   *            <code>null</code>.
   * @return <code>true</code> if the persistence instance is affected by
   *         this change; <code>false</code> otherwise.
   */
  //virtual bool IsChangeImportant(IRegistryChangeEvent::Pointer event) = 0;

  /**
   * Reads the various elements from the registry. Subclasses should extend,
   * but must not override.
   */
  virtual void Read();


  /**
   * Whether the preference and registry change listeners have been attached
   * yet.
   */
  bool registryListenerAttached;


private:

  /**
   * The registry change listener for this class.
   */
  //IRegistryChangeListener::Pointer registryChangeListener;

};

}

#endif /* BERRYREGISTRYPERSISTENCE_H_ */
