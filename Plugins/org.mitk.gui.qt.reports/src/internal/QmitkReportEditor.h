/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkReportEditor_h
#define QmitkReportEditor_h

#include <berryQtEditorPart.h>

class QmitkHtmlWidget;
class QUrl;

/**
 * \brief Shows a report of the QmitkReportStore, which the user can save as
 *        HTML or open in the web browser.
 *
 * The report lives as long as its editor: closing the editor removes it, after
 * asking the user whether to save it if it is unsaved. The question follows the
 * close, because a part cannot veto the closing of its tab.
 */
class QmitkReportEditor : public berry::QtEditorPart
{
  Q_OBJECT

public:
  berryObjectMacro(QmitkReportEditor);

  static const QString EDITOR_ID;

  QmitkReportEditor();
  ~QmitkReportEditor() override;

  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input) override;
  void SetFocus() override;

  void DoSave() override {}
  void DoSaveAs() override {}
  bool IsDirty() const override { return false; }
  bool IsSaveAsAllowed() const override { return false; }

protected:
  void CreateQtPartControl(QWidget* parent) override;

private:
  void ShowReport(bool keepScrollPosition);
  void OnReportChanged(int id);
  void OnLinkClicked(const QUrl& url);
  void SaveAsHtml();
  void OpenInBrowser();

  int m_ReportId = 0;
  QmitkHtmlWidget* m_View = nullptr;
};

#endif
