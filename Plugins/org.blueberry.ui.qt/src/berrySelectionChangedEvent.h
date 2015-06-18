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
#ifndef BERRYSELECTIONCHANGEDEVENT_H_
#define BERRYSELECTIONCHANGEDEVENT_H_

#include "berryISelection.h"

namespace berry
{

struct ISelectionProvider;

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Event object describing a selection change. The source of these
 * events is a selection provider.
 *
 * @see ISelection
 * @see ISelectionProvider
 * @see ISelectionChangedListener
 */
class BERRY_UI_QT SelectionChangedEvent : public Object
{

public:

  berryObjectMacro(SelectionChangedEvent);

  /**
   * Creates a new event for the given source and selection.
   *
   * @param source the selection provider
   * @param selection the selection
   */
  SelectionChangedEvent(SmartPointer<ISelectionProvider> source,
      ISelection::ConstPointer selection);

  SmartPointer<ISelectionProvider> GetSource() const;

  /**
   * Returns the selection.
   *
   * @return the selection
   */
  ISelection::ConstPointer GetSelection() const;

  /**
   * Returns the selection provider that is the source of this event.
   *
   * @return the originating selection provider
   */
  SmartPointer<ISelectionProvider> GetSelectionProvider() const;

protected:

  /**
   * The selection.
   */
  ISelection::ConstPointer selection;

private:

  SmartPointer<ISelectionProvider> source;
};

}

#endif /*BERRYSELECTIONCHANGEDEVENT_H_*/
