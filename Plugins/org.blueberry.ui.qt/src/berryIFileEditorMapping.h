/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIFILEEDITORMAPPING_H_
#define BERRYIFILEEDITORMAPPING_H_

#include <berryMacros.h>

#include "berryIEditorDescriptor.h"


namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
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
struct BERRY_UI_QT IFileEditorMapping : public Object
{

  berryObjectMacro(berry::IFileEditorMapping);

  ~IFileEditorMapping() override;

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
    virtual QList<IEditorDescriptor::Pointer> GetEditors() const = 0;

    /**
     * Returns the list of editors formerly registered for this type mapping
     * which have since been deleted.
     *
     * @return a possibly empty list of editors
     */
    virtual QList<IEditorDescriptor::Pointer> GetDeletedEditors() const = 0;

    /**
     * Returns the file's extension for this type mapping.
     *
     * @return the extension for this mapping
     */
    virtual QString GetExtension() const = 0;

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
    virtual QString GetLabel() const = 0;

    /**
     * Returns the file's name for this type mapping.
     *
     * @return the name for this mapping
     */
    virtual QString GetName() const = 0;
};

}
#endif /*BERRYIFILEEDITORMAPPING_H_*/
