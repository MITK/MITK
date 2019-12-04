/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryMockSelectionProvider.h"

namespace berry
{

struct EmptyStructuredSelection : public IStructuredSelection
{

  EmptyStructuredSelection()
  : elements(new ContainerType())
  {}

  Object::Pointer GetFirstElement() const
  {
    return Object::Pointer(0);
  }

  iterator Begin() const
  {
    return elements->begin();
  }

  iterator End() const
  {
    return elements->end();
  }

  int Size() const
  {
    return elements->size();
  }

  bool IsEmpty() const
  {
    return true;
  }

  ContainerType::Pointer ToVector() const
  {
    return elements;
  }

private:

  ContainerType::Pointer elements;
};

MockSelectionProvider::MockSelectionProvider()
: emptySelection(new EmptyStructuredSelection())
{

}

void MockSelectionProvider::FireSelection()
{
  SelectionChangedEvent::Pointer event(new SelectionChangedEvent(
      ISelectionProvider::Pointer(this), emptySelection));
  FireSelection(event);
}

void MockSelectionProvider::FireSelection(SelectionChangedEvent::Pointer event)
{
  listeners.selectionChanged(event);
}

void MockSelectionProvider::AddSelectionChangedListener(
    ISelectionChangedListener::Pointer listener)
{
  listeners.AddListener(listener);
}

ISelection::ConstPointer MockSelectionProvider::GetSelection() const
{
  return emptySelection;
}

void MockSelectionProvider::RemoveSelectionChangedListener(
    ISelectionChangedListener::Pointer listener)
{
  listeners.RemoveListener(listener);
}

void MockSelectionProvider::SetSelection(ISelection::Pointer /*selection*/)
{
}

}
