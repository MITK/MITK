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
 * The report lives as long as its editor: closing the editor removes it. An
 * unsaved report counts as dirty, and the editor asks the user before it is
 * closed whether to save the report, discard it or keep the editor open. The
 * store asks after the close if the editor was closed in a way that did not
 * ask.
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

  /** \brief Runs "Save as HTML...". */
  void DoSave() override;
  void DoSaveAs() override {}

  /** \brief Whether the report is neither saved nor given up. */
  bool IsDirty() const override;
  bool IsSaveAsAllowed() const override { return false; }

  /** \brief Asks whether to save the report as HTML, discard it or keep the editor open. */
  CloseAnswer PromptToSaveOnClose() override;

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
