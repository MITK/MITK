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

#ifndef BERRYPRESENTABLEPART_H_
#define BERRYPRESENTABLEPART_H_

#include "presentations/berryIPresentablePart.h"

#include "berryWorkbenchPartReference.h"

namespace berry
{

class PartPane;

/**
 * This is a lightweight adapter that allows PartPanes to be used by a StackPresentation. All methods
 * either redirect directly to PartPane or do trivial type conversions. All listeners registered by
 * the presentation are kept here rather than registering them directly on the PartPane. This allows
 * us to remove all listeners registered by a presentation that has been disposed, offering some
 * protection against memory leaks.
 */
class PresentablePart: public IPresentablePart
{

public:

  berryObjectMacro(PresentablePart)

private:

  SmartPointer<PartPane> part;

  /**
   * Local listener list -- we use this rather than registering listeners directly on the part
   * in order to protect against memory leaks in badly behaved presentations.
   */
  //List listeners = new ArrayList();

  // Lazily initialized. Use getPropertyListenerProxy() to access.
  QScopedPointer<IPropertyChangeListener> lazyPropertyListenerProxy;

  //ListenerList partPropertyChangeListeners = new ListenerList();
  IPropertyChangeListener::Events partPropertyChangeEvents;

  //IPropertyChangeListener::Pointer lazyPartPropertyChangeListener;

  // Lazily initialized. Use getMenu() to access
  //IPartMenu viewMenu;

  // True iff the "set" methods on this object are talking to the real part (disabled
  // if the part is currently being managed by another presentation stack)
  bool enableInputs;

  // True iff the "get" methods are returning up-to-date info from the real part (disabled
  // for efficiency if the presentation is invisible)
  bool enableOutputs;
  QRect savedBounds;
  bool isVisible;

  // Saved state (only used when the part is inactive)
  QString name;
  QString titleStatus;
  bool isDirty;
  bool isBusy;
  bool hasViewMenu;

  struct PropertyListenerProxy: public IPropertyChangeListener
  {

    PropertyListenerProxy(PresentablePart* part);

    using IPropertyChangeListener::PropertyChange;
    void PropertyChange(const PropertyChangeEvent::Pointer& e);

  private:
    PresentablePart* part;

  };

  friend struct PropertyListenerProxy;

  IPropertyChangeListener* GetPropertyListenerProxy();

  WorkbenchPartReference::Pointer GetPartReference() const;

protected:

  void FirePropertyChange(int propId);
  void FirePropertyChange(const PropertyChangeEvent::Pointer& event);

public:

  /**
   * Constructor
   *
   * @param part
   */
  PresentablePart(SmartPointer<PartPane> part, QWidget* parent);

  SmartPointer<PartPane> GetPane() const;

  /**
   * Detach this PresentablePart from the real part. No further methods should
   * be invoked on this object.
   */
  ~PresentablePart();

  //    void firePropertyChange(int propertyId) {
  //        for (int i = 0; i < listeners.size(); i++) {
  //            ((IPropertyListener) listeners.get(i)).propertyChanged(this, propertyId);
  //        }
  //    }

  void AddPropertyListener(IPropertyChangeListener* listener);

  void RemovePropertyListener(IPropertyChangeListener* listener);

  //    void addPartPropertyListener(IPropertyChangeListener listener) {
  //      partPropertyChangeListeners.add(listener);
  //    }
  //
  //    void removePartPropertyListener(IPropertyChangeListener listener) {
  //      partPropertyChangeListeners.remove(listener);
  //    }

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentablePart#setBounds(org.blueberry.swt.graphics.QRect)
   */
  void SetBounds(const QRect& bounds);

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentablePart#setVisible(boolean)
   */
  void SetVisible(bool isVisible);

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentablePart#setFocus()
   */
  void SetFocus();

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentablePart#getName()
   */
  QString GetName() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentablePart#getTitle()
   */
  QString GetTitle() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentablePart#getTitleStatus()
   */
  QString GetTitleStatus() const;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.presentations.IPresentablePart#getTitleImage()
   */
  QIcon GetTitleImage();

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.presentations.IPresentablePart#getTitleToolTip()
   */
  QString GetTitleToolTip() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentablePart#isDirty()
   */
  bool IsDirty() const;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.presentations.IPresentablePart#isBusy()
   */
  bool IsBusy() const;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.presentations.IPresentablePart#getToolBar()
   */
  QToolBar* GetToolBar();

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.presentations.IPresentablePart#getMenu()
   */
  //    IPartMenu getMenu() {
  //        boolean hasMenu;
  //
  //        if (enableOutputs) {
  //            hasMenu = part.hasViewMenu();
  //        } else {
  //            hasMenu = this.hasViewMenu;
  //        }
  //
  //        if (!hasMenu) {
  //            return null;
  //        }
  //
  //        if (viewMenu == null) {
  //            viewMenu = new IPartMenu() {
  //                public void showMenu(Point location) {
  //                    part.showViewMenu(location);
  //                }
  //            };
  //        }
  //
  //        return viewMenu;
  //    }

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentablePart#isCloseable()
   */
  bool IsCloseable() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentablePart#getControl()
   */
  QWidget* GetControl();

  void EnableOutputs(bool isActive);

  void EnableInputs(bool isActive);

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IPresentablePart#getPartProperty(java.lang.String)
   */
  QString GetPartProperty(const QString& key) const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.ISizeProvider#computePreferredSize(boolean, int, int, int)
   */
  int ComputePreferredSize(bool width, int availableParallel,
      int availablePerpendicular, int preferredResult);

  /* (non-Javadoc)
   * @see org.blueberry.ui.ISizeProvider#getSizeFlags(boolean)
   */
  int GetSizeFlags(bool width);

};

}

#endif /* BERRYPRESENTABLEPART_H_ */
