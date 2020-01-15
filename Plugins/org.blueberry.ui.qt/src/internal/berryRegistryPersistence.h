/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREGISTRYPERSISTENCE_H
#define BERRYREGISTRYPERSISTENCE_H

#include "berryWorkbenchRegistryConstants.h"
#include "berryIRegistryEventListener.h"

#include <QScopedPointer>

namespace berry {

template<class T> class SmartPointer;

class Command;
class Expression;
class ParameterizedCommand;

struct ICommandService;
struct IConfigurationElement;
struct IExtension;
struct IExtensionPoint;
struct IRegistryEventListener;
struct IStatus;

/**
 * <p>
 * A manager for items parsed from the registry. This attaches a listener to the
 * registry after the first read, and monitors the registry for changes from
 * that point on. When the destructor is called, the listener is detached.
 * </p>
 * <p>
 * This class is only intended for internal use within the
 * <code>org.blueberry.ui.qt</code> plug-in.
 * </p>
 */
class RegistryPersistence : public WorkbenchRegistryConstants, private IRegistryEventListener
{

protected:

  /**
   * The expression to return when there is an error. Never <code>null</code>.
   */
  static const SmartPointer<Expression> ERROR_EXPRESSION;

  /**
   * Adds a warning to be logged at some later point in time.
   *
   * @param warningsToLog
   *            The collection of warnings to be logged; must not be
   *            <code>null</code>.
   * @param message
   *            The mesaage to log; must not be <code>null</code>.
   * @param element
   *            The element from which the warning originates; may be
   *            <code>null</code>.
   */
  static void AddWarning(QList<SmartPointer<IStatus> >& warningsToLog,
                         const QString& message,
                         const SmartPointer<IConfigurationElement>& element);

  /**
   * Adds a warning to be logged at some later point in time. This logs the
   * identifier of the item.
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
   */
  static void AddWarning(QList<SmartPointer<IStatus> >& warningsToLog,
                         const QString& message,
                         const SmartPointer<IConfigurationElement>& element,
                         const QString& id);

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
  static void AddWarning(QList<SmartPointer<IStatus> >& warningsToLog,
                         const QString& message,
                         const SmartPointer<IConfigurationElement>& element,
                         const QString& id,
                         const QString& extraAttributeName,
                         const QString& extraAttributeValue);

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
  static bool CheckClass(const SmartPointer<IConfigurationElement>& configurationElement,
                         QList<SmartPointer<IStatus> >& warningsToLog,
                         const QString& message,
                         const QString& id);

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
  static bool IsPulldown(const SmartPointer<IConfigurationElement>& element);

  /**
   * Logs any warnings in <code>warningsToLog</code>.
   *
   * @param warningsToLog
   *            The warnings to log; may be <code>null</code>.
   * @param message
   *            The message to include in the log entry; must not be
   *            <code>null</code>.
   */
  static void LogWarnings(QList<SmartPointer<IStatus> >& warningsToLog,
                          const QString& message);

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
  static bool ReadBoolean(const SmartPointer<IConfigurationElement>& configurationElement,
                          const QString& attribute,
                          const bool defaultValue);

  /**
   * Reads an optional attribute from an element. This converts zero-length
   * strings into <code>null</code>.
   *
   * @param configurationElement
   *            The configuration element from which to read the attribute;
   *            must not be <code>null</code>.
   * @param attribute
   *            The attribute to read; must not be <code>null</code>.
   * @return The attribute's value; may be <code>null</code> if none.
   */
  static QString ReadOptional(const SmartPointer<IConfigurationElement>& configurationElement,
                              const QString& attribute);

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
  static SmartPointer<ParameterizedCommand> ReadParameterizedCommand(
      const SmartPointer<IConfigurationElement>& configurationElement,
      ICommandService* const commandService,
      QList<SmartPointer<IStatus> >& warningsToLog,
      const QString& message, const QString& id);

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
  static SmartPointer<ParameterizedCommand> ReadParameters(
      const SmartPointer<IConfigurationElement>& configurationElement,
      QList<SmartPointer<IStatus> >& warningsToLog,
      const SmartPointer<Command>& command);

  /**
   * Reads a required attribute from the configuration element.
   *
   * @param configurationElement
   *            The configuration element from which to read; must not be
   *            <code>null</code>.
   * @param attribute
   *            The attribute to read; must not be <code>null</code>.
   * @param warningsToLog
   *            The list of warnings; must not be <code>null</code>.
   * @param message
   *            The warning message to use if the attribute is missing; must
   *            not be <code>null</code>.
   * @return The required attribute; may be <code>null</code> if missing.
   */
  static QString ReadRequired(const SmartPointer<IConfigurationElement>& configurationElement,
                              const QString& attribute,
                              QList<SmartPointer<IStatus> >& warningsToLog,
                              const QString& message);

  /**
   * Reads a required attribute from the configuration element. This logs the
   * identifier of the item if this required element cannot be found.
   *
   * @param configurationElement
   *            The configuration element from which to read; must not be
   *            <code>null</code>.
   * @param attribute
   *            The attribute to read; must not be <code>null</code>.
   * @param warningsToLog
   *            The list of warnings; must not be <code>null</code>.
   * @param message
   *            The warning message to use if the attribute is missing; must
   *            not be <code>null</code>.
   * @param id
   *            The identifier of the element for which this is a required
   *            attribute; may be <code>null</code>.
   * @return The required attribute; may be <code>null</code> if missing.
   */
  static QString ReadRequired(const SmartPointer<IConfigurationElement>& configurationElement,
                              const QString& attribute,
                              QList<SmartPointer<IStatus> >& warningsToLog,
                              const QString& message, const QString& id);

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
  static SmartPointer<Expression> ReadWhenElement(
      const SmartPointer<IConfigurationElement>& parentElement,
      const QString& whenElementName, const QString& id,
      QList<SmartPointer<IStatus> >& warningsToLog);

  /**
   * Whether the preference and registry change listeners have been attached
   * yet.
   */
  bool registryListenerAttached;

  /**
   * Constructs a new instance of {@link RegistryPersistence}. A registry
   * change listener is created.
   */
  RegistryPersistence();

  /**
   * Detaches the registry change listener from the registry.
   */
  ~RegistryPersistence() override;

  enum RegistryChangeType {
    OBJECTS_ADDED,
    OBJECTS_REMOVED
  };

  /**
   * Checks whether the registry change could affect this persistence class.
   *
   * @param extension
   *            The added or removed extensions.
   * @return <code>true</code> if the persistence instance is affected by
   *         this change; <code>false</code> otherwise.
   */
  virtual bool IsChangeImportant(const QList<SmartPointer<IExtension> >& extensions,
                                 RegistryChangeType changeType) = 0;

  /**
   * Checks whether the registry change could affect this persistence class.
   *
   * @param extensionPoints
   *            The added or removed extension points.
   * @return <code>true</code> if the persistence instance is affected by
   *         this change; <code>false</code> otherwise.
   */
  virtual bool IsChangeImportant(const QList<SmartPointer<IExtensionPoint> >& extensionPoints,
                                 RegistryChangeType changeType) = 0;

  /**
   * Reads the various elements from the registry. Subclasses should extend,
   * but must not override.
   */
  virtual void Read();

private:

  class ReadRunnable;

  friend class ReadRunnable;

  void Added(const QList<SmartPointer<IExtension> >& extensions) override;

  void Removed(const QList<SmartPointer<IExtension> >& extensions) override;

  void Added(const QList<SmartPointer<IExtensionPoint> >& extensionPoints) override;

  void Removed(const QList<SmartPointer<IExtensionPoint> >& extensionPoints) override;
};

}

#endif // BERRYREGISTRYPERSISTENCE_H
