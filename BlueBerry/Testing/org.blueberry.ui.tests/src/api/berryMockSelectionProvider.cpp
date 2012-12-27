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
