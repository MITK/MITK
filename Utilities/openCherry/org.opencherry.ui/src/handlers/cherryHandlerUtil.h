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

#ifndef CHERRYHANDLERUTIL_H_
#define CHERRYHANDLERUTIL_H_

#include <org.opencherry.core.commands/cherryExecutionEvent.h>
#include <org.opencherry.osgi/cherryObjectVector.h>
#include <org.opencherry.osgi/cherryObjectString.h>

#include "../cherryUiDll.h"
#include "../cherryIWorkbenchPart.h"
#include "../cherryIWorkbenchPartSite.h"
#include "../cherryISelection.h"
#include "../cherryIWorkbenchWindow.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 * 
 * Some common utilities for working with handlers in Platform UI.
 * <p>
 * <b>Note</b>: this class should not be instantiated or extended by clients.
 * </p>
 * 
 * @since 3.3
 */
class CHERRY_UI HandlerUtil {
  
private:
  
  static void NoVariableFound(ExecutionEvent::Pointer event, const std::string& name);

  static void IncorrectTypeFound(ExecutionEvent::Pointer event, const std::string& name,
      const std::string& expectedType, const std::string& wrongType);

  
public:
  
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
  static Object::Pointer GetVariable(ExecutionEvent::Pointer event, const std::string& name);

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
  static Object::Pointer GetVariableChecked(ExecutionEvent::Pointer event, const std::string& name);

  /**
   * Extract the variable.
   * 
   * @param context
   *            The IEvaluationContext or <code>null</code>
   * @param name
   *            The variable name to extract.
   * @return The object from the application context, or <code>null</code>
   *         if it could not be found.
   * @since 3.4
   */
  static Object::Pointer GetVariable(Object::Pointer context, const std::string& name);

  /**
   * Return the active contexts.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return a collection of String contextIds, or <code>null</code>.
   */
  static ObjectVector::Pointer GetActiveContexts(ExecutionEvent::Pointer event);

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
  static ObjectVector::Pointer GetActiveContextsChecked(ExecutionEvent::Pointer event);

  /**
   * Return the active shell. Is not necessarily the active workbench window
   * shell.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the active shell, or <code>null</code>.
   */
//  static Shell GetActiveShell(ExecutionEvent::Pointer event) {
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
//  static Shell GetActiveShellChecked(ExecutionEvent::Pointer event)
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
   static IWorkbenchWindow::Pointer GetActiveWorkbenchWindow(ExecutionEvent::Pointer event);

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
        ExecutionEvent::Pointer event);

  /**
   * Return the active editor.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the active editor, or <code>null</code>.
   */
  //static IEditorPart::Pointer GetActiveEditor(ExecutionEvent::Pointer event);

  /**
   * Return the active editor.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the active editor. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active editor variable is not found.
   */
  //static IEditorPart::Pointer GetActiveEditorChecked(ExecutionEvent::Pointer event);

  /**
   * Return the part id of the active editor.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the part id of the active editor, or <code>null</code>.
   */
  static ObjectString::Pointer GetActiveEditorId(ExecutionEvent::Pointer event);

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
  static ObjectString::Pointer GetActiveEditorIdChecked(ExecutionEvent::Pointer event);

  /**
   * Return the active part.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the active part, or <code>null</code>.
   */
  static IWorkbenchPart::Pointer GetActivePart(ExecutionEvent::Pointer event);

  /**
   * Return the active part.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the active part. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active part variable is not found.
   */
  static IWorkbenchPart::Pointer GetActivePartChecked(ExecutionEvent::Pointer event);

  /**
   * Return the part id of the active part.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the part id of the active part, or <code>null</code>.
   */
  static ObjectString::Pointer GetActivePartId(ExecutionEvent::Pointer event);

  /**
   * Return the part id of the active part.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the part id of the active part. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active part id variable is not found.
   */
  static ObjectString::Pointer GetActivePartIdChecked(ExecutionEvent::Pointer event);

  /**
   * Return the active part site.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the active part site, or <code>null</code>.
   */
  static IWorkbenchPartSite::Pointer GetActiveSite(ExecutionEvent::Pointer event);

  /**
   * Return the active part site.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the active part site. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the active part site variable is not found.
   */
  static IWorkbenchPartSite::Pointer GetActiveSiteChecked(ExecutionEvent::Pointer event);

  /**
   * Return the current selection.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the current selection, or <code>null</code>.
   */
  static ISelection::Pointer GetCurrentSelection(ExecutionEvent::Pointer event);

  /**
   * Return the current selection.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the current selection. Will not return <code>null</code>.
   * @throws ExecutionException
   *             If the current selection variable is not found.
   */
  static ISelection::Pointer GetCurrentSelectionChecked(ExecutionEvent::Pointer event);

  /**
   * Return the menu IDs that were applied to the registered context menu. For
   * example, #CompilationUnitEditorContext.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the menu IDs, or <code>null</code>.
   */
  static ObjectVector::Pointer GetActiveMenus(ExecutionEvent::Pointer event);

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
  static ObjectVector::Pointer GetActiveMenusChecked(ExecutionEvent::Pointer event);

  /**
   * Return the active menu selection. The active menu is a registered context
   * menu.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the active menu selection, or <code>null</code>.
   */
  static ISelection::Pointer GetActiveMenuSelection(ExecutionEvent::Pointer event);

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
  static ISelection::Pointer GetActiveMenuSelectionChecked(ExecutionEvent::Pointer event);

  /**
   * Return the active menu editor input, if available. The active menu is a
   * registered context menu.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the active menu editor, or <code>null</code>.
   */
  static ISelection::Pointer GetActiveMenuEditorInput(ExecutionEvent::Pointer event);

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
  static ISelection::Pointer GetActiveMenuEditorInputChecked(
      ExecutionEvent::Pointer event);

  /**
   * Return the ShowInContext selection.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the show in selection, or <code>null</code>.
   * @since 3.4
   */
  static ISelection::Pointer GetShowInSelection(ExecutionEvent::Pointer event);

  /**
   * Return the ShowInContext selection. Will not return <code>null</code>.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the show in selection, or <code>null</code>.
   * @throws ExecutionException
   *             If the show in selection variable is not found.
   * @since 3.4
   */
  static ISelection::Pointer GetShowInSelectionChecked(ExecutionEvent::Pointer event);

  /**
   * Return the ShowInContext input.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the show in input, or <code>null</code>.
   * @since 3.4
   */
  static Object::Pointer GetShowInInput(ExecutionEvent::Pointer event);

  /**
   * Return the ShowInContext input. Will not return <code>null</code>.
   * 
   * @param event
   *            The execution event that contains the application context
   * @return the show in input, or <code>null</code>.
   * @throws ExecutionException
   *             If the show in input variable is not found.
   * @since 3.4
   */
  static Object::Pointer GetShowInInputChecked(ExecutionEvent::Pointer event);
};

}
#endif /*CHERRYHANDLERUTIL_H_*/
