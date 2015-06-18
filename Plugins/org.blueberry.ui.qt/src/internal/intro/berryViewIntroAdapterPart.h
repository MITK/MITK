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

#ifndef BERRYVIEWINTROADAPTERPART_H_
#define BERRYVIEWINTROADAPTERPART_H_

#include <berryViewPart.h>

#include <intro/berryIIntroPart.h>
#include <intro/berryIIntroSite.h>

namespace berry
{

/**
 * Simple view that will wrap an <code>IIntroPart</code>.
 *
 * @since 3.0
 */
class ViewIntroAdapterPart: public ViewPart
{

private:

  IIntroPart::Pointer introPart;

  IIntroSite::Pointer introSite;

  QScopedPointer<IPropertyChangeListener> propChangeListener;
  friend struct PropertyChangeIntAdapter<ViewIntroAdapterPart> ;

  void PropertyChange(const Object::Pointer& source, int propId);

  //     bool handleZoomEvents = true;

  //    /**
  //     * Adds a listener that toggles standby state if the view pane is zoomed.
  //     */
  //    void AddPaneListener() {
  //        IWorkbenchPartSite site = getSite();
  //        if (site instanceof PartSite) {
  //            final WorkbenchPartReference ref = ((WorkbenchPartReference)((PartSite) site).getPartReference());
  //            ref.addInternalPropertyListener(
  //                    new IPropertyListener() {
  //                        public void propertyChanged(Object source, int propId) {
  //                            if (handleZoomEvents) {
  //                                if (propId == WorkbenchPartReference.INTERNAL_PROPERTY_ZOOMED) {
  //                                    setStandby(!ref.getPane().isZoomed());
  //                                }
  //                            }
  //                        }
  //                    });
  //        }
  //    }

  //  /**
  //   * Sets whether the CoolBar/PerspectiveBar should be visible.
  //   *
  //   * @param visible whether the CoolBar/PerspectiveBar should be visible
  //   */
  //   void SetBarVisibility(bool visible) {
  //    WorkbenchWindow window = (WorkbenchWindow) getSite()
  //        .getWorkbenchWindow();
  //
  //    final boolean layout = (visible != window.getCoolBarVisible())
  //        || (visible != window.getPerspectiveBarVisible()); // don't layout unless things have actually changed
  //    if (visible) {
  //      window.setCoolBarVisible(true);
  //      window.setPerspectiveBarVisible(true);
  //    } else {
  //      window.setCoolBarVisible(false);
  //      window.setPerspectiveBarVisible(false);
  //    }
  //
  //    if (layout) {
  //      window.getShell().layout();
  //    }
  //  }

public:

  ViewIntroAdapterPart();

  /**
   * Forces the standby state of the intro part.
   *
   * @param standby update the standby state
   */
  void SetStandby(bool standby);

  //    /**
  //     * Toggles handling of zoom events.
  //     *
  //     * @param handle whether to handle zoom events
  //     */
  //    void SetHandleZoomEvents(boolean handle) {
  //        handleZoomEvents = handle;
  //    }

  /* (non-Javadoc)
   * @see org.eclipse.ui.IWorkbenchPart#createPartControl(org.eclipse.swt.widgets.Composite)
   */
  void CreatePartControl(QWidget* parent) override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.IWorkbenchPart#dispose()
   */
  ~ViewIntroAdapterPart();

  /* (non-Javadoc)
   * @see org.eclipse.ui.IWorkbenchPart#getTitleImage()
   */
  QIcon GetTitleImage() const override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.part.WorkbenchPart#GetPartName()
   */
  QString GetPartName() const override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.IViewPart#init(org.eclipse.ui.IViewSite, org.eclipse.ui.IMemento)
   */
  void Init(IViewSite::Pointer site, IMemento::Pointer memento =
      IMemento::Pointer(nullptr)) throw (PartInitException) override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.IWorkbenchPart#setFocus()
   */
  void SetFocus() override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.IViewPart#saveState(org.eclipse.ui.IMemento)
   */
  void SaveState(IMemento::Pointer memento) override;

};

}

#endif /* BERRYVIEWINTROADAPTERPART_H_ */
