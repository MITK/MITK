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

#ifndef BERRYIFILEEDITORMAPPING_H_
#define BERRYIFILEEDITORMAPPING_H_

#include <osgi/framework/Macros.h>

#include "berryIEditorDescriptor.h"

#include <string>
#include <vector>
#include <list>

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 * An association between a file name/extension and a list of known editors for
 * files of that type.
 * <p>
 * The name and extension can never empty or null. The name may contain
 * the single wild card character (*) to indicate the editor applies to
 * all files with the same extension (e.g. *.doc). The name can never
 * embed the wild card character within itself (i.e. rep*)
 * </p>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see IEditorRegistry#getFileEditorMappings
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI IFileEditorMapping : public Object {

  osgiInterfaceMacro(berry::IFileEditorMapping)

  virtual ~IFileEditorMapping() {}

    /**
     * Returns the default editor registered for this type mapping.
     *
     * @return the descriptor of the default editor, or <code>null</code> if there
     *   is no default editor registered. Will also return <code>null</code> if
     *   the default editor exists but fails the Expressions check.
     */
    virtual IEditorDescriptor::Pointer GetDefaultEditor() = 0;

    /**
     * Returns the list of editors registered for this type mapping.
     *
     * @return a possibly empty list of editors.
     */
    virtual std::list<IEditorDescriptor::Pointer> GetEditors() const = 0;

    /**
     * Returns the list of editors formerly registered for this type mapping
     * which have since been deleted.
     *
     * @return a possibly empty list of editors
     */
    virtual std::list<IEditorDescriptor::Pointer> GetDeletedEditors() const = 0;

    /**
     * Returns the file's extension for this type mapping.
     *
     * @return the extension for this mapping
     */
    virtual std::string GetExtension() const = 0;

    /**
     * Returns the descriptor of the image to use for a file of this type.
     * <p>
     * The image is obtained from the default editor. A default file image is
     * returned if no default editor is available.
     * </p>
     *
     * @return the descriptor of the image to use for a resource of this type
     */
    //virtual ImageDescriptor GetImageDescriptor() = 0;

    /**
     * Returns the label to use for this mapping.
     * Labels have the form "<it>name</it>.<it>extension</it>".
     *
     * @return the label to use for this mapping
     */
    virtual std::string GetLabel() const = 0;

    /**
     * Returns the file's name for this type mapping.
     *
     * @return the name for this mapping
     */
    virtual std::string GetName() const = 0;
};

}
#endif /*BERRYIFILEEDITORMAPPING_H_*/
