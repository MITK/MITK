/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkPipInstallDialog.h>
#include <ui_QmitkPipInstallDialog.h>

#include <mitkPipInstaller.h>

#include <QCloseEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QScrollBar>
#include <QTreeWidgetItem>

QmitkPipInstallDialog::QmitkPipInstallDialog(const mitk::PipInstallSpec& spec, QWidget* parent)
  : QDialog(parent),
    m_Ui(std::make_unique<Ui::QmitkPipInstallDialog>()),
    m_Installer(new mitk::PipInstaller(this)),
    m_Spec(spec)
{
  m_Ui->setupUi(this);

  this->setWindowTitle("Install " + spec.name);
  m_Ui->descriptionLabel->setText(
    QString("<h3>Install %1?</h3>"
            "<p>The required Python packages will be downloaded and installed. "
            "This may take a while depending on your internet connection.</p>")
      .arg(spec.name.toHtmlEscaped()));

  // Initially hide advanced settings and details.
  m_Ui->advancedGroupBox->hide();
  m_Ui->detailsTextEdit->hide();

  // Package tree starts hidden until resolve finishes.
  m_Ui->packageTree->hide();
  m_Ui->progressBar->hide();

  BuildAdvancedSettings();

  // Button connections.
  disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkPipInstallDialog::OnInstallClicked);
  connect(m_Ui->advancedButton, &QPushButton::clicked, this, &QmitkPipInstallDialog::OnShowAdvancedSettingsClicked);
  connect(m_Ui->detailsButton, &QPushButton::clicked, this, &QmitkPipInstallDialog::OnShowDetailsClicked);

  // Installer connections.
  connect(m_Installer, &mitk::PipInstaller::pipUpgradeStarted, this, &QmitkPipInstallDialog::OnPipUpgradeStarted);
  connect(m_Installer, &mitk::PipInstaller::pipUpgradeFinished, this, &QmitkPipInstallDialog::OnPipUpgradeFinished);
  connect(m_Installer, &mitk::PipInstaller::resolveStarted, this, &QmitkPipInstallDialog::OnResolveStarted);
  connect(m_Installer, &mitk::PipInstaller::resolveFinished, this, &QmitkPipInstallDialog::OnResolveFinished);
  connect(m_Installer, &mitk::PipInstaller::packageStatusChanged, this, &QmitkPipInstallDialog::OnPackageStatusChanged);
  connect(m_Installer, &mitk::PipInstaller::installFinished, this, &QmitkPipInstallDialog::OnInstallFinished);
  connect(m_Installer, &mitk::PipInstaller::outputReceived, this, &QmitkPipInstallDialog::OnOutputReceived);
  connect(m_Installer, &mitk::PipInstaller::progressChanged, this, &QmitkPipInstallDialog::OnProgressChanged);
  connect(m_Installer, &mitk::PipInstaller::errorOccurred, this, &QmitkPipInstallDialog::OnErrorOccurred);
}

QmitkPipInstallDialog::~QmitkPipInstallDialog()
{
}

void QmitkPipInstallDialog::closeEvent(QCloseEvent* event)
{
  if (m_IsInstalling)
  {
    auto answer = QMessageBox::question(
      this,
      "Cancel installation",
      "<p><b>WARNING:</b> The installation is still in progress. "
      "Closing now may leave packages in an incomplete state.</p>"
      "<p>Close anyway?</p>");

    if (answer == QMessageBox::No)
    {
      event->ignore();
      return;
    }

    m_Installer->Cancel();
  }
}

void QmitkPipInstallDialog::reject()
{
  if (m_IsInstalling)
  {
    // Block Esc key during installation. The user must use the close button
    // to get the confirmation dialog.
    return;
  }

  QDialog::reject();
}

// --- Slots: user actions ---

void QmitkPipInstallDialog::OnInstallClicked()
{
  // Read back any edits from the advanced settings form.
  auto* formLayout = m_Ui->advancedGroupBox->findChild<QFormLayout*>("advancedFormLayout");

  if (formLayout != nullptr)
  {
    int fieldIndex = 0;

    for (int g = 0; g < m_Spec.groups.size(); ++g)
    {
      auto& group = m_Spec.groups[g];

      for (int r = 0; r < group.requirements.size(); ++r)
      {
        auto* lineEdit = qobject_cast<QLineEdit*>(formLayout->itemAt(fieldIndex, QFormLayout::FieldRole)->widget());

        if (lineEdit != nullptr && !lineEdit->text().isEmpty())
          group.requirements[r] = lineEdit->text();

        ++fieldIndex;
      }

      if (!group.indexUrl.isEmpty())
      {
        auto* lineEdit = qobject_cast<QLineEdit*>(formLayout->itemAt(fieldIndex, QFormLayout::FieldRole)->widget());

        if (lineEdit != nullptr && !lineEdit->text().isEmpty())
          group.indexUrl = lineEdit->text();

        ++fieldIndex;
      }
    }
  }

  m_Installer->SetInstallSpec(m_Spec);
  SetUiInstalling();
  m_Installer->StartResolveAndInstall();
}

// --- Slots: installer events ---

void QmitkPipInstallDialog::OnPipUpgradeStarted()
{
  m_Ui->statusLabel->setText("Upgrading pip...");
}

void QmitkPipInstallDialog::OnPipUpgradeFinished(bool success)
{
  if (!success)
    m_Ui->statusLabel->setText("pip upgrade failed (non-fatal). Continuing...");
}

void QmitkPipInstallDialog::OnResolveStarted()
{
  m_Ui->statusLabel->setText("Resolving dependencies...");
  m_Ui->progressBar->show();
  m_Ui->progressBar->setRange(0, 0); // Indeterminate.
}

void QmitkPipInstallDialog::OnResolveFinished(bool success, const QList<mitk::PipPackageInfo>& packages)
{
  if (!success)
  {
    SetUiFinished(false);
    m_Ui->statusLabel->setText("Dependency resolution failed.");
    return;
  }

  // resolveFinished is emitted once per group with the accumulated list.
  // Only add packages that are new since the last call.
  auto existingCount = m_Ui->packageTree->topLevelItemCount();

  if (packages.size() > existingCount)
  {
    m_Ui->packageTree->show();

    for (int i = existingCount; i < packages.size(); ++i)
    {
      const auto& pkg = packages[i];
      auto* item = new QTreeWidgetItem(m_Ui->packageTree);
      item->setText(0, pkg.name);
      item->setText(1, pkg.version);
      item->setText(2, "Pending");
      item->setForeground(2, QColor(128, 128, 128));
    }

    m_Ui->packageTree->resizeColumnToContents(0);
    m_Ui->packageTree->resizeColumnToContents(1);
  }

  m_Ui->statusLabel->setText("Installing packages...");
}

void QmitkPipInstallDialog::OnPackageStatusChanged(int index, const QString& /*name*/, mitk::PackageStatus status)
{
  SetPackageStatus(index, status);
}

void QmitkPipInstallDialog::OnInstallFinished(bool success)
{
  SetUiFinished(success);

  if (success)
  {
    m_Ui->statusLabel->setText("All packages installed successfully.");

    if (m_Ui->autoCloseCheckBox->isChecked())
      this->accept();
  }
  else
  {
    m_Ui->statusLabel->setText("Some packages failed to install.");
  }
}

void QmitkPipInstallDialog::OnOutputReceived(const QString& text, bool isError)
{
  auto formatted = text.toHtmlEscaped().replace('\n', "<br>");
  auto color = isError ? "red" : "inherit";

  m_Ui->detailsTextEdit->moveCursor(QTextCursor::End);
  m_Ui->detailsTextEdit->insertHtml(
    QString("<span style=\"font-family: 'Courier New', monospace; color: %1\">%2</span>")
      .arg(color, formatted));

  AutoScrollOutput();
}

void QmitkPipInstallDialog::OnProgressChanged(int current, int total)
{
  m_Ui->progressBar->setRange(0, total);
  m_Ui->progressBar->setValue(current);
}

void QmitkPipInstallDialog::OnErrorOccurred(const QString& message)
{
  m_Ui->statusLabel->setText("Error: " + message);
  SetUiFinished(false);
}

void QmitkPipInstallDialog::OnShowAdvancedSettingsClicked(bool checked)
{
  m_Ui->advancedGroupBox->setVisible(checked);
  m_Ui->advancedButton->setText(checked
    ? "Hide advanced settings"
    : "Show advanced settings");
}

void QmitkPipInstallDialog::OnShowDetailsClicked(bool checked)
{
  m_Ui->detailsTextEdit->setVisible(checked);
  m_Ui->detailsButton->setText(checked
    ? "Hide details"
    : "Show details");
}

// --- Private helpers ---

void QmitkPipInstallDialog::SetPackageStatus(int index, mitk::PackageStatus status)
{
  auto* item = m_Ui->packageTree->topLevelItem(index);

  if (item == nullptr)
    return;

  switch (status)
  {
  case mitk::PackageStatus::Pending:
    item->setText(2, "Pending");
    item->setForeground(2, QColor(128, 128, 128));
    break;

  case mitk::PackageStatus::Installing:
    item->setText(2, "Installing...");
    item->setForeground(2, QColor(0, 120, 215));
    m_Ui->packageTree->scrollToItem(item);
    break;

  case mitk::PackageStatus::Installed:
    item->setText(2, "Installed");
    item->setForeground(2, QColor(0, 128, 0));
    break;

  case mitk::PackageStatus::Failed:
    item->setText(2, "Failed");
    item->setForeground(2, QColor(200, 0, 0));
    break;
  }
}

void QmitkPipInstallDialog::SetUiInstalling()
{
  m_IsInstalling = true;
  m_Ui->buttonBox->setEnabled(false);
  m_Ui->advancedButton->setEnabled(false);
  m_Ui->detailsTextEdit->clear();
}

void QmitkPipInstallDialog::SetUiFinished(bool success)
{
  m_IsInstalling = false;

  if (success)
  {
    m_Ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkPipInstallDialog::OnInstallClicked);
    connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  }
  else
  {
    m_Ui->buttonBox->setStandardButtons(QDialogButtonBox::Retry | QDialogButtonBox::Cancel);
    disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkPipInstallDialog::OnInstallClicked);
  }

  m_Ui->buttonBox->setEnabled(true);
  m_Ui->advancedButton->setEnabled(true);
}

void QmitkPipInstallDialog::AutoScrollOutput()
{
  if (auto* scrollBar = m_Ui->detailsTextEdit->verticalScrollBar(); scrollBar != nullptr)
    scrollBar->setValue(scrollBar->maximum());
}

void QmitkPipInstallDialog::BuildAdvancedSettings()
{
  auto* formLayout = m_Ui->advancedGroupBox->findChild<QFormLayout*>("advancedFormLayout");

  if (formLayout == nullptr)
    return;

  for (int g = 0; g < m_Spec.groups.size(); ++g)
  {
    const auto& group = m_Spec.groups[g];

    for (const auto& req : group.requirements)
    {
      // Extract package name from specifier for the label.
      auto name = req;
      auto specEnd = req.indexOf(QRegularExpression("[><=!~;@\\[]"));

      if (specEnd > 0)
        name = req.left(specEnd);

      auto* lineEdit = new QLineEdit(req, m_Ui->advancedGroupBox);
      formLayout->addRow(name + ":", lineEdit);
    }

    if (!group.indexUrl.isEmpty())
    {
      auto* lineEdit = new QLineEdit(group.indexUrl, m_Ui->advancedGroupBox);
      formLayout->addRow("--index-url:", lineEdit);
    }
  }
}
