/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYCOMMANDSERVICE_H_
#define CHERRYCOMMANDSERVICE_H_

#include "../commands/cherryICommandImageService.h"

namespace cherry {

/**
 * <p>
 * Provides services related to the command architecture within the workbench.
 * This service can be used to access the set of commands and handlers.
 * </p>
 *
 * @since 3.1
 */
class CommandService : public ICommandService {


private:

  /**
   * The preference key prefix for all handler state.
   */
  static const std::string PREFERENCE_KEY_PREFIX = "org.eclipse.ui.commands/state"; //$NON-NLS-1$

  /**
   * Creates a preference key for the given piece of state on the given
   * command.
   *
   * @param command
   *            The command for which the preference key should be created;
   *            must not be <code>null</code>.
   * @param stateId
   *            The identifier of the state for which the preference key
   *            should be created; must not be <code>null</code>.
   * @return A suitable preference key; never <code>null</code>.
   */
  static std::string CreatePreferenceKey(Command::ConstPointer command,
      const std::string& stateId) {
    return PREFERENCE_KEY_PREFIX + '/' + command.getId() + '/' + stateId;
  }

  /**
   * The command manager that supports this service. This value is never
   * <code>null</code>.
   */
  CommandManager commandManager;

  /**
   * The persistence class for this command service.
   */
  CommandPersistence commandPersistence;

  /**
   * This is a map of commandIds to a list containing currently registered
   * callbacks, in the form of ICallbackReferences.
   */
  Poco::HashMap<std::string, std::vector<ICallbackReference::Pointer> > commandCallbacks;



public:

  /**
   * Constructs a new instance of <code>CommandService</code> using a
   * command manager.
   *
   * @param commandManager
   *            The command manager to use; must not be <code>null</code>.
   */
  CommandService(final CommandManager commandManager) {
    if (commandManager == null) {
      throw new NullPointerException(
          "Cannot create a command service with a null manager"); //$NON-NLS-1$
    }
    this.commandManager = commandManager;
    this.commandPersistence = new CommandPersistence(this);
  }

  void addExecutionListener(final IExecutionListener listener) {
    commandManager.addExecutionListener(listener);
  }

  void defineUncategorizedCategory(final String name,
      final String description) {
    commandManager.defineUncategorizedCategory(name, description);
  }

  ParameterizedCommand deserialize(
      final String serializedParameterizedCommand)
      throws NotDefinedException, SerializationException {
    return commandManager.deserialize(serializedParameterizedCommand);
  }

  void dispose() {
    commandPersistence.dispose();

    /*
     * All state on all commands neeeds to be disposed. This is so that the
     * state has a chance to persist any changes.
     */
    final Command[] commands = commandManager.getAllCommands();
    for (int i = 0; i < commands.length; i++) {
      final Command command = commands[i];
      final String[] stateIds = command.getStateIds();
      for (int j = 0; j < stateIds.length; j++) {
        final String stateId = stateIds[j];
        final State state = command.getState(stateId);
        if (state instanceof PersistentState) {
          final PersistentState persistentState = (PersistentState) state;
          if (persistentState.shouldPersist()) {
            persistentState.save(PrefUtil
                .getInternalPreferenceStore(),
                createPreferenceKey(command, stateId));
          }
        }
      }
    }
    commandCallbacks = null;
  }

  Category getCategory(final String categoryId) {
    return commandManager.getCategory(categoryId);
  }

  Command getCommand(final String commandId) {
    return commandManager.getCommand(commandId);
  }

  Category[] getDefinedCategories() {
    return commandManager.getDefinedCategories();
  }

  Collection getDefinedCategoryIds() {
    return commandManager.getDefinedCategoryIds();
  }

  Collection getDefinedCommandIds() {
    return commandManager.getDefinedCommandIds();
  }

  Command[] getDefinedCommands() {
    return commandManager.getDefinedCommands();
  }

  Collection getDefinedParameterTypeIds() {
    return commandManager.getDefinedParameterTypeIds();
  }

  ParameterType[] getDefinedParameterTypes() {
    return commandManager.getDefinedParameterTypes();
  }

  String getHelpContextId(final Command command)
      throws NotDefinedException {
    return commandManager.getHelpContextId(command);
  }

  String getHelpContextId(final String commandId)
      throws NotDefinedException {
    final Command command = getCommand(commandId);
    return commandManager.getHelpContextId(command);
  }

  ParameterType getParameterType(final String parameterTypeId) {
    return commandManager.getParameterType(parameterTypeId);
  }

  void readRegistry() {
    commandPersistence.read();
  }

  void removeExecutionListener(final IExecutionListener listener) {
    commandManager.removeExecutionListener(listener);
  }

  void setHelpContextId(final IHandler handler,
      final String helpContextId) {
    commandManager.setHelpContextId(handler, helpContextId);
  }

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.commands.ICommandService#refreshElements(java.lang.String,
   *      java.util.Map)
   */
  void refreshElements(String commandId, Map filter) {
    Command cmd = getCommand(commandId);

    if (!cmd.isDefined() || !(cmd.getHandler() instanceof IElementUpdater)) {
      return;
    }
    final IElementUpdater updater = (IElementUpdater) cmd.getHandler();

    if (commandCallbacks == null) {
      return;
    }

    List callbackRefs = (List) commandCallbacks.get(commandId);
    if (callbackRefs == null) {
      return;
    }

    for (Iterator i = callbackRefs.iterator(); i.hasNext();) {
      final IElementReference callbackRef = (IElementReference) i.next();
      final Map parms = Collections.unmodifiableMap(callbackRef
          .getParameters());
      ISafeRunnable run = new ISafeRunnable() {
        public void handleException(Throwable exception) {
          WorkbenchPlugin.log("Failed to update callback: "  //$NON-NLS-1$
              + callbackRef.getCommandId(), exception);
        }

        public void run() throws Exception {
          updater.updateElement(callbackRef.getElement(), parms);
        }
      };
      if (filter == null) {
        SafeRunner.run(run);
      } else {
        boolean match = true;
        for (Iterator j = filter.entrySet().iterator(); j.hasNext()
            && match;) {
          Map.Entry parmEntry = (Map.Entry) j.next();
          Object value = parms.get(parmEntry.getKey());
          if (!parmEntry.getValue().equals(value)) {
            match = false;
          }
        }
        if (match) {
          SafeRunner.run(run);
        }
      }
    }
  }

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.commands.ICommandService#registerElementForCommand(org.eclipse.core.commands.ParameterizedCommand,
   *      org.eclipse.ui.menus.UIElement)
   */
  IElementReference registerElementForCommand(
      ParameterizedCommand command, UIElement element)
      throws NotDefinedException {
    if (!command.getCommand().isDefined()) {
      throw new NotDefinedException(
          "Cannot define a callback for undefined command " //$NON-NLS-1$
              + command.getCommand().getId());
    }
    if (element == null) {
      throw new NotDefinedException("No callback defined for command " //$NON-NLS-1$
          + command.getCommand().getId());
    }

    ElementReference ref = new ElementReference(command.getId(), element,
        command.getParameterMap());
    registerElement(ref);
    return ref;
  }

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.commands.ICommandService#registerElement(org.eclipse.ui.commands.IElementReference)
   */
  void registerElement(IElementReference elementReference) {
    List parameterizedCommands = (List) commandCallbacks
        .get(elementReference.getCommandId());
    if (parameterizedCommands == null) {
      parameterizedCommands = new ArrayList();
      commandCallbacks.put(elementReference.getCommandId(),
          parameterizedCommands);
    }
    parameterizedCommands.add(elementReference);

    // If the active handler wants to update the callback, it can do
    // so now
    Command command = getCommand(elementReference.getCommandId());
    if (command.isDefined()) {
      if (command.getHandler() instanceof IElementUpdater) {
        ((IElementUpdater) command.getHandler()).updateElement(
            elementReference.getElement(), elementReference
                .getParameters());
      }
    }
  }

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.commands.ICommandService#unregisterElement(org.eclipse.ui.commands.IElementReference)
   */
  void unregisterElement(IElementReference elementReference) {
    List parameterizedCommands = (List) commandCallbacks
        .get(elementReference.getCommandId());
    if (parameterizedCommands != null) {
      parameterizedCommands.remove(elementReference);
      if (parameterizedCommands.isEmpty()) {
        commandCallbacks.remove(elementReference.getCommandId());
      }
    }
  }

  /**
   * @return Returns the commandPersistence.
   */
  CommandPersistence getCommandPersistence() {
    return commandPersistence;
  }
};

}

#endif /* CHERRYCOMMANDSERVICE_H_ */
