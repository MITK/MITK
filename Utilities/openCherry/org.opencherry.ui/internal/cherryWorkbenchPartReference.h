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

#ifndef CHERRYWORKBENCHPARTREFERENCE_H_
#define CHERRYWORKBENCHPARTREFERENCE_H_

#include "../cherryIWorkbenchPartReference.h"
#include "../cherryIWorkbenchPart.h"
#include "../cherryIPartPane.h"

namespace cherry {

class WorkbenchPartReference : virtual public IWorkbenchPartReference {

    /**
     * Internal property ID: Indicates that the underlying part was created
     */
public: static const int INTERNAL_PROPERTY_OPENED; // = 0x211;
    
    /**
     * Internal property ID: Indicates that the underlying part was destroyed
     */
public: static const int INTERNAL_PROPERTY_CLOSED; // = 0x212;

    /**
     * Internal property ID: Indicates that the result of IEditorReference.isPinned()
     */
public: static const int INTERNAL_PROPERTY_PINNED; // = 0x213;

    /**
     * Internal property ID: Indicates that the result of getVisible() has changed
     */
public: static const int INTERNAL_PROPERTY_VISIBLE; // = 0x214;

    /**
     * Internal property ID: Indicates that the result of isZoomed() has changed
     */
//public: static const int INTERNAL_PROPERTY_ZOOMED = 0x215;

    /**
     * Internal property ID: Indicates that the part has an active child and the
     * active child has changed. (fired by PartStack)
     */
public: static const int INTERNAL_PROPERTY_ACTIVE_CHILD_CHANGED; // = 0x216;

    /**
     * Internal property ID: Indicates that changed in the min / max
     * state has changed
     */
//public: static final int INTERNAL_PROPERTY_MAXIMIZED = 0x217;

    // State constants //////////////////////////////
    
    /**
     * State constant indicating that the part is not created yet
     */
public: static int STATE_LAZY; // = 0
     
    /**
     * State constant indicating that the part is in the process of being created
     */
public: static int STATE_CREATION_IN_PROGRESS; // = 1
    
    /**
     * State constant indicating that the part has been created
     */
public: static int STATE_CREATED; // = 2
    
    /**
     * State constant indicating that the reference has been disposed (the reference shouldn't be
     * used anymore)
     */
public: static int STATE_DISPOSED; // = 3
  
    /**
     * Current state of the reference. Used to detect recursive creation errors, disposed
     * references, etc. 
     */
private: int state;
   
protected: IWorkbenchPart::Pointer part;

protected: IPartPane::Pointer pane;

private: std::string id;

private: bool pinned;

private: std::string tooltip;

    /**
     * Stores the current Image for this part reference. Lazily created. Null if not allocated.
     */
//private: Image image;
    
//private: ImageDescriptor defaultImageDescriptor;
    
    /**
     * Stores the current image descriptor for the part. 
     */
//private: ImageDescriptor imageDescriptor;

    /**
     * API listener list
     */
//private: ListenerList propChangeListeners;

    /**
     * Internal listener list. Listens to the INTERNAL_PROPERTY_* property change events that are not yet API.
     * TODO: Make these properties API in 3.2
     */
//private: ListenerList internalPropChangeListeners = new ListenerList();
    
//private: ListenerList partChangeListeners = new ListenerList();
    
private: std::string partName;

private: std::string contentDescription;
    
//protected: Map propertyCache = new HashMap();
    
    /**
     * Used to remember which events have been queued.
     */
//private: BitSet queuedEvents;

//private: bool queueEvents;

//private: static DisposeListener prematureDisposeListener = new DisposeListener() {
//        public void widgetDisposed(DisposeEvent e) {
//            WorkbenchPlugin.log(new RuntimeException("Widget disposed too early!")); //$NON-NLS-1$
//        }    
//    };
    
//private: IPropertyListener propertyChangeListener = new IPropertyListener() {
//        /* (non-Javadoc)
//         * @see org.eclipse.ui.IPropertyListener#propertyChanged(java.lang.Object, int)
//         */
//        public void propertyChanged(Object source, int propId) {
//            partPropertyChanged(source, propId);
//        }
//    };
    
//private: IPropertyChangeListener partPropertyChangeListener = new IPropertyChangeListener() {
//    public void propertyChange(PropertyChangeEvent event) {
//      partPropertyChanged(event);
//    }
//    };
    
public: WorkbenchPartReference();
    
public: virtual bool IsDisposed();
    
protected: virtual void CheckReference();
    
    /**
     * Calling this with deferEvents(true) will queue all property change events until a subsequent
     * call to deferEvents(false). This should be used at the beginning of a batch of related changes
     * to prevent duplicate property change events from being sent.
     * 
     * @param shouldQueue
     */
//private: virtual void DeferEvents(bool shouldQueue);

protected: virtual void SetPartName(const std::string& newPartName);

protected: virtual void SetContentDescription(const std::string& newContentDescription);

//protected: virtual void SetImageDescriptor(ImageDescriptor descriptor);
    
protected: virtual void SetToolTip(const std::string& newToolTip);

//protected: virtual void partPropertyChanged(Object source, int propId);
    
//protected: virtual void partPropertyChanged(PropertyChangeEvent event);

    /**
     * Refreshes all cached values with the values from the real part 
     */
protected: virtual void RefreshFromPart();
    
//protected: virtual ImageDescriptor ComputeImageDescriptor();

public: virtual void Init(const std::string& id, const std::string& tooltip,
            /*ImageDescriptor desc,*/ const std::string& paneName, const std::string& contentDescription);

    /**
     * Releases any references maintained by this part reference
     * when its actual part becomes known (not called when it is disposed).
     */
protected: virtual void ReleaseReferences();

//protected: virtual void addInternalPropertyListener(IPropertyListener listener);
    
//protected: virtual void removeInternalPropertyListener(IPropertyListener listener);

//protected: virtual void fireInternalPropertyChange(int id);
    
    /**
     * @see IWorkbenchPart
     */
//public: virtual void addPropertyListener(IPropertyListener listener);

    /**
     * @see IWorkbenchPart
     */
//public: virtual void removePropertyListener(IPropertyListener listener);

public: std::string GetId();

public: virtual std::string GetTitleToolTip();

protected: std::string GetRawToolTip();

    /**
     * Returns the pane name for the part
     * 
     * @return the pane name for the part
     */
public: virtual std::string GetPartName();
    
    /**
     * Gets the part name directly from the associated workbench part,
     * or the empty string if none.
     * 
     * @return
     */
protected: std::string GetRawPartName();

protected: virtual std::string ComputePartName();

    /**
     * Returns the content description for this part.
     * 
     * @return the pane name for the part
     */
public: virtual std::string GetContentDescription();

    /**
     * Computes a new content description for the part. Subclasses may override to change the
     * default behavior
     * 
     * @return the new content description for the part
     */
protected: virtual std::string ComputeContentDescription();

    /**
     * Returns the content description as set directly by the part, or the empty string if none
     * 
     * @return the unmodified content description from the part (or the empty string if none)
     */
protected: std::string GetRawContentDescription();

public: virtual bool IsDirty();

//public: virtual Image GetTitleImage();
    
//public: virtual ImageDescriptor GetTitleImageDescriptor();
    
//    /* package */ virtual void fireVisibilityChange();

//    /* package */ virtual void fireZoomChange();
    
public: virtual bool GetVisible();
    
public: virtual void SetVisible(bool isVisible);
    
//protected: virtual void firePropertyChange(int id);
    
//private: virtual void immediateFirePropertyChange(int id);

public: IWorkbenchPart::Pointer GetPart(bool restore);
    
protected: virtual IWorkbenchPart::Pointer CreatePart() = 0;

protected: virtual IPartPane::Pointer CreatePane() = 0;
    
    /**
     * Returns the part pane for this part reference. Does not return null. 
     * 
     * @return
     */
public: IPartPane::Pointer GetPane();

public: void Dispose();

  /**
   * Clears all of the listeners in a listener list. TODO Bug 117519 Remove
   * this method when fixed.
   * 
   * @param list
   *            The list to be clear; must not be <code>null</code>.
   */
//private: void clearListenerList(const ListenerList list);


public: virtual void SetPinned(bool newPinned);
    
public: virtual bool IsPinned();

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchPartReference#getPartProperty(java.lang.String)
     */
//public: virtual std::string GetPartProperty(const std::string& key);
    
    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchPartReference#addPartPropertyListener(org.eclipse.jface.util.IPropertyChangeListener)
     */
//public: virtual void addPartPropertyListener(IPropertyChangeListener listener);
    
    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchPartReference#removePartPropertyListener(org.eclipse.jface.util.IPropertyChangeListener)
     */
//public: virtual void removePartPropertyListener(IPropertyChangeListener listener);
    
//protected: virtual void firePartPropertyChange(PropertyChangeEvent event);
    
//protected: virtual void createPartProperties(IWorkbenchPart3 workbenchPart);
        
};

} // namespace cherry

#endif /*CHERRYWORKBENCHPARTREFERENCE_H_*/
