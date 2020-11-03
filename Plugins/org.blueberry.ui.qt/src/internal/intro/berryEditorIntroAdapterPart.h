/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYEDITORINTROADAPTERPART_H_
#define BERRYEDITORINTROADAPTERPART_H_

#include <berryEditorPart.h>

#include <intro/berryIIntroPart.h>
#include <intro/berryIIntroSite.h>

namespace berry
{

/**
 * Simple editor that will wrap an <code>IIntroPart</code>.
 */
class EditorIntroAdapterPart: public EditorPart
{
  Q_OBJECT

private:

  IIntroPart::Pointer introPart;

  IIntroSite::Pointer introSite;

  QScopedPointer<IPropertyChangeListener> propChangeListener;
  friend struct PropertyChangeIntAdapter<EditorIntroAdapterPart> ;

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

  EditorIntroAdapterPart();

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
  ~EditorIntroAdapterPart() override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.IWorkbenchPart#getTitleImage()
   */
  QIcon GetTitleImage() const override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.part.WorkbenchPart#GetPartName()
   */
  QString GetPartName() const override;

  void Init(IEditorSite::Pointer site, IEditorInput::Pointer input) override;

  void DoSave(/*IProgressMonitor monitor*/) override;

  void DoSaveAs() override;

  bool IsDirty() const override;

  bool IsSaveAsAllowed() const override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.IWorkbenchPart#setFocus()
   */
  void SetFocus() override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.IViewPart#saveState(org.eclipse.ui.IMemento)
   */
  void SaveState(IMemento::Pointer memento);

};

}

#endif /* BERRYEDITORINTROADAPTERPART_H_ */
