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

#ifndef BERRYWORKBENCHSOURCEPROVIDER_H
#define BERRYWORKBENCHSOURCEPROVIDER_H

#include "berryAbstractSourceProvider.h"
#include "berryINullSelectionListener.h"
#include "berryIPartListener.h"
#include "berryIWindowListener.h"
#include "berryIPropertyChangeListener.h"
#include "berryIPerspectiveListener.h"
#include "berryIShellListener.h"

namespace berry {

struct IEditorPart;
struct IEditorInput;
struct IShowInSource;

class Display;
class ObjectBool;
class ObjectString;
class ShowInContext;
class WorkbenchWindow;

class WorkbenchSourceProvider : public QObject, public AbstractSourceProvider,
    private INullSelectionListener, private IPartListener, private IWindowListener,
    private IPropertyChangeListener, private IPerspectiveListener
{
  Q_OBJECT
  Q_INTERFACES(berry::ISourceProvider)

private:

  /**
   * The names of the sources supported by this source provider.
   */
  static const QList<QString> PROVIDED_SOURCE_NAMES;

  IWorkbench* workbench;
  WeakPointer<IWorkbenchWindow> lastWindow;
//  private IServiceLocator locator;

  // Selection SourceProvider

  SmartPointer<const ISelection> selection;

public:

  berryObjectMacro(berry::WorkbenchSourceProvider)

  WorkbenchSourceProvider();

  void Initialize(IServiceLocator* locator) override;

  ~WorkbenchSourceProvider() override;

  QList<QString> GetProvidedSourceNames() const override;

  ISourceProvider::StateMapType GetCurrentState() const override;

private:

  void SelectionChanged(const SmartPointer<IWorkbenchPart>& part,
                        const SmartPointer<const ISelection>& newSelection) override;

  int UpdateSelection(ISourceProvider::StateMapType& currentState) const;

  void UpdateWindows(IWorkbenchWindow* newWindow);

  // Active Part SourceProvider

  /**
   * The last active editor part seen as active by this provider. This value
   * may be <code>null</code> if there is no currently active editor.
   */
  const IEditorPart* lastActiveEditor;

  /**
   * The last active editor id seen as active by this provider. This value may
   * be <code>null</code> if there is no currently active editor.
   */
  SmartPointer<const ObjectString> lastActiveEditorId;

  /**
   * The last active part seen as active by this provider. This value may be
   * <code>null</code> if there is no currently active part.
   */
  const IWorkbenchPart* lastActivePart;

  /**
   * The last active part id seen as active by this provider. This value may
   * be <code>null</code> if there is no currently active part.
   */
  SmartPointer<const ObjectString> lastActivePartId;

  /**
   * The last active part site seen by this provider. This value may be
   * <code>null</code> if there is no currently active site.
   */
  const IWorkbenchPartSite* lastActivePartSite;

  SmartPointer<const Object> lastShowInInput;

  void PartActivated(const SmartPointer<IWorkbenchPartReference>& part) override;
  void PartBroughtToTop(const SmartPointer<IWorkbenchPartReference>& part) override;
  void PartClosed(const SmartPointer<IWorkbenchPartReference>& part) override;
  void PartDeactivated(const SmartPointer<IWorkbenchPartReference>& part) override;
  void PartOpened(const SmartPointer<IWorkbenchPartReference>& part) override;

  void WindowActivated(const SmartPointer<IWorkbenchWindow>& window) override;
  void WindowClosed(const SmartPointer<IWorkbenchWindow>& window) override;
  void WindowDeactivated(const SmartPointer<IWorkbenchWindow>& window) override;
  void WindowOpened(const SmartPointer<IWorkbenchWindow>& window) override;

  WeakPointer<const IEditorInput> lastEditorInput;

  void HandleCheck(const SmartPointer<const Shell>& s);

  void CheckActivePart();

  void CheckActivePart(bool updateShowInSelection);

  SmartPointer<IShowInSource> GetShowInSource(const SmartPointer<IWorkbenchPart>& sourcePart) const;

  SmartPointer<ShowInContext> GetContext(const SmartPointer<IWorkbenchPart>& sourcePart) const;

  IWorkbenchWindow* GetActiveWindow() const;

  void UpdateActivePart(ISourceProvider::StateMapType& currentState) const;

  void UpdateActivePart(ISourceProvider::StateMapType& currentState, bool updateShowInSelection) const;

  // Active Part SourceProvider

  /**
   * The display on which this provider is working.
   */
  Display* display;

  /**
   * The last shell seen as active by this provider. This value may be
   * <code>null</code> if the last call to
   * <code>Display.getActiveShell()</code> returned <code>null</code>.
   */
  WeakPointer<const Shell> lastActiveShell;

  /**
   * The last workbench window shell seen as active by this provider. This
   * value may be <code>null</code> if the last call to
   * <code>workbench.getActiveWorkbenchWindow()</code> returned
   * <code>null</code>.
   */
  WeakPointer<const Shell> lastActiveWorkbenchWindowShell;

  /**
   * The last workbench window seen as active by this provider. This value may
   * be null if the last call to
   * <code>workbench.getActiveWorkbenchWindow()</code> returned
   * <code>null</code>.
   */
  WeakPointer<const WorkbenchWindow> lastActiveWorkbenchWindow;

  /**
   * The result of the last visibility check on the toolbar of the last active
   * workbench window.
   */
  SmartPointer<const ObjectBool> lastToolbarVisibility;

  /**
   * The result of the last visibility check on the perspective bar of the
   * last active workbench window.
   */
  SmartPointer<const ObjectBool> lastPerspectiveBarVisibility;

  /**
   * The result of the last visibility check on the status line for the last
   * workbench window.
   */
  SmartPointer<const ObjectBool> lastStatusLineVisibility;

  /**
   * The last perspective id that was provided by this source.
   */
  SmartPointer<const ObjectString> lastPerspectiveId;

  /**
   * The listener to individual window properties.
   */
  void PropertyChange(const SmartPointer<PropertyChangeEvent>& event) override;
  void PropertyChange(const Object::Pointer& source, int propId) override;

  void PerspectiveActivated(const SmartPointer<IWorkbenchPage>& page,
                            const SmartPointer<IPerspectiveDescriptor>& perspective) override;

  using IPerspectiveListener::PerspectiveChanged;
  void PerspectiveChanged(const SmartPointer<IWorkbenchPage>& page,
                          const SmartPointer<IPerspectiveDescriptor>& perspective,
                          const QString& changeId) override;

  bool eventFilter(QObject *obj, QEvent *event) override;

  /**
   * The listener to shell activations on the display.
   * Notifies all listeners that the source has changed.
   */
  void HandleShellEvent();

  void CheckOtherSources(const SmartPointer<const Shell>& s);

  void HandleInputChanged(const SmartPointer<IEditorPart>& editor);

  void HookListener(WorkbenchWindow* lastActiveWorkbenchWindow,
                    WorkbenchWindow* newActiveWorkbenchWindow);

  void HookListener(IEditorPart* lastActiveEditor,
                    IEditorPart* newActiveEditor);

  void UpdateActiveShell(ISourceProvider::StateMapType& currentState) const;

  IPartListener::Events::Types GetPartEventTypes() const override;

  IPerspectiveListener::Events::Types GetPerspectiveEventTypes() const override;

};

}

#endif // BERRYWORKBENCHSOURCEPROVIDER_H
