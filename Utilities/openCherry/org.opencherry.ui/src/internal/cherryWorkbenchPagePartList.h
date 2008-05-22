/*=========================================================================
 
Program:   openCherry Platform
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

#ifndef CHERRYWORKBENCHPAGEPARTLIST_H_
#define CHERRYWORKBENCHPAGEPARTLIST_H_

#include <org.opencherry.osgi/cherrySmartPointer.h>

namespace cherry {

class IWorkbenchPartReference;
class WorkbenchPartReference;
class IWorkbenchPart;
class IEditorPart;
class IEditorReference;
class PageSelectionService;

class WorkbenchPagePartList {
    
private:
  
  PageSelectionService* selectionService;
    
  PartService partService; // = new PartService(UIListenerLogging.PAGE_PARTLISTENER_EVENTS, 
            //UIListenerLogging.PAGE_PARTLISTENER2_EVENTS);
  
 SmartPointer<IWorkbenchPartReference> activePartReference;

  SmartPointer<IEditorReference> activeEditorReference;

  // private List parts = new ArrayList();

//  IPropertyListener listener = new IPropertyListener() {
//    public void propertyChanged(Object source, int propId) {
//      WorkbenchPartReference ref = (WorkbenchPartReference) source;
//
//      switch (propId) {
//      case WorkbenchPartReference.INTERNAL_PROPERTY_OPENED:
//        partOpened(ref);
//        break;
//      case WorkbenchPartReference.INTERNAL_PROPERTY_CLOSED:
//        partClosed(ref);
//        break;
//      case WorkbenchPartReference.INTERNAL_PROPERTY_VISIBLE: {
//        if (ref.getVisible()) {
//          partVisible(ref);
//        } else {
//          partHidden(ref);
//        }
//        break;
//      }
//      case IWorkbenchPartConstants.PROP_INPUT: {
//        partInputChanged(ref);
//        break;
//      }
//      }
//    }
//  };

  
public:
  
  SmartPointer<IWorkbenchPartReference> GetActivePartReference();

  SmartPointer<IEditorReference> GetActiveEditorReference();

  SmartPointer<IEditorPart> GetActiveEditor();

  SmartPointer<IWorkbenchPart> GetActivePart();

  void AddPart(SmartPointer<WorkbenchPartReference> ref);

  /**
   * Sets the active part.
   * 
   * @param ref
   */
  void SetActivePart(SmartPointer<IWorkbenchPartReference> ref);

  void SetActiveEditor(SmartPointer<IEditorReference> ref);

  /**
   * In order to remove a part, it must first be deactivated.
   */
  void RemovePart(SmartPointer<WorkbenchPartReference> ref);
  
private:
  
  void PartInputChanged(SmartPointer<WorkbenchPartReference> ref);

  void PartHidden(SmartPointer<WorkbenchPartReference> ref);

  void PartOpened(SmartPointer<WorkbenchPartReference> ref);

  /**
   * Called when a concrete part is about to be destroyed. This is called
   * BEFORE disposal happens, so the part should still be accessable from the
   * part reference.
   * 
   * @param ref
   */
  void PartClosed(SmartPointer<WorkbenchPartReference> ref);

  void PartVisible(SmartPointer<WorkbenchPartReference> ref);
  
  
public:
  
  WorkbenchPagePartList(PageSelectionService* selectionService);
    
  IPartService* GetPartService();
  
  
protected:
  
  void FirePartOpened(SmartPointer<IWorkbenchPartReference> part);

    void FirePartClosed(SmartPointer<IWorkbenchPartReference> part);

    void FirePartAdded(SmartPointer<IWorkbenchPartReference> part);

    void FirePartRemoved(SmartPointer<IWorkbenchPartReference> part);

    void FireActiveEditorChanged(SmartPointer<IWorkbenchPartReference> ref);

    void FireActivePartChanged(SmartPointer<IWorkbenchPartReference> oldRef, SmartPointer<IWorkbenchPartReference> newRef);
    
    void FirePartHidden(SmartPointer<IWorkbenchPartReference> ref);

    void FirePartVisible(SmartPointer<IWorkbenchPartReference> ref);
    
    void FirePartInputChanged(SmartPointer<IWorkbenchPartReference> ref);

    
public:
  
  void FirePartBroughtToTop(SmartPointer<IWorkbenchPartReference> ref);
};

}

#endif /*CHERRYWORKBENCHPAGEPARTLIST_H_*/
