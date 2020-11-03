/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYISELECTIONLISTENER_H_
#define BERRYISELECTIONLISTENER_H_

#include <berryMessage.h>

#include <org_blueberry_ui_qt_Export.h>

#include "berryIWorkbenchPart.h"
#include "berryISelection.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Interface for listening to selection changes.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @see ISelectionService#AddSelectionListener(ISelectionListener::Pointer)
 * @see ISelectionService#AddSelectionListener(const QString&, ISelectionListener::Pointer)
 * @see INullSelectionListener
 */
struct BERRY_UI_QT ISelectionListener
{

  struct Events
  {

    Message2<const IWorkbenchPart::Pointer&, const ISelection::ConstPointer&>
        selectionChanged;

    void AddListener(ISelectionListener* listener);
    void RemoveListener(ISelectionListener* listener);

    typedef MessageDelegate2<ISelectionListener, const IWorkbenchPart::Pointer&,
        const ISelection::ConstPointer&> Delegate;
  };

  virtual ~ISelectionListener();

  /**
   * Notifies this listener that the selection has changed.
   * <p>
   * This method is called when the selection changes from one to a
   * <code>non-null</code> value, but not when the selection changes to
   * <code>null</code>. If there is a requirement to be notified in the latter
   * scenario, implement <code>INullSelectionListener</code>. The event will
   * be posted through this method.
   * </p>
   *
   * @param part the workbench part containing the selection
   * @param selection the current selection. This may be <code>null</code>
   *    if <code>INullSelectionListener</code> is implemented.
   */
  virtual void SelectionChanged(const IWorkbenchPart::Pointer& part,
                                const ISelection::ConstPointer& selection) = 0;
};

/**
 * \ingroup org_blueberry_ui_qt
 *
 * This template can be used like this:
 *
 * <code>
 * class MyClass {
 *
 *   private:
 *     void HandleSelectionChanged(berry::IWorkbenchPart::Pointer part, berry::ISelection::ConstPointer selection)
 *     { // do something }
 *
 *     berry::ISelectionListener::Pointer m_SelectionListener;
 *
 *   public:
 *     MyClass()
 *     : m_SelectionListener(new berry::SelectionChangedAdapter<MyClass>(this, &MyClass::HandleSelectionChanged))
 *     {
 *       // get the selection service
 *       // ...
 *       service->AddPostSelectionListener(m_SelectionListener);
 *     }
 * };
 * </code>
 */
template<typename R>
struct SelectionChangedAdapter: public ISelectionListener
{

  typedef R Listener;
  typedef void
      (R::*Callback)(const IWorkbenchPart::Pointer&, const ISelection::ConstPointer&);

  SelectionChangedAdapter(R* l, Callback c) :
    listener(l), callback(c)
  {
    poco_assert(listener);
    poco_assert(callback);
  }

  void SelectionChanged(const IWorkbenchPart::Pointer& part, const ISelection::ConstPointer& selection) override
  {
    (listener->*callback)(part, selection);
  }

private:

  Listener* listener;
  Callback callback;
};

}

#endif /*BERRYISELECTIONLISTENER_H_*/
