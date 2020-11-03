/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIWORKBENCHPARTREFERENCE_H_
#define BERRYIWORKBENCHPARTREFERENCE_H_

#include <berryMacros.h>

#include <org_blueberry_ui_qt_Export.h>
#include "berryIPropertyChangeListener.h"

namespace berry {

struct IWorkbenchPart;
struct IWorkbenchPage;

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Implements a reference to a IWorkbenchPart.
 * The IWorkbenchPart will not be instanciated until the part
 * becomes visible or the API getPart is sent with true;
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 */
struct BERRY_UI_QT IWorkbenchPartReference : public Object
{

  berryObjectMacro(berry::IWorkbenchPartReference);

  ~IWorkbenchPartReference() override;

  /**
   * Returns the IWorkbenchPart referenced by this object.
   * Returns <code>null</code> if the editors was not instantiated or
   * it failed to be restored. Tries to restore the editor
   * if <code>restore</code> is true.
   */
  virtual SmartPointer<IWorkbenchPart> GetPart(bool restore) = 0;

  /**
   * @see IWorkbenchPart#getTitleImage
   */
  virtual QIcon GetTitleImage() const = 0;

  /**
   * @see IWorkbenchPart#getTitleToolTip
   */
  virtual QString GetTitleToolTip() const = 0;

  /**
   * @see IWorkbenchPartSite#getId
   */
  virtual QString GetId() const = 0;

  /**
   * @see IWorkbenchPart#addPropertyListener
   */
  virtual void AddPropertyListener(IPropertyChangeListener* listener) = 0;

  /**
   * @see IWorkbenchPart#removePropertyListener
   */
  virtual void RemovePropertyListener(IPropertyChangeListener* listener) = 0;

  /**
   * Returns the workbench page that contains this part
   */
  virtual SmartPointer<IWorkbenchPage> GetPage() const = 0;

  /**
   * Returns the name of the part, as it should be shown in tabs.
   *
   * @return the part name
    */
  virtual QString GetPartName() const = 0;

  /**
   * Returns the content description for the part (or the empty string if none)
   *
   * @return the content description for the part
   */
  virtual QString GetContentDescription() const = 0;

  /**
   * Returns true if the part is pinned otherwise returns false.
   */
  virtual bool IsPinned() const = 0;

  /**
   * Returns whether the part is dirty (i.e. has unsaved changes).
   *
   * @return <code>true</code> if the part is dirty, <code>false</code> otherwise
   */
  virtual bool IsDirty() const = 0;

  /**
   * Return an arbitrary property from the reference. If the part has been
   * instantiated, it just delegates to the part. If not, then it looks in its
   * own cache of properties. If the property is not available or the part has
   * never been instantiated, it can return <code>null</code>.
   *
   * @param key
   *            The property to return. Must not be <code>null</code>.
   * @return The String property, or <code>null</code>.
   */
  virtual QString GetPartProperty(const QString& key) const = 0;

  /**
   * Add a listener for changes in the arbitrary properties set.
   *
   * @param listener
   *            Must not be <code>null</code>.
   */
  //virtual void addPartPropertyListener(IPropertyChangeListener listener) = 0;

  /**
   * Remove a listener for changes in the arbitrary properties set.
   *
   * @param listener
   *            Must not be <code>null</code>.
   */
  //virtual void removePartPropertyListener(IPropertyChangeListener listener) = 0;
};

}  // namespace berry

#endif /*BERRYIWORKBENCHPARTREFERENCE_H_*/
