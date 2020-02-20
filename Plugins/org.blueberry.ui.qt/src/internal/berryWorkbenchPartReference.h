/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHPARTREFERENCE_H_
#define BERRYWORKBENCHPARTREFERENCE_H_

#include <berryMessage.h>

#include "berryISizeProvider.h"
#include "berryIWorkbenchPartReference.h"
#include "berryIWorkbenchPart.h"

#include <QIcon>

namespace berry {

class PartPane;

/**
 * \ingroup org_blueberry_ui_internal
 *
 */
class BERRY_UI_QT WorkbenchPartReference : virtual public IWorkbenchPartReference, public ISizeProvider {

public:

  berryObjectMacro(WorkbenchPartReference);


    // State constants //////////////////////////////

  /**
   * State constant indicating that the part is not created yet
   */
  static int STATE_LAZY; // = 0

  /**
   * State constant indicating that the part is in the process of being created
   */
  static int STATE_CREATION_IN_PROGRESS; // = 1

  /**
   * State constant indicating that the part has been created
   */
  static int STATE_CREATED; // = 2

  /**
   * State constant indicating that the reference has been disposed (the reference shouldn't be
   * used anymore)
   */
  static int STATE_DISPOSED; // = 3

  WorkbenchPartReference();
  ~WorkbenchPartReference() override;

  virtual bool IsDisposed() const;

  virtual void Init(const QString& id, const QString& tooltip,
                    const QIcon& desc, const QString& paneName, const QString& contentDescription);

  /**
   * @see IWorkbenchPart
   */
  void AddPropertyListener(IPropertyChangeListener* listener) override;

  /**
   * @see IWorkbenchPart
   */
  void RemovePropertyListener(IPropertyChangeListener* listener) override;

  QString GetId() const override;

  QString GetTitleToolTip() const override;

  /**
   * Returns the pane name for the part
   *
   * @return the pane name for the part
   */
  QString GetPartName() const override;

  /**
   * Returns the content description for this part.
   *
   * @return the pane name for the part
   */
  QString GetContentDescription() const override;

  bool IsDirty() const override;

  QIcon GetTitleImage() const override;

  virtual void FireVisibilityChange();

  virtual bool GetVisible();

  virtual void SetVisible(bool isVisible);

  IWorkbenchPart::Pointer GetPart(bool restore) override;

  /**
   * Returns the part pane for this part reference. Does not return null.
   *
   * @return
   */
  SmartPointer<PartPane> GetPane();

  void Dispose();

  virtual void SetPinned(bool newPinned);

  bool IsPinned() const override;

  /*
   * @see org.blueberry.ui.IWorkbenchPartReference#getPartProperty(java.lang.String)
   */
  QString GetPartProperty(const QString& key) const override;

  int ComputePreferredSize(bool width, int availableParallel,
                           int availablePerpendicular, int preferredResult) override;

  int GetSizeFlags(bool width) override;

protected:

  IWorkbenchPart::Pointer part;

  SmartPointer<PartPane> pane;

  QHash<QString, QString> propertyCache;

  virtual void CheckReference();

  virtual void SetPartName(const QString& newPartName);

  virtual void SetContentDescription(const QString& newContentDescription);

  virtual void SetImageDescriptor(const QIcon& descriptor);

  virtual void SetToolTip(const QString& newToolTip);

  virtual void PropertyChanged(const Object::Pointer& source, int propId);

  virtual void PropertyChanged(const PropertyChangeEvent::Pointer& event);

  /**
   * Refreshes all cached values with the values from the real part
   */
  virtual void RefreshFromPart();

  virtual QIcon ComputeImageDescriptor();

  // /* package */ virtual void fireZoomChange();

  QString GetRawToolTip() const;

  /**
   * Gets the part name directly from the associated workbench part,
   * or the empty string if none.
   *
   * @return
   */
  QString GetRawPartName() const;

  virtual QString ComputePartName() const;

  /**
   * Computes a new content description for the part. Subclasses may override to change the
   * default behavior
   *
   * @return the new content description for the part
   */
  virtual QString ComputeContentDescription() const;

  /**
   * Returns the content description as set directly by the part, or the empty string if none
   *
   * @return the unmodified content description from the part (or the empty string if none)
   */
  QString GetRawContentDescription() const;

  virtual void FirePropertyChange(int id);

  virtual IWorkbenchPart::Pointer CreatePart() = 0;

  virtual SmartPointer<PartPane> CreatePane() = 0;

  void DoDisposePart();

  virtual void FirePropertyChange(const PropertyChangeEvent::Pointer& event);

  virtual void CreatePartProperties(IWorkbenchPart::Pointer workbenchPart);

private:

  /**
   * Current state of the reference. Used to detect recursive creation errors, disposed
   * references, etc.
   */
  int state;

  QString id;

  bool pinned;

  QString tooltip;

  QIcon defaultImageDescriptor;

  /**
   * Stores the current image descriptor for the part.
   */
  QIcon imageDescriptor;

  /**
   * API listener list
   */
  IPropertyChangeListener::Events propChangeEvents;

  //private: ListenerList partChangeListeners = new ListenerList();

  QString partName;

  QString contentDescription;

  /**
   * Used to remember which events have been queued.
   */
  QSet<int> queuedEvents;

  bool queueEvents;

//static DisposeListener prematureDisposeListener = new DisposeListener() {
//        public void widgetDisposed(DisposeEvent e) {
//            WorkbenchPlugin.log(new RuntimeException("Widget disposed too early!")); //$NON-NLS-1$
//        }
//    };

  struct PropertyChangeListener : public IPropertyChangeListener
  {
    PropertyChangeListener(WorkbenchPartReference* ref);
    using IPropertyChangeListener::PropertyChange;
    void PropertyChange(const PropertyChangeEvent::Pointer& event) override;

  private:
    WorkbenchPartReference* partRef;
  };

  QScopedPointer<IPropertyChangeListener> propertyChangeListener;

  /**
   * Calling this with deferEvents(true) will queue all property change events until a subsequent
   * call to deferEvents(false). This should be used at the beginning of a batch of related changes
   * to prevent duplicate property change events from being sent.
   *
   * @param shouldQueue
   */
  void DeferEvents(bool shouldQueue);

  void ImmediateFirePropertyChange(int id);

  /**
   * Clears all of the listeners in a listener list. TODO Bug 117519 Remove
   * this method when fixed.
   *
   * @param list
   *            The list to be clear; must not be <code>null</code>.
   */
  //private: void clearListenerList(const ListenerList list);

};

} // namespace berry

#endif /*BERRYWORKBENCHPARTREFERENCE_H_*/
