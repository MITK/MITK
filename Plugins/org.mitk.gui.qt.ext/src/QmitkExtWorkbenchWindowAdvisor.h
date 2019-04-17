/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKEXTWORKBENCHWINDOWADVISOR_H_
#define QMITKEXTWORKBENCHWINDOWADVISOR_H_

#include <berryWorkbenchWindowAdvisor.h>

#include <berryIPartListener.h>
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryWorkbenchAdvisor.h>
#include <berryWorkbenchWindowAdvisor.h>

#include <org_mitk_gui_qt_ext_Export.h>

#include <QList>

class QAction;
class QMenu;

class MITK_QT_COMMON_EXT_EXPORT QmitkExtWorkbenchWindowAdvisor : public QObject, public berry::WorkbenchWindowAdvisor
{
  Q_OBJECT

public:

    QmitkExtWorkbenchWindowAdvisor(berry::WorkbenchAdvisor* wbAdvisor,
        berry::IWorkbenchWindowConfigurer::Pointer configurer);

    ~QmitkExtWorkbenchWindowAdvisor() override;

    berry::SmartPointer<berry::ActionBarAdvisor> CreateActionBarAdvisor(
        berry::SmartPointer<berry::IActionBarConfigurer> configurer) override;

    QWidget* CreateEmptyWindowContents(QWidget* parent) override;

    void PostWindowCreate() override;

    void PreWindowOpen() override;

    void PostWindowOpen() override;

    void PostWindowClose() override;

    void ShowViewToolbar(bool show);

    void ShowPerspectiveToolbar(bool show);

    void ShowVersionInfo(bool show);

    void ShowMitkVersionInfo(bool show);

    void ShowViewMenuItem(bool show);

    void ShowNewWindowMenuItem(bool show);

    void ShowClosePerspectiveMenuItem(bool show);

    bool GetShowClosePerspectiveMenuItem();

    void ShowMemoryIndicator(bool show);

    bool GetShowMemoryIndicator();

    //TODO should be removed when product support is here
    void SetProductName(const QString& product);
    void SetWindowIcon(const QString& wndIcon);

    void SetPerspectiveExcludeList(const QList<QString> &v);
    QList<QString> GetPerspectiveExcludeList();

    void SetViewExcludeList(const QList<QString> &v);
    QList<QString> GetViewExcludeList();

protected slots:

    virtual void onIntro();
    virtual void onHelp();
    virtual void onHelpOpenHelpPerspective();
    virtual void onAbout();

private:

  /**
   * Hooks the listeners needed on the window
   *
   * @param configurer
   */
  void HookTitleUpdateListeners(berry::IWorkbenchWindowConfigurer::Pointer configurer);

  QString ComputeTitle();

  void RecomputeTitle();

  QString GetQSettingsFile() const;

  /**
   * Updates the window title. Format will be: [pageInput -]
   * [currentPerspective -] [editorInput -] [workspaceLocation -] productName
   * @param editorHidden TODO
   */
  void UpdateTitle(bool editorHidden);

  void PropertyChange(const berry::Object::Pointer& /*source*/, int propId);

  static QString QT_SETTINGS_FILENAME;

  QScopedPointer<berry::IPartListener> titlePartListener;
  QScopedPointer<berry::IPerspectiveListener> titlePerspectiveListener;
  QScopedPointer<berry::IPerspectiveListener> menuPerspectiveListener;
  QScopedPointer<berry::IPartListener> imageNavigatorPartListener;
  QScopedPointer<berry::IPartListener> viewNavigatorPartListener;
  QScopedPointer<berry::IPropertyChangeListener> editorPropertyListener;
  friend struct berry::PropertyChangeIntAdapter<QmitkExtWorkbenchWindowAdvisor>;
  friend class PartListenerForTitle;
  friend class PerspectiveListenerForTitle;
  friend class PerspectiveListenerForMenu;
  friend class PartListenerForImageNavigator;
  friend class PartListenerForViewNavigator;

  berry::IEditorPart::WeakPtr lastActiveEditor;
  berry::IPerspectiveDescriptor::WeakPtr lastPerspective;
  berry::IWorkbenchPage::WeakPtr lastActivePage;
  QString lastEditorTitle;
  berry::IAdaptable* lastInput;

  berry::WorkbenchAdvisor* wbAdvisor;
  bool showViewToolbar;
  bool showPerspectiveToolbar;
  bool showVersionInfo;
  bool showMitkVersionInfo;
  bool showViewMenuItem;
  bool showNewWindowMenuItem;
  bool showClosePerspectiveMenuItem;
  bool viewNavigatorFound;
  bool showMemoryIndicator;
  QString productName;
  QString windowIcon;

  // enables DnD on the editor area
  QScopedPointer<berry::IDropTargetListener> dropTargetListener;

  // stringlist for excluding perspectives from the perspective menu entry (e.g. Welcome Perspective)
  QList<QString> perspectiveExcludeList;

  // stringlist for excluding views from the menu entry
  QList<QString> viewExcludeList;

  // maps perspective ids to QAction objects
  QHash<QString, QAction*> mapPerspIdToAction;

  // actions which will be enabled/disabled depending on the application state
  QList<QAction*> viewActions;
  QAction* fileSaveProjectAction;
  QAction* closeProjectAction;
  QAction* undoAction;
  QAction* redoAction;
  QAction* imageNavigatorAction;
  QAction* viewNavigatorAction;
  QAction* resetPerspAction;
  QAction* closePerspAction;
  QAction* openDicomEditorAction;
  QAction* openStdMultiWidgetEditorAction;
  QAction* openMxNMultiWidgetEditorAction;
};

#endif /*QMITKEXTWORKBENCHWINDOWADVISOR_H_*/
