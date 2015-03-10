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


#ifndef BERRYINULLSELECTIONLISTENER_H_
#define BERRYINULLSELECTIONLISTENER_H_

#include "berryISelectionListener.h"

namespace berry
{

/**
 * Interface for listening to <code>null</code> selection changes.
 * <p>
 * This interface should be implemented by selection listeners
 * that want to be notified when the selection becomes <code>null</code>.
 * It has no methods. It simply indicates the desire to receive
 * <code>null</code> selection events through the existing
 * <code>SelectionChanged()</code> method. Either the part or the
 * selection may be <code>null</code>.
 * </p>
 *
 * @see ISelectionListener#SelectionChanged()
 * @see IActionDelegate#SelectionChanged()
 * @see ISelectionListener
 *
 */
struct BERRY_UI_QT INullSelectionListener : public ISelectionListener
{
  berryObjectMacro(berry::INullSelectionListener)

  ~INullSelectionListener();
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
 *     berry::INullSelectionListener::Pointer m_SelectionListener;
 *
 *   public:
 *     MyClass()
 *     : m_SelectionListener(new berry::NullSelectionChangedAdapter<MyClass>(this, &MyClass::HandleSelectionChanged))
 *     {
 *       // get the selection service
 *       // ...
 *       service->AddPostSelectionListener(m_SelectionListener);
 *     }
 * };
 * </code>
 */
template<typename R>
struct NullSelectionChangedAdapter: public INullSelectionListener
{

  typedef R Listener;
  typedef void
      (R::*Callback)(const IWorkbenchPart::Pointer&, const ISelection::ConstPointer&);

  NullSelectionChangedAdapter(R* l, Callback c) :
    listener(l), callback(c)
  {
    poco_assert(listener);
    poco_assert(callback);
  }

  void SelectionChanged(const IWorkbenchPart::Pointer& part,
                        const ISelection::ConstPointer& selection)
  {
    (listener->*callback)(part, selection);
  }

private:

  Listener* listener;
  Callback callback;
};

}

#endif /* BERRYINULLSELECTIONLISTENER_H_ */
