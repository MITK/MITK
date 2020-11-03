/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYVIEWREFERENCE_H_
#define BERRYVIEWREFERENCE_H_

#include "berryIMemento.h"

#include "berryWorkbenchPartReference.h"
#include "berryIViewReference.h"
#include "berryPartPane.h"

namespace berry {

class ViewFactory;
struct IWorkbenchPage;

/**
 * \ingroup org_blueberry_ui_internal
 *
 */
class ViewReference : public WorkbenchPartReference, public IViewReference {

public: berryObjectMacro(ViewReference)

private: const ViewFactory* factory;

public: QString secondaryId;

private: IMemento::Pointer memento;

//private: ViewActionBuilder actionBuilder;

public: ViewReference(ViewFactory* factory, const QString& id,
    const QString& secondaryId, IMemento::Pointer memento);

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.internal.WorkbenchPartReference#dispose()
   */
protected: virtual void DoDisposePart();

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IWorkbenchPartReference#getPage()
   */
public: SmartPointer<IWorkbenchPage> GetPage() const override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.internal.WorkbenchPartReference#getRegisteredName()
   */
public: virtual QString GetRegisteredName();

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IViewReference
   */
public: QString GetSecondaryId() override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IViewReference#getView(boolean)
   */
public: IViewPart::Pointer GetView(bool restore) override;


  /**
   * Wrapper for restoring the view. First, this delegates to
   * busyRestoreViewHelper to do the real work of restoring the view. If
   * unable to restore the view, this method tries to substitute an error part
   * and return success.
   *
   * @return the created part
   */
protected: IWorkbenchPart::Pointer CreatePart() override;

protected: PartPane::Pointer CreatePane() override;

private: IWorkbenchPart::Pointer CreatePartHelper();

  /**
   * The memento is that last view state saved by the workbench.
   *
   * @return the last state that was saved by the workbench. It can return
   *         <code>null</code>.
   * @since 3.1.1
   */
public: virtual IMemento::Pointer GetMemento();
};

}  // namespace berry

#endif /*BERRYVIEWREFERENCE_H_*/
