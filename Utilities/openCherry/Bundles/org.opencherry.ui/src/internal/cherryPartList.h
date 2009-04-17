#ifndef CHERRYPARTLIST_H_
#define CHERRYPARTLIST_H_

#include "cherryWorkbenchPartReference.h"
#include "cherryIStackableContainer.h"

#include "../cherryIEditorReference.h"
#include "../cherryIViewReference.h"

#include <deque>

namespace cherry
{

class PartList : public IPropertyChangeListener {

private:

  // list of parts in the activation order (oldest last)
  //std::deque<IWorkbenchPartReference::Pointer> parts;

  IWorkbenchPartReference::Pointer activePartReference;
  IEditorReference::Pointer activeEditorReference;

public:

  void PropertyChange(Object::Pointer source, int propId);

  IWorkbenchPartReference::Pointer GetActivePartReference();

  IEditorReference::Pointer GetActiveEditorReference();

  IEditorPart::Pointer GetActiveEditor();

  IWorkbenchPart::Pointer GetActivePart();

  //std::vector<IEditorReference::Pointer> GetEditors();

  void AddPart(WorkbenchPartReference::Pointer ref);

  /**
   * Sets the active part.
   *
   * @param ref
   */
  void SetActivePart(IWorkbenchPartReference::Pointer ref);

  void SetActiveEditor(IEditorReference::Pointer ref);

  /**
   * In order to remove a part, it must first be deactivated.
   */
  void RemovePart(WorkbenchPartReference::Pointer ref);

  //int IndexOf(const IWorkbenchPartReference::Pointer ref) const;

  /*
   * Ensures that the given part appears AFTER any other part in the same
   * container.
   */
  //void BringToTop(IWorkbenchPartReference::Pointer ref);

  /*
   * Return a list with all parts (editors and views).
   */
  //std::vector<IWorkbenchPartReference::Pointer> GetParts(const std::vector<IViewReference::Pointer>& views);


private:

  /*
   * Returns the last (most recent) index of the given container in the activation list, or returns
   * -1 if the given container does not appear in the activation list.
   */
  //std::deque<IWorkbenchPartReference::Pointer>::iterator
  //  LastIndexOfContainer(IStackableContainer::Pointer container);

  void PartInputChanged(WorkbenchPartReference::Pointer ref);

  void PartHidden(WorkbenchPartReference::Pointer ref);

  void PartOpened(WorkbenchPartReference::Pointer ref);

  /**
   * Called when a concrete part is about to be destroyed. This is called
   * BEFORE disposal happens, so the part should still be accessable from the
   * part reference.
   *
   * @param ref
   */
  void PartClosed(WorkbenchPartReference::Pointer ref);

  void PartVisible(WorkbenchPartReference::Pointer ref);

  //bool Contains(IWorkbenchPartReference::Pointer ref);

protected:

  /**
   * Fire the event indicating that a part reference was just realized. That
   * is, the concrete IWorkbenchPart has been attached to the part reference.
   *
   * @param part
   *            the reference that was create
   */
  virtual void FirePartOpened(IWorkbenchPartReference::Pointer part) = 0;

  /**
   * Fire the event indicating that a part reference was just realized. That
   * is, the concrete IWorkbenchPart has been attached to the part reference.
   *
   * @param part
   *            the reference that was create
   */
  virtual void FirePartClosed(IWorkbenchPartReference::Pointer part) = 0;

  /**
   * Indicates that a new part reference was added to the list.
   *
   * @param part
   */
  virtual void FirePartAdded(IWorkbenchPartReference::Pointer part) = 0;

  /**
   * Indicates that a part reference was removed from the list
   *
   * @param part
   */
  virtual void FirePartRemoved(IWorkbenchPartReference::Pointer part) = 0;

  /**
   * Indicates that the active editor changed
   *
   * @param part
   *            active part reference or null if none
   */
  virtual void FireActiveEditorChanged(IWorkbenchPartReference::Pointer ref) = 0;

  /**
   * Indicates that the active part has changed
   *
   * @param part
   *            active part reference or null if none
   */
  virtual void FireActivePartChanged(
      IWorkbenchPartReference::Pointer oldPart, IWorkbenchPartReference::Pointer newPart) = 0;

  /**
   * Indicates that the part has been made visible
   *
   * @param ref
   */
  virtual void FirePartVisible(IWorkbenchPartReference::Pointer ref) = 0;

  /**
   * Indicates that the part has been hidden
   *
   * @param ref
   */
  virtual void FirePartHidden(IWorkbenchPartReference::Pointer ref) = 0;

  /**
   * Indicates that the part input has changed
   *
   * @param ref
   */
  virtual void FirePartInputChanged(IWorkbenchPartReference::Pointer ref) = 0;

  virtual void FirePartBroughtToTop(IWorkbenchPartReference::Pointer ref) = 0;
};

}

#endif /*CHERRYPARTLIST_H_*/
