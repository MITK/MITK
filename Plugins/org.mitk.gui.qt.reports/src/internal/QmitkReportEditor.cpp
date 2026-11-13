/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkReportEditor.h"
#include "QmitkReportEditorInput.h"
#include "QmitkReportsPluginActivator.h"
#include "QmitkReportStore.h"

#include <QmitkHtmlWidget.h>
#include <QmitkIconTheme.h>

#include <mitkException.h>

#include <berryUIException.h>

#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
#include <QScrollBar>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

const QString QmitkReportEditor::EDITOR_ID = "org.mitk.editors.report";

namespace
{
  const QString Scheme = QStringLiteral("mitk-report");
}

QmitkReportEditor::QmitkReportEditor() = default;

QmitkReportEditor::~QmitkReportEditor()
{
  if (auto* store = QmitkReportsPluginActivator::GetStore(); store != nullptr)
    store->CloseReport(m_ReportId);
}

void QmitkReportEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
  const auto reportInput = input.Cast<QmitkReportEditorInput>();

  if (reportInput.IsNull())
    throw berry::PartInitException("Invalid input: must be a QmitkReportEditorInput");

  m_ReportId = reportInput->GetReportId();

  this->SetSite(site);
  this->SetInput(input);
  this->SetPartName(input->GetName());

  if (auto* store = QmitkReportsPluginActivator::GetStore(); store != nullptr)
    connect(store, &QmitkReportStore::Changed, this, &QmitkReportEditor::OnReportChanged);
}

void QmitkReportEditor::SetFocus()
{
  if (m_View != nullptr)
    m_View->setFocus();
}

void QmitkReportEditor::CreateQtPartControl(QWidget* parent)
{
  auto* layout = new QVBoxLayout(parent);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  auto* toolBar = new QToolBar(parent);
  toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  layout->addWidget(toolBar);

  auto* save = toolBar->addAction(
    QmitkIconTheme::GetIcon(QStringLiteral(":/org.mitk.gui.qt.reports/SaveAsHtml.svg")),
    "Save as HTML...");
  save->setToolTip("Save the report as an HTML file with its images included");
  connect(save, &QAction::triggered, this, &QmitkReportEditor::SaveAsHtml);

  auto* browser = toolBar->addAction(
    QmitkIconTheme::GetIcon(QStringLiteral(":/org.mitk.gui.qt.reports/OpenInBrowser.svg")),
    "Open in Browser");
  browser->setToolTip("Open the report in the web browser, for example to print it. The page is a temporary copy, "
                      "which is removed when the report is closed.");
  connect(browser, &QAction::triggered, this, &QmitkReportEditor::OpenInBrowser);

  m_View = new QmitkHtmlWidget(parent);
  m_View->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(m_View);

  m_View->SetResourceHandler([this](const QUrl& url) {
    const auto* store = QmitkReportsPluginActivator::GetStore();
    const auto* report = store != nullptr ? store->Get(m_ReportId) : nullptr;

    return url.scheme() == Scheme && report != nullptr ? report->Images.value(url.fileName()) : QByteArray();
  });

  connect(m_View, &QmitkHtmlWidget::linkClicked, this, &QmitkReportEditor::OnLinkClicked);
  connect(QmitkIconTheme::GetInstance(), &QmitkIconTheme::Changed, this, [this]() { this->ShowReport(true); });

  this->ShowReport(false);
}

void QmitkReportEditor::ShowReport(bool keepScrollPosition)
{
  const auto* store = QmitkReportsPluginActivator::GetStore();
  const auto* report = store != nullptr ? store->Get(m_ReportId) : nullptr;

  if (report == nullptr || m_View == nullptr)
    return;

  const auto scrollPosition = m_View->verticalScrollBar()->value();

  m_View->SetHtml(store->GetDocument(m_ReportId, false), QUrl(Scheme + QStringLiteral(":/%1/").arg(m_ReportId)));

  if (keepScrollPosition)
    m_View->verticalScrollBar()->setValue(scrollPosition);

  this->SetPartName(report->Title);
}

void QmitkReportEditor::OnReportChanged(int id)
{
  if (id == m_ReportId)
    this->ShowReport(true);
}

void QmitkReportEditor::OnLinkClicked(const QUrl& url)
{
  if (url.scheme() == Scheme)
  {
    if (url.hasFragment())
      m_View->ScrollToAnchor(url.fragment());
  }
  else if (url.scheme() == "http" || url.scheme() == "https" || url.scheme() == "mailto")
  {
    QDesktopServices::openUrl(url);
  }
}

void QmitkReportEditor::SaveAsHtml()
{
  if (auto* store = QmitkReportsPluginActivator::GetStore(); store != nullptr)
    store->SaveReport(m_ReportId, m_View->window());
}

void QmitkReportEditor::OpenInBrowser()
{
  auto* store = QmitkReportsPluginActivator::GetStore();

  if (store == nullptr)
    return;

  try
  {
    const auto path = store->WriteTemporaryFile(m_ReportId);

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
      QMessageBox::warning(m_View->window(), "Open in Browser", "No program opens " + QDir::toNativeSeparators(path) + ".");
  }
  catch (const mitk::Exception& e)
  {
    QMessageBox::warning(m_View->window(), "Open in Browser", QString::fromStdString(e.GetDescription()));
  }
}
