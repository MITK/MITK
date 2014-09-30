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

  static const std::string EDITOR_ID;

  HelpEditor();
  ~HelpEditor();

  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input);

  void SetFocus();

  void DoSave() {}
  void DoSaveAs() {}
  bool IsDirty() const { return false; }
  bool IsSaveAsAllowed() const { return false; }

  QWebPage* GetQWebPage() const;

  IPartListener::Events::Types GetPartEventTypes() const;
  void PartDeactivated(IWorkbenchPartReference::Pointer /*partRef*/);

  IPerspectiveListener::Events::Types GetPerspectiveEventTypes() const;
  void PerspectiveActivated(SmartPointer<IWorkbenchPage> page, IPerspectiveDescriptor::Pointer perspective);
  void PerspectiveDeactivated(SmartPointer<IWorkbenchPage> page, IPerspectiveDescriptor::Pointer perspective);

protected:

  void CreateQtPartControl(QWidget* parent);

  void DoSetInput(IEditorInput::Pointer input);
  void SetInputWithNotify(IEditorInput::Pointer input);
  void SetInput(IEditorInput::Pointer input);

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
