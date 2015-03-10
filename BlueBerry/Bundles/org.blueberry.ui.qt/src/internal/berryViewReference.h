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
public: virtual SmartPointer<IWorkbenchPage> GetPage() const;

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
public: virtual QString GetSecondaryId();

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IViewReference#getView(boolean)
   */
public: virtual IViewPart::Pointer GetView(bool restore);


  /**
   * Wrapper for restoring the view. First, this delegates to
   * busyRestoreViewHelper to do the real work of restoring the view. If
   * unable to restore the view, this method tries to substitute an error part
   * and return success.
   *
   * @return the created part
   */
protected: IWorkbenchPart::Pointer CreatePart();

protected: PartPane::Pointer CreatePane();

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
