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

namespace cherry {

    /**
     * Internal property ID: Indicates that the underlying part was created
     */
static const int WorkbenchPartReference::INTERNAL_PROPERTY_OPENED = 0x211;
    
    /**
     * Internal property ID: Indicates that the underlying part was destroyed
     */
static const int WorkbenchPartReference::INTERNAL_PROPERTY_CLOSED = 0x212;

    /**
     * Internal property ID: Indicates that the result of IEditorReference.isPinned()
     */
static const int WorkbenchPartReference::INTERNAL_PROPERTY_PINNED = 0x213;

    /**
     * Internal property ID: Indicates that the result of getVisible() has changed
     */
static const int WorkbenchPartReference::INTERNAL_PROPERTY_VISIBLE = 0x214;

    /**
     * Internal property ID: Indicates that the result of isZoomed() has changed
     */
//static const int WorkbenchPartReference::INTERNAL_PROPERTY_ZOOMED = 0x215;

    /**
     * Internal property ID: Indicates that the part has an active child and the
     * active child has changed. (fired by PartStack)
     */
static const int WorkbenchPartReference::INTERNAL_PROPERTY_ACTIVE_CHILD_CHANGED = 0x216;

    /**
     * Internal property ID: Indicates that changed in the min / max
     * state has changed
     */
//static final int WorkbenchPartReference::INTERNAL_PROPERTY_MAXIMIZED = 0x217;

    // State constants //////////////////////////////
    
    /**
     * State constant indicating that the part is not created yet
     */
static int WorkbenchPartReference::STATE_LAZY = 0;
     
    /**
     * State constant indicating that the part is in the process of being created
     */
static int WorkbenchPartReference::STATE_CREATION_IN_PROGRESS = 1;
    
    /**
     * State constant indicating that the part has been created
     */
static int WorkbenchPartReference::STATE_CREATED = 2;
    
    /**
     * State constant indicating that the reference has been disposed (the reference shouldn't be
     * used anymore)
     */
static int WorkbenchPartReference::STATE_DISPOSED = 3;
  


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
    
WorkbenchPartReference::WorkbenchPartReference()
: queueEvents(false), pinned(false), state(STATE_LAZY) {
        //no-op
    }
    
bool WorkbenchPartReference::IsDisposed() {
        return state == STATE_DISPOSED;
    }
    
void WorkbenchPartReference::CheckReference() {
        if (state == STATE_DISPOSED) {
            throw new RuntimeException("Error: IWorkbenchPartReference disposed"); //$NON-NLS-1$
        }
    }
    
    /**
     * Calling this with deferEvents(true) will queue all property change events until a subsequent
     * call to deferEvents(false). This should be used at the beginning of a batch of related changes
     * to prevent duplicate property change events from being sent.
     * 
     * @param shouldQueue
     */
//void WorkbenchPartReference::DeferEvents(bool shouldQueue) {
//        queueEvents = shouldQueue;
//
//        if (queueEvents == false) {
//          // do not use nextSetBit, to allow compilation against JCL Foundation (bug 80053)
//          for (int i = 0, n = queuedEvents.size(); i < n; ++i) {
//            if (queuedEvents.get(i)) {
//              firePropertyChange(i);
//              queuedEvents.clear(i);
//            }
//            }
//        }
//    }

void WorkbenchPartReference::SetTitle(const std::string& newTitle) {
        if (Util.equals(title, newTitle)) {
            return;
        }

        title = newTitle;
        firePropertyChange(IWorkbenchPartConstants.PROP_TITLE);
    }

void WorkbenchPartReference::SetPartName(const std::string& newPartName) {
        if (Util.equals(partName, newPartName)) {
            return;
        }

        partName = newPartName;
        firePropertyChange(IWorkbenchPartConstants.PROP_PART_NAME);
    }

void WorkbenchPartReference::SetContentDescription(const std::string& newContentDescription) {
        if (Util.equals(contentDescription, newContentDescription)) {
            return;
        }

        contentDescription = newContentDescription;
        firePropertyChange(IWorkbenchPartConstants.PROP_CONTENT_DESCRIPTION);
    }

void WorkbenchPartReference::SetImageDescriptor(ImageDescriptor descriptor) {
        if (Util.equals(imageDescriptor, descriptor)) {
            return;
        }

        Image oldImage = image;
        ImageDescriptor oldDescriptor = imageDescriptor;
        image = null;
        imageDescriptor = descriptor;
        
        // Don't queue events triggered by image changes. We'll dispose the image
        // immediately after firing the event, so we need to fire it right away.
        immediateFirePropertyChange(IWorkbenchPartConstants.PROP_TITLE);
        if (queueEvents) {
            // If there's a PROP_TITLE event queued, remove it from the queue because
            // we've just fired it.
            queuedEvents.clear(IWorkbenchPartConstants.PROP_TITLE);
        }
        
        // If we had allocated the old image, deallocate it now (AFTER we fire the property change 
        // -- listeners may need to clean up references to the old image)
        if (oldImage != null) {
            JFaceResources.getResources().destroy(oldDescriptor);
        }
    }
    
void WorkbenchPartReference::SetToolTip(const std::string& newToolTip) {
        if (Util.equals(tooltip, newToolTip)) {
            return;
        }

        tooltip = newToolTip;
        firePropertyChange(IWorkbenchPartConstants.PROP_TITLE);
    }

void WorkbenchPartReference::partPropertyChanged(Object source, int propId) {

        // We handle these properties directly (some of them may be transformed
        // before firing events to workbench listeners)
        if (propId == IWorkbenchPartConstants.PROP_CONTENT_DESCRIPTION
                || propId == IWorkbenchPartConstants.PROP_PART_NAME
                || propId == IWorkbenchPartConstants.PROP_TITLE) {

            refreshFromPart();
        } else {
            // Any other properties are just reported to listeners verbatim
            firePropertyChange(propId);
        }
        
        // Let the model manager know as well
        if (propId == IWorkbenchPartConstants.PROP_DIRTY) {
          IWorkbenchPart actualPart = getPart(false);
          if (actualPart != null) {
        SaveablesList modelManager = (SaveablesList) actualPart.getSite().getService(ISaveablesLifecycleListener.class);
            modelManager.dirtyChanged(actualPart);
          }
        }
    }
    
void WorkbenchPartReference::partPropertyChanged(PropertyChangeEvent event) {
      firePartPropertyChange(event);
    }

    /**
     * Refreshes all cached values with the values from the real part 
     */
void WorkbenchPartReference::refreshFromPart() {
        deferEvents(true);

        setPartName(computePartName());
        setTitle(computeTitle());
        setContentDescription(computeContentDescription());
        setToolTip(getRawToolTip());
        setImageDescriptor(computeImageDescriptor());

        deferEvents(false);
    }
    
ImageDescriptor WorkbenchPartReference::computeImageDescriptor() {
        if (part != null) {
            return ImageDescriptor.createFromImage(part.getTitleImage(), Display.getCurrent());
        }
        return defaultImageDescriptor;
    }

void WorkbenchPartReference::init(const std::string& id, const  std::string& title, const std::string& tooltip,
            ImageDescriptor desc, const std::string& paneName, const std::string& contentDescription) {
        Assert.isNotNull(id);
        Assert.isNotNull(title);
        Assert.isNotNull(tooltip);
        Assert.isNotNull(desc);
        Assert.isNotNull(paneName);
        Assert.isNotNull(contentDescription);
        
        this.id = id;
        this.title = title;
        this.tooltip = tooltip;
        this.partName = paneName;
        this.contentDescription = contentDescription;
        this.defaultImageDescriptor = desc;
        this.imageDescriptor = computeImageDescriptor();
    }

    /**
     * Releases any references maintained by this part reference
     * when its actual part becomes known (not called when it is disposed).
     */
void WorkbenchPartReference::releaseReferences() {

    }

//protected: void addInternalPropertyListener(IPropertyListener listener) {
//        internalPropChangeListeners.add(listener);
//    }
    
//protected: void removeInternalPropertyListener(IPropertyListener listener) {
//        internalPropChangeListeners.remove(listener);
//    }

//protected: void fireInternalPropertyChange(int id) {
//        Object listeners[] = internalPropChangeListeners.getListeners();
//        for (int i = 0; i < listeners.length; i++) {
//            ((IPropertyListener) listeners[i]).propertyChanged(this, id);
//        }
//    }
    
    /**
     * @see IWorkbenchPart
     */
//public: void addPropertyListener(IPropertyListener listener) {
//        // The properties of a disposed reference will never change, so don't
//        // add listeners
//        if (isDisposed()) {
//            return;
//        }
//        
//        propChangeListeners.add(listener);
//    }

    /**
     * @see IWorkbenchPart
     */
//public: void removePropertyListener(IPropertyListener listener) {
//        // Currently I'm not calling checkReference here for fear of breaking things late in 3.1, but it may
//        // make sense to do so later. For now we just turn it into a NOP if the reference is disposed.
//        if (isDisposed()) {
//            return;
//        }
//        propChangeListeners.remove(listener);
//    }

const std::string& WorkbenchPartReference::getId() {
        if (part != null) {
            IWorkbenchPartSite site = part.getSite();
            if (site != null) {
        return site.getId();
      }
        }
        return Util.safeString(id);
    }

const std::string& WorkbenchPartReference::getTitleToolTip() {
        return Util.safeString(tooltip);
    }

const std::string& WorkbenchPartReference::getRawToolTip() {
        return Util.safeString(part.getTitleToolTip());
    }

    /**
     * Returns the pane name for the part
     * 
     * @return the pane name for the part
     */
const std::string& WorkbenchPartReference::getPartName() {
        return Util.safeString(partName);
    }
    
    /**
     * Gets the part name directly from the associated workbench part,
     * or the empty string if none.
     * 
     * @return
     */
const std::string& WorkbenchPartReference::getRawPartName() {
        String result = ""; //$NON-NLS-1$

        if (part instanceof IWorkbenchPart2) {
            IWorkbenchPart2 part2 = (IWorkbenchPart2) part;

            result = Util.safeString(part2.getPartName());
        }

        return result;
    }

const std::string& WorkbenchPartReference::computePartName() {
        return getRawPartName();
    }

    /**
     * Returns the content description for this part.
     * 
     * @return the pane name for the part
     */
const std::string& WorkbenchPartReference::getContentDescription() {
        return Util.safeString(contentDescription);
    }

    /**
     * Computes a new content description for the part. Subclasses may override to change the
     * default behavior
     * 
     * @return the new content description for the part
     */
const std::string& WorkbenchPartReference::computeContentDescription() {
        return getRawContentDescription();
    }

    /**
     * Returns the content description as set directly by the part, or the empty string if none
     * 
     * @return the unmodified content description from the part (or the empty string if none)
     */
const std::string& WorkbenchPartReference::getRawContentDescription() {
        if (part instanceof IWorkbenchPart2) {
            IWorkbenchPart2 part2 = (IWorkbenchPart2) part;

            return part2.getContentDescription();
        }

        return ""; //$NON-NLS-1$        
    }

bool WorkbenchPartReference::isDirty() {
        if (!(part instanceof ISaveablePart)) {
      return false;
    }
        return ((ISaveablePart) part).isDirty();
    }

const std::string& WorkbenchPartReference::getTitle() {
        return Util.safeString(title);
    }

    /**
     * Computes a new title for the part. Subclasses may override to change the default behavior.
     * 
     * @return the title for the part
     */
const std::string& WorkbenchPartReference::computeTitle() {
        return getRawTitle();
    }

    /**
     * Returns the unmodified title for the part, or the empty string if none
     * 
     * @return the unmodified title, as set by the IWorkbenchPart. Returns the empty string if none.
     */
const std::string& WorkbenchPartReference::getRawTitle() {
        return Util.safeString(part.getTitle());
    }

Image WorkbenchPartReference::getTitleImage() {
        if (isDisposed()) {
            return PlatformUI.getWorkbench().getSharedImages().getImage(ISharedImages.IMG_DEF_VIEW);
        }
        
        if (image == null) {        
            image = JFaceResources.getResources().createImageWithDefault(imageDescriptor);
        }
        return image;
    }
    
ImageDescriptor WorkbenchPartReference::getTitleImageDescriptor() {
        if (isDisposed()) {
            return PlatformUI.getWorkbench().getSharedImages().getImageDescriptor(ISharedImages.IMG_DEF_VIEW);
        }
        
        return imageDescriptor;
    }
    
//    /* package */ void fireVisibilityChange() {
//        fireInternalPropertyChange(INTERNAL_PROPERTY_VISIBLE);
//    }

//    /* package */ void fireZoomChange() {
//        fireInternalPropertyChange(INTERNAL_PROPERTY_ZOOMED);
//    }
    
bool WorkbenchPartReference::getVisible() {
        if (isDisposed()) {
            return false;
        }
        return getPane().getVisible();
    }
    
void WorkbenchPartReference::setVisible(bool isVisible) {
        if (isDisposed()) {
            return;
        }
        getPane().setVisible(isVisible);
    }
    
//protected: void firePropertyChange(int id) {
//
//        if (queueEvents) {
//            queuedEvents.set(id);
//            return;
//        }
//        
//        immediateFirePropertyChange(id);
//    }
    
//private: void immediateFirePropertyChange(int id) {
//        UIListenerLogging.logPartReferencePropertyChange(this, id);
//        Object listeners[] = propChangeListeners.getListeners();
//        for (int i = 0; i < listeners.length; i++) {
//            ((IPropertyListener) listeners[i]).propertyChanged(part, id);
//        }
//        
//        fireInternalPropertyChange(id);
//    }

IWorkbenchPart::ConstPtr WorkbenchPartReference::getPart(bool restore) {
        if (isDisposed()) {
            return null;
        }
        
        if (part == null && restore) {
            
            if (state == STATE_CREATION_IN_PROGRESS) {
                IStatus result = WorkbenchPlugin.getStatus(
                        new PartInitException(NLS.bind("Warning: Detected recursive attempt by part {0} to create itself (this is probably, but not necessarily, a bug)",  //$NON-NLS-1$
                                getId())));
                WorkbenchPlugin.log(result);
                return null;
            }
            
            try {
                state = STATE_CREATION_IN_PROGRESS;
                
                IWorkbenchPart newPart = createPart();
                if (newPart != null) {
                    part = newPart;
                    // Add a dispose listener to the part. This dispose listener does nothing but log an exception
                    // if the part's widgets get disposed unexpectedly. The workbench part reference is the only
                    // object that should dispose this control, and it will remove the listener before it does so.
                    getPane().getControl().addDisposeListener(prematureDisposeListener);
                    part.addPropertyListener(propertyChangeListener);
                    if (part instanceof IWorkbenchPart3) {
                      ((IWorkbenchPart3)part).addPartPropertyListener(partPropertyChangeListener);
                    }

                    refreshFromPart();
                    releaseReferences();
                    
                    fireInternalPropertyChange(INTERNAL_PROPERTY_OPENED);
                    
                    ISizeProvider sizeProvider = (ISizeProvider) Util.getAdapter(part, ISizeProvider.class);
                    if (sizeProvider != null) {
                        // If this part has a preferred size, indicate that the preferred size may have changed at this point
                        if (sizeProvider.getSizeFlags(true) != 0 || sizeProvider.getSizeFlags(false) != 0) {
                            fireInternalPropertyChange(IWorkbenchPartConstants.PROP_PREFERRED_SIZE);
                        }
                    }
                }
            } finally {
                state = STATE_CREATED;
            }
        }        
        
        return part;
    }
    
    
    /**
     * Returns the part pane for this part reference. Does not return null. Should not be called
     * if the reference has been disposed.
     * 
     * TODO: clean up all code that has any possibility of calling this on a disposed reference
     * and make this method throw an exception if anyone else attempts to do so.
     * 
     * @return
     */
PartPane WorkbenchPartReference::getPane() {
        
        // Note: we should never call this if the reference has already been disposed, since it
        // may cause a PartPane to be created and leaked.
        if (pane == null) {
            pane = createPane();
        }
        return pane;
    }

void WorkbenchPartReference::dispose() {
        
        if (isDisposed()) {
            return;
        }
        
        // Store the current title, tooltip, etc. so that anyone that they can be returned to
        // anyone that held on to the disposed reference.
        partName = getPartName();
        contentDescription = getContentDescription();
        tooltip = getTitleToolTip();
        title = getTitle();
        
        if (state == STATE_CREATION_IN_PROGRESS) {
            IStatus result = WorkbenchPlugin.getStatus(
                    new PartInitException(NLS.bind("Warning: Blocked recursive attempt by part {0} to dispose itself during creation",  //$NON-NLS-1$
                            getId())));
            WorkbenchPlugin.log(result);
            return;
        }
        
      // Disposing the pane disposes the part's widgets. The part's widgets need to be disposed before the part itself.
        if (pane != null) {
            // Remove the dispose listener since this is the correct place for the widgets to get disposed
            Control targetControl = getPane().getControl(); 
            if (targetControl != null) {
                targetControl.removeDisposeListener(prematureDisposeListener);
            }
            pane.dispose();
        }
        
        doDisposePart();
   
        if (pane != null) {
          pane.removeContributions();
        }
        
        clearListenerList(internalPropChangeListeners);
        clearListenerList(partChangeListeners);
        Image oldImage = image;
        ImageDescriptor oldDescriptor = imageDescriptor;
        image = null;
        
        state = STATE_DISPOSED;
        imageDescriptor = ImageDescriptor.getMissingImageDescriptor();
        defaultImageDescriptor = ImageDescriptor.getMissingImageDescriptor();
        immediateFirePropertyChange(IWorkbenchPartConstants.PROP_TITLE);
        clearListenerList(propChangeListeners);
        
        if (oldImage != null) {
            JFaceResources.getResources().destroy(oldDescriptor);
        }
    }

  /**
   * Clears all of the listeners in a listener list. TODO Bug 117519 Remove
   * this method when fixed.
   * 
   * @param list
   *            The list to be clear; must not be <code>null</code>.
   */
//private: void clearListenerList(const ListenerList list) {
//    final Object[] listeners = list.getListeners();
//    for (int i = 0; i < listeners.length; i++) {
//      list.remove(listeners[i]);
//    }
//  }

    /**
     * 
     */
void WorkbenchPartReference::doDisposePart() {
        if (part != null) {
            fireInternalPropertyChange(INTERNAL_PROPERTY_CLOSED);
            // Don't let exceptions in client code bring us down. Log them and continue.
            try {
                part.removePropertyListener(propertyChangeListener);
                if (part instanceof IWorkbenchPart3) {
                  ((IWorkbenchPart3)part).removePartPropertyListener(partPropertyChangeListener);
                }
                part.dispose();
            } catch (Exception e) {
                WorkbenchPlugin.log(e);
            }
            part = null;
        }
    }

void WorkbenchPartReference::setPinned(bool newPinned) {
        if (isDisposed()) {
            return;
        }

        if (newPinned == pinned) {
            return;
        }
        
        pinned = newPinned;
        
        setImageDescriptor(computeImageDescriptor());
        
        fireInternalPropertyChange(INTERNAL_PROPERTY_PINNED);
    }
    
bool WorkbenchPartReference::isPinned() {
        return pinned;
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchPartReference#getPartProperty(java.lang.String)
     */
const std::string& WorkbenchPartReference::getPartProperty(const std::string& key) {
    if (part != null) {
      if (part instanceof IWorkbenchPart3) {
        return ((IWorkbenchPart3) part).getPartProperty(key);
      }
    } else {
      return (String)propertyCache.get(key);
    }
    return null;
  }
    
    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchPartReference#addPartPropertyListener(org.eclipse.jface.util.IPropertyChangeListener)
     */
//public: void addPartPropertyListener(IPropertyChangeListener listener) {
//      if (isDisposed()) {
//        return;
//      }
//      partChangeListeners.add(listener);
//    }
    
    /* (non-Javadoc)
     * @see org.eclipse.ui.IWorkbenchPartReference#removePartPropertyListener(org.eclipse.jface.util.IPropertyChangeListener)
     */
//public: void removePartPropertyListener(IPropertyChangeListener listener) {
//      if (isDisposed()) {
//        return;
//      }
//      partChangeListeners.remove(listener);
//    }
    
//protected: void firePartPropertyChange(PropertyChangeEvent event) {
//    Object[] l = partChangeListeners.getListeners();
//    for (int i = 0; i < l.length; i++) {
//      ((IPropertyChangeListener) l[i]).propertyChange(event);
//    }
//  }
    
//protected: void createPartProperties(IWorkbenchPart3 workbenchPart) {
//    Iterator i = propertyCache.entrySet().iterator();
//    while (i.hasNext()) {
//      Map.Entry e = (Map.Entry) i.next();
//      workbenchPart.setPartProperty((String) e.getKey(), (String) e.getValue());
//    }
//  }

} // namespace cherry
