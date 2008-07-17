#ifndef CHERRYSELECTIONCHANGEDEVENT_H_
#define CHERRYSELECTIONCHANGEDEVENT_H_

#include "cherryISelectionProvider.h"
#include "cherryISelection.h"

namespace cherry
{

/**
 * \ingroup org_opencherry_ui
 * 
 * Event object describing a selection change. The source of these
 * events is a selection provider.
 *
 * @see ISelection
 * @see ISelectionProvider
 * @see ISelectionChangedListener
 */
class CHERRY_UI SelectionChangedEvent : public Object
{

public:

  cherryClassMacro(SelectionChangedEvent);

  /**
   * Creates a new event for the given source and selection.
   *
   * @param source the selection provider
   * @param selection the selection
   */
  SelectionChangedEvent(ISelectionProvider::Pointer source,
      ISelection::Pointer selection);

  ISelectionProvider::Pointer GetSource();

  /**
   * Returns the selection.
   *
   * @return the selection
   */
  ISelection::Pointer GetSelection();

  /**
   * Returns the selection provider that is the source of this event.
   *
   * @return the originating selection provider
   */
  ISelectionProvider::Pointer GetSelectionProvider();

protected:

  /**
   * The selection.
   */
  ISelection::Pointer selection;

private:

  ISelectionProvider::Pointer source;
};

}

#endif /*CHERRYSELECTIONCHANGEDEVENT_H_*/
