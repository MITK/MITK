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

#ifndef BERRYIPATHEDITORINPUT_H_
#define BERRYIPATHEDITORINPUT_H_

#include "berryIEditorInput.h"

#include <Poco/Path.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 * This interface defines an editor input based on the local file system path
 * of a file.
 * <p>
 * Clients implementing this editor input interface should override
 * <code>Object.equals(Object)</code> to answer true for two inputs
 * that are the same. The <code>IWorkbenchPage.openEditor</code> APIs
 * are dependent on this to find an editor with the same input.
 * </p><p>
 * Path-oriented editors should support this as a valid input type, and
 * can allow full read-write editing of its content.
 * </p><p>
 * All editor inputs must implement the <code>IAdaptable</code> interface;
 * extensions are managed by the platform's adapter manager.
 * </p>
 *
 * @see org.blueberry.core.runtime.IPath
 * @since 3.0
 */
struct BERRY_UI IPathEditorInput : public IEditorInput {

  osgiInterfaceMacro(berry::IPathEditorInput)

  virtual ~IPathEditorInput() {}

    /**
     * Returns the local file system path of the file underlying this editor input.
     *
     * @return a local file system path
     */
   virtual Poco::Path GetPath() const = 0;
};

}

#endif /*BERRYIPATHEDITORINPUT_H_*/
