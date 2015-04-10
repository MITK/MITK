/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYIPERSISTABLEEDITOR_H
#define BERRYIPERSISTABLEEDITOR_H

#include <berryIPersistable.h>

namespace berry {

/**
 * An editor can implement this interface and participate in the workbench
 * session save/restore cycle using <code>IMemento</code>, similar to how
 * <code>IViewPart</code> currently works.
 * <p>
 * Refer to IWorkbenchPart for the part lifecycle.
 * </p>
 * <p>
 * If a memento is available, RestoreState(*) will be inserted into the editor
 * startup.
 * <ol>
 * <li><code>editor.Init(site, input)</code></li>
 * <li><code>editor.RestoreState(memento)</code></li>
 * <li><code>editor.CreatePartControl(parent)</code></li>
 * <li>...</li>
 * </ol>
 * </p>
 * <p>
 * On workbench shutdown, the editor state will be persisted when the editor
 * references are saved.
 * </p>
 */
struct IPersistableEditor : public IPersistable
{
  /**
   * Called with a memento for this editor. The editor can parse the data or
   * save the memento. This method may not be called.
   *
   * @param memento
   *            the saved state for this editor. May be <code>null</code>.
   */
  virtual void RestoreState(const SmartPointer<const IMemento>& memento) = 0;
};

}

#endif // BERRYIPERSISTABLEEDITOR_H

