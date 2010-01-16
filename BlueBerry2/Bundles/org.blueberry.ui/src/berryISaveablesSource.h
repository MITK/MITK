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


#ifndef BERRYISAVEABLESSOURCE_H_
#define BERRYISAVEABLESSOURCE_H_

#include <osgi/framework/Macros.h>
#include "berryUiDll.h"
#include "berrySaveable.h"

namespace berry {

/**
 * Represents a source of Saveable objects (units of saveability). Workbench
 * parts that show more than one unit of saveability, or whose units of
 * saveability change over time, should implement this interface in order to
 * provide better integration with workbench facilities like the Save command,
 * prompts to save on part close or shutdown, etc.
 * <p>
 * IMPORTANT: As of 3.2, implementers of <code>ISaveablesSource</code> must
 * satisfy the following conditions:
 * <ul>
 * <li>If ISaveablesSource is implemented by an IWorkbenchPart:
 * <ul>
 * <li>the part must implement <code>ISaveablePart</code></li>
 * <li>if any of its Saveable objects are dirty, the part must return
 * <code>true</code> from {@link ISaveablePart#isDirty()}</li>
 * <li>the part must return <code>true</code> from
 * {@link ISaveablePart#isSaveOnCloseNeeded()} if it is dirty (the default
 * behaviour implemented by {@link EditorPart})</li>
 * <li>the part must not implement {@link ISaveablePart2}</li>
 * </ul>
 * </li>
 * <li>If ISaveablesSource is implemented by a non-part (possible as of 3.2.1 and 3.3):
 * <ul>
 * <li>the Workbench's {@link ISaveablesLifecycleListener} (obtained from the
 * Workbench by calling
 * <code>workbench.getService(ISaveablesLifecycleListener.class)</code>) must
 * be notified of any change to the result of {@link #getSaveables()} </li>
 * <li>getActiveSaveables() should be implemented to return an empty array
 * </li>
 * </ul>
 * </ul>
 * If any of these conditions are not met, it is undefined whether the Workbench
 * will prompt to save dirty Saveables when closing parts or the Workbench.
 * </p>
 * <p>
 * These conditions may be relaxed in future releases.
 * </p>
 *
 * @since 3.2
 */
struct BERRY_UI ISaveablesSource : public virtual Object {

  osgiInterfaceMacro(berry::ISaveablesSource);

  /**
   * Returns the saveables presented by the workbench part. If the return
   * value of this method changes during the lifetime of
   * this part (i.e. after initialization and control creation but before disposal)
   * the part must notify an implicit listener using
   * {@link ISaveablesLifecycleListener#handleLifecycleEvent(SaveablesLifecycleEvent)}.
   * <p>
   * Additions of saveables to the list of saveables of this part are
   * announced using an event of type
   * {@link SaveablesLifecycleEvent#POST_OPEN}. Removals are announced in a
   * two-stage process, first using an event of type
   * {@link SaveablesLifecycleEvent#PRE_CLOSE} followed by an event of type
   * {@link SaveablesLifecycleEvent#POST_CLOSE}. Since firing the
   * <code>PRE_CLOSE</code> event may trigger prompts to save dirty
   * saveables, the cancellation status of the event must be checked by the
   * part after the notification. When removing only non-dirty saveables,
   * <code>POST_CLOSE</code> notification is sufficient.
   * </p>
   * <p>
   * The listener is obtained from the part site by calling
   * <code>partSite.getService(ISaveablesLifecycleListener.class)</code>.
   * </p>
   * <p>
   * The part must not notify from its initialization methods (e.g. <code>init</code>
   * or <code>createPartControl</code>), or from its dispose method. Parts that
   * implement {@link IReusableEditor} must notify when their input is changed
   * through {@link IReusableEditor#setInput(IEditorInput)}.
   * </p>
   *
   * @return the saveables presented by the workbench part
   *
   * @see ISaveablesLifecycleListener
   */
  virtual std::vector<Saveable::Pointer> GetSaveables() = 0;

  /**
   * Returns the saveables currently active in the workbench part.
   * <p>
   * Certain workbench actions, such as Save, target only the active saveables
   * in the active part. For example, the active saveables could be determined
   * based on the current selection in the part.
   * </p>
   *
   * @return the saveables currently active in the workbench part
   */
  virtual std::vector<Saveable::Pointer> GetActiveSaveables() = 0;
};

}


#endif /* BERRYISAVEABLESSOURCE_H_ */
