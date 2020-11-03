/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHANDLERUTIL_H_
#define BERRYHANDLERUTIL_H_

#include <berryExecutionEvent.h>
#include <berryObjectList.h>
#include <berryObjectString.h>

#include <org_blueberry_ui_qt_Export.h>
#include "berryIWorkbenchPart.h"
#include "berryIWorkbenchPartSite.h"
#include "berryISelection.h"
#include "berryIWorkbenchWindow.h"

#include "berryISources.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Some common utilities for working with handlers in Platform UI.
 * <p>
 * <b>Note</b>: this class should not be instantiated or extended by clients.
 * </p>
 *
 * @since 3.3
 */
class BERRY_UI_QT HandlerUtil {

private:

  static void NoVariableFound(const ExecutionEvent::ConstPointer& event, const QString& name);

  static void IncorrectTypeFound(const ExecutionEvent::ConstPointer& event, const QString& name,
                                 const QString& expectedType, const QString& wrongType);


public:

  typedef ObjectList<ObjectString::Pointer> StringVectorType;

  /**
   * Extract the variable.
   *
   * @param event
   *            The execution event that contains the application context
   * @param name
   *            The variable name to extract.
   * @return The object from the application context, or <code>null</code>
   *         if it could not be found.
   */
  static Object::ConstPointer GetVariable(const ExecutionEvent::ConstPointer& event, const QString& name);

  /**
   * Extract the variable.
   *
   * @param event
   *            The execution event that contains the application context
   * @param name
   *            The variable name to extract.
   * @return The object from the application context. Will not return
   *         <code>null</code>.
   * @throws ExecutionException
   *             if the variable is not found.
   */
  static Object::ConstPointer GetVariableChecked(const ExecutionEvent::ConstPointer& event, const QString& name);

  /**
   * Extract the variable.
   *
   * @param context
   *            The IEvaluationContext or <code>null</code>
   * @param name
   *            The variable name to extract.
   * @return The object from the application context, or <code>null</code>
   *         if it could not be found.
   */
  static Object::ConstPointer GetVariable(Object::Pointer context, const QString& name);

  /**
   * Return the active contexts.
   *
   * @param event
   *            The execution event that contains the application context
   * @return a collection of String contextIds, or <code>null</code>.
   */
  static StringVectorType::ConstPointer GetActiveContexts(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active contexts.
   *
   * @param event
   *            The execution event that contains the application context
   * @return a collection of String contextIds. Will not return
   *         <code>null</code>.
   * @throws ExecutionException
   *             If the context variable is not found.
   */
  static StringVectorType::ConstPointer GetActiveContextsChecked(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active shell. Is not necessarily the active workbench window
   * shell.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active shell, or <code>null</code>.
   */
//  static Shell GetActiveShell(const ExecutionEvent::ConstPointer& event) {
//    Object::Pointer o = getVariable(event, ISources.ACTIVE_SHELL_NAME);
//    if (o instanceof Shell) {
//      return (Shell) o;
//    }
//    return null;
//  }

  /**
   * Return the active shell. Is not necessarily the active workbench window
   * shell.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active shell. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active shell variable is not found.
   */
//  static Shell GetActiveShellChecked(const ExecutionEvent::ConstPointer& event)
//       {
//    Object::Pointer o = getVariableChecked(event, ISources.ACTIVE_SHELL_NAME);
//    if (!(o instanceof Shell)) {
//      incorrectTypeFound(event, ISources.ACTIVE_SHELL_NAME, Shell.class,
//          o.getClass());
//    }
//    return (Shell) o;
//  }

  /**
     * Return the active workbench window.
     *
     * @param event
     *            The execution event that contains the application context
     * @return the active workbench window, or <code>null</code>.
     */
   static IWorkbenchWindow::Pointer GetActiveWorkbenchWindow(const ExecutionEvent::ConstPointer& event);

    /**
     * Return the active workbench window.
     *
     * @param event
     *            The execution event that contains the application context
     * @return the active workbench window. Will not return <code>null</code>.
     * @throws ExecutionException
     *             If the active workbench window variable is not found.
     */
    static IWorkbenchWindow::Pointer GetActiveWorkbenchWindowChecked(
        const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active editor.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active editor, or <code>null</code>.
   */
  //static IEditorPart::Pointer GetActiveEditor(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active editor.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active editor. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active editor variable is not found.
   */
  //static IEditorPart::Pointer GetActiveEditorChecked(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the part id of the active editor.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the part id of the active editor, or <code>null</code>.
   */
  static ObjectString::ConstPointer GetActiveEditorId(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the part id of the active editor.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the part id of the active editor. Will not return
   *         <code>null</code>.
   * @throws ExecutionException
   *             If the active editor id variable is not found.
   */
  static ObjectString::ConstPointer GetActiveEditorIdChecked(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active part.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active part, or <code>null</code>.
   */
  static IWorkbenchPart::Pointer GetActivePart(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active part.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active part. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active part variable is not found.
   */
  static IWorkbenchPart::Pointer GetActivePartChecked(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the part id of the active part.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the part id of the active part, or <code>null</code>.
   */
  static ObjectString::ConstPointer GetActivePartId(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the part id of the active part.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the part id of the active part. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active part id variable is not found.
   */
  static ObjectString::ConstPointer GetActivePartIdChecked(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active part site.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active part site, or <code>null</code>.
   */
  static IWorkbenchPartSite::Pointer GetActiveSite(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active part site.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active part site. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active part site variable is not found.
   */
  static IWorkbenchPartSite::Pointer GetActiveSiteChecked(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the current selection.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the current selection, or <code>null</code>.
   */
  static ISelection::ConstPointer GetCurrentSelection(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the current selection.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the current selection. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the current selection variable is not found.
   */
  static ISelection::ConstPointer GetCurrentSelectionChecked(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the menu IDs that were applied to the registered context menu. For
   * example, #CompilationUnitEditorContext.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the menu IDs, or <code>null</code>.
   */
  static StringVectorType::ConstPointer GetActiveMenus(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the menu IDs that were applied to the registered context menu. For
   * example, #CompilationUnitEditorContext.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the menu IDs. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active menus variable is not found.
   */
  static StringVectorType::ConstPointer GetActiveMenusChecked(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active menu selection. The active menu is a registered context
   * menu.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active menu selection, or <code>null</code>.
   */
  static ISelection::ConstPointer GetActiveMenuSelection(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active menu selection. The active menu is a registered context
   * menu.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active menu selection. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active menu selection variable is not found.
   */
  static ISelection::ConstPointer GetActiveMenuSelectionChecked(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active menu editor input, if available. The active menu is a
   * registered context menu.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active menu editor, or <code>null</code>.
   */
  static ISelection::ConstPointer GetActiveMenuEditorInput(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the active menu editor input. The active menu is a registered
   * context menu. Some context menus do not include the editor input which
   * will throw an exception.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the active menu editor input. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active menu editor input variable is not found.
   */
  static ISelection::ConstPointer GetActiveMenuEditorInputChecked(
      const ExecutionEvent::ConstPointer& event);

  /**
   * Return the ShowInContext selection.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the show in selection, or <code>null</code>.
   */
  static ISelection::ConstPointer GetShowInSelection(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the ShowInContext selection. Will not return <code>null</code>.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the show in selection, or <code>null</code>.
   * @throws ExecutionException
   *             If the show in selection variable is not found.
   */
  static ISelection::ConstPointer GetShowInSelectionChecked(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the ShowInContext input.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the show in input, or <code>null</code>.
   */
  static Object::ConstPointer GetShowInInput(const ExecutionEvent::ConstPointer& event);

  /**
   * Return the ShowInContext input. Will not return <code>null</code>.
   *
   * @param event
   *            The execution event that contains the application context
   * @return the show in input, or <code>null</code>.
   * @throws ExecutionException
   *             If the show in input variable is not found.
   */
  static Object::ConstPointer GetShowInInputChecked(const ExecutionEvent::ConstPointer& event);

  /**
   * Toggles the command's state.
   *
   * @param command The command whose state needs to be toggled
   * @return the original value before toggling
   *
   * @throws ExecutionException
   * When the command doesn't contain the toggle state or when the state doesn't contain a boolean value
   */
  static bool ToggleCommandState(const SmartPointer<Command>& command);

  /**
   * Checks whether the radio state of the command is same as the radio state
   * parameter's value
   *
   * @param event
   *            The execution event that contains the application context
   * @return <code>true</code> whe the values are same, <code>false</code>
   *         otherwise
   *
   * @throws ExecutionException
   *             When the command doesn't have the radio state or the event
   *             doesn't have the radio state parameter
   */
  static bool MatchesRadioState(const SmartPointer<ExecutionEvent>& event);

  /**
   * Updates the radio state of the command to the given value
   *
   * @param command
   *            the command whose state should be updated
   * @param newState
   *            the new state
   *
   * @throws ExecutionException
   *             When the command doesn't have a radio state
   */
  static void UpdateRadioState(const SmartPointer<Command>& command, const QString& newState);
};

}
#endif /*BERRYHANDLERUTIL_H_*/
