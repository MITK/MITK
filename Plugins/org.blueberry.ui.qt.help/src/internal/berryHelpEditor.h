/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHELPEDITOR_H_
#define BERRYHELPEDITOR_H_

#include <berryIReusableEditor.h>
#include <berryIPartListener.h>
#include <berryQtEditorPart.h>

class QToolBar;

namespace berry {

class HelpWebView;

class HelpEditor : public QtEditorPart, public IReusableEditor, public IPartListener
{
  Q_OBJECT

public:
  berryObjectMacro(HelpEditor);

  static const QString EDITOR_ID;

  HelpEditor();
  ~HelpEditor() override;

  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input) override;

  void SetFocus() override;

  void DoSave() override {}
  void DoSaveAs() override {}
  bool IsDirty() const override { return false; }
  bool IsSaveAsAllowed() const override { return false; }

  IPartListener::Events::Types GetPartEventTypes() const override;
  void PartDeactivated(const IWorkbenchPartReference::Pointer& /*partRef*/) override;

protected:

  void CreateQtPartControl(QWidget* parent) override;

  void DoSetInput(IEditorInput::Pointer input);
  void SetInputWithNotify(IEditorInput::Pointer input) override;
  void SetInput(IEditorInput::Pointer input) override;

private Q_SLOTS:

  void HomePageChanged(const QString& page);

  void InitializeTitle();

private:

  void enableShortcuts();
  void disableShortcuts();

private:

  Q_DISABLE_COPY(HelpEditor)

  QToolBar* m_ToolBar;
  HelpWebView* m_WebView;

  QAction* m_BackAction;
  QAction* m_ForwardAction;
  QAction* m_ZoomIn;
  QAction* m_ZoomOut;
};

} // end namespace berry

#endif /*BERRYHELPEDITOR_H_*/
