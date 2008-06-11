#ifndef CHERRYPARTLIST_H_
#define CHERRYPARTLIST_H_

#include "cherryWorkbenchPartReference.h"
#include "../cherryIEditorReference.h"

namespace cherry
{

class PartList {
  
private:
  
  IWorkbenchPartReference::Pointer activePartReference;

  IEditorReference::Pointer activeEditorReference;
  
  void PropertyChanged(WorkbenchPartReference::Pointer ref, int propId);

public:
  
  IWorkbenchPartReference::Pointer GetActivePartReference();

  IEditorReference::Pointer GetActiveEditorReference();

  IEditorPart::Pointer GetActiveEditor();

  IWorkbenchPart::Pointer GetActivePart();

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

private:
  
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
