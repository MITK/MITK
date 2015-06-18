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

#ifndef BERRYHELPEDITOR_H_
#define BERRYHELPEDITOR_H_

#include <berryIReusableEditor.h>
#include <berryIPartListener.h>
#include <berryIPerspectiveListener.h>
#include <berryQtEditorPart.h>

class QToolBar;
class QWebPage;

namespace berry {

class HelpWebView;
class HelpEditorFindWidget;

class HelpEditor : public QtEditorPart, public IReusableEditor, public IPartListener, public IPerspectiveListener
{
  Q_OBJECT

public:
  berryObjectMacro(HelpEditor);

  static const QString EDITOR_ID;

  HelpEditor();
  ~HelpEditor();

  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input) override;

  void SetFocus() override;

  void DoSave() override {}
  void DoSaveAs() override {}
  bool IsDirty() const override { return false; }
  bool IsSaveAsAllowed() const override { return false; }

  QWebPage* GetQWebPage() const;

  IPartListener::Events::Types GetPartEventTypes() const override;
  void PartDeactivated(const IWorkbenchPartReference::Pointer& /*partRef*/) override;

  IPerspectiveListener::Events::Types GetPerspectiveEventTypes() const override;
  void PerspectiveActivated(const SmartPointer<IWorkbenchPage>& page,
                            const IPerspectiveDescriptor::Pointer& perspective) override;
  void PerspectiveDeactivated(const SmartPointer<IWorkbenchPage>& page,
                              const IPerspectiveDescriptor::Pointer& perspective) override;

protected:

  void CreateQtPartControl(QWidget* parent) override;

  void DoSetInput(IEditorInput::Pointer input);
  void SetInputWithNotify(IEditorInput::Pointer input) override;
  void SetInput(IEditorInput::Pointer input) override;

private Q_SLOTS:

  void HomePageChanged(const QString& page);
  void OpenHelpPerspective();
  void CloseHelpPerspective();

  void InitializeTitle();
  void ShowTextSearch();

  void findNext();
  void findPrevious();
  void find(const QString& ttf, bool forward);

private:

  bool findInWebPage(const QString& ttf, bool forward);

  void enableShortcuts();
  void disableShortcuts();

private:

  Q_DISABLE_COPY(HelpEditor)

  QToolBar* m_ToolBar;
  HelpWebView* m_WebView;
  HelpEditorFindWidget* m_FindWidget;

  QAction* m_BackAction;
  QAction* m_ForwardAction;
  QAction* m_FindAction;
  QAction* m_ZoomIn;
  QAction* m_ZoomOut;
  QAction* m_OpenHelpMode;
  QAction* m_CloseHelpMode;
  QAction* m_HomeAction;
  QAction* m_PrintAction;
};

} // end namespace berry

#endif /*BERRYHELPEDITOR_H_*/
