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

#ifndef CHERRYIEDITORDESCRIPTOR_H_
#define CHERRYIEDITORDESCRIPTOR_H_

#include "cherryUiDll.h"
#include "cherryIWorkbenchPartDescriptor.h"
#include "cherryIEditorMatchingStrategy.h"

namespace cherry
{

/**
 * \ingroup org_opencherry_ui
 *
 * Description of an editor in the workbench editor registry. The
 * editor descriptor contains the information needed to create editor instances.
 * <p>
 * An editor descriptor typically represents one of three types of editors:
 * <ul>
 *   <li>a file editor extension for a specific file extension.</li>
 *   <li>a file editor added by the user (via the workbench preference page)</li>
 *   <li>a general editor extension which works on objects other than files.</li>
 * </ul>
 * </p>
 * <p>
 * This interface is not intended to be implemented or extended by clients.
 * </p>
 *
 * @see IEditorRegistry
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct CHERRY_UI IEditorDescriptor : public IWorkbenchPartDescriptor
{

  cherryInterfaceMacro(IEditorDescriptor, cherry);

  virtual ~IEditorDescriptor()
  {}

  /**
   * Returns the editor id.
   * <p>
   * For internal editors, this is the extension id as defined in the workbench
   * registry; for external editors, it is path and file name of the external
   * program.
   * </p>
   *
   * @return the id of the editor
   */
  virtual std::string GetId() const = 0;

  /**
   * Returns the descriptor of the image for this editor.
   *
   * @return the descriptor of the image to display next to this editor
   */
  //ImageDescriptor getImageDescriptor() = 0;

  /**
   * Returns the label to show for this editor.
   *
   * @return the editor label
   */
  virtual std::string GetLabel() = 0;

  /**
   * Returns whether this editor descriptor will open a regular editor
   * part inside the editor area.
   *
   * @return <code>true</code> if editor is inside editor area, and
   * <code>false</code> otherwise
   * @since 3.0
   */
  virtual bool IsInternal() = 0;

  /**
   * Returns whether this editor descriptor will open an external
   * editor in-place inside the editor area.
   *
   * @return <code>true</code> if editor is in-place, and <code>false</code>
   * otherwise
   * @since 3.0
   */
  virtual bool IsOpenInPlace() = 0;

  /**
   * Returns whether this editor descriptor will open an external editor
   * in a new window outside the workbench.
   *
   * @return <code>true</code> if editor is external, and <code>false</code>
   * otherwise
   * @since 3.0
   */
  virtual bool IsOpenExternal() = 0;

  /**
   * Returns the editor matching strategy object for editors
   * represented by this editor descriptor, or <code>null</code>
   * if there is no explicit matching strategy specified.
   *
   * @return the editor matching strategy, or <code>null</code> if none
   * @since 3.1
   */
  virtual IEditorMatchingStrategy::Pointer GetEditorMatchingStrategy() = 0;
};

}

#endif /*CHERRYIEDITORDESCRIPTOR_H_*/
