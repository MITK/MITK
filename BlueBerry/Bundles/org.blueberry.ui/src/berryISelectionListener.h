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

#ifndef BERRYISELECTIONLISTENER_H_
#define BERRYISELECTIONLISTENER_H_

#include <berryMessage.h>

#include "berryUiDll.h"

#include "berryIWorkbenchPart.h"
#include "berryISelection.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui
 *
 * Interface for listening to selection changes.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @see ISelectionService#AddSelectionListener(ISelectionListener::Pointer)
 * @see ISelectionService#AddSelectionListener(const std::string&, ISelectionListener::Pointer)
 * @see INullSelectionListener
 */
struct BERRY_UI ISelectionListener: public virtual Object
{

  berryInterfaceMacro(ISelectionListener, berry)

  struct Events
  {

    Message2<IWorkbenchPart::Pointer, ISelection::ConstPointer>
        selectionChanged;

    void AddListener(ISelectionListener::Pointer listener);
    void RemoveListener(ISelectionListener::Pointer listener);

    typedef MessageDelegate2<ISelectionListener, IWorkbenchPart::Pointer,
        ISelection::ConstPointer> Delegate;
  };

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
  virtual void SelectionChanged(IWorkbenchPart::Pointer part,
      ISelection::ConstPointer selection) = 0;
};

/**
 * \ingroup org_blueberry_ui
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
      (R::*Callback)(IWorkbenchPart::Pointer, ISelection::ConstPointer);

  SelectionChangedAdapter(R* l, Callback c) :
    listener(l), callback(c)
  {
    poco_assert(listener);
    poco_assert(callback);
  }

  void SelectionChanged(IWorkbenchPart::Pointer part, ISelection::ConstPointer selection)
  {
    (listener->*callback)(part, selection);
  }

private:

  Listener* listener;
  Callback callback;
};

}

#endif /*BERRYISELECTIONLISTENER_H_*/
