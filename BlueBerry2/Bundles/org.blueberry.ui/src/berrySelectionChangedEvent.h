#ifndef BERRYSELECTIONCHANGEDEVENT_H_
#define BERRYSELECTIONCHANGEDEVENT_H_

#include "berryISelection.h"

namespace berry
{

using namespace osgi::framework;

struct ISelectionProvider;

/**
 * \ingroup org_blueberry_ui
 *
 * Event object describing a selection change. The source of these
 * events is a selection provider.
 *
 * @see ISelection
 * @see ISelectionProvider
 * @see ISelectionChangedListener
 */
class BERRY_UI SelectionChangedEvent : public Object
{

public:

  osgiObjectMacro(SelectionChangedEvent);

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
