/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkFindSegmentationTaskDialog.h>
#include <ui_QmitkFindSegmentationTaskDialog.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

namespace
{
  struct Preferences
  {
    std::vector<std::byte> geometry;
    std::array<int, 7> columnWidths;
  };

  Preferences GetPreferences()
  {
    auto* node = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentationtasklist");

    Preferences prefs;

    prefs.geometry = node->GetByteArray("QmitkFindSegmentationTaskDialog geometry", nullptr, 0);

    for (size_t column = 0; column < prefs.columnWidths.size(); ++column)
      prefs.columnWidths[column] = node->GetInt("QmitkFindSegmentationTaskDialog column width " + std::to_string(column), 125);

    return prefs;
  }

  void SaveGeometry(const QByteArray& geometry)
  {
    auto* node = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentationtasklist");
    node->PutByteArray("QmitkFindSegmentationTaskDialog geometry", reinterpret_cast<const std::byte*>(geometry.data()), geometry.size());
  }

  void SaveColumnWidths(const QTableWidget* table)
  {
    auto* node = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentationtasklist");
    const int numberOfColumns = table->columnCount();

    for (int column = 0; column < numberOfColumns; ++column)
      node->PutInt("QmitkFindSegmentationTaskDialog column width " + std::to_string(column), table->columnWidth(column));
  }
}

QmitkFindSegmentationTaskDialog::QmitkFindSegmentationTaskDialog(QWidget* parent)
  : QDialog(parent),
    m_Ui(new Ui::QmitkFindSegmentationTaskDialog)
{
  this->setWindowFlag(Qt::WindowContextHelpButtonHint, false);

  m_Ui->setupUi(this);

  using Self = QmitkFindSegmentationTaskDialog;

  connect(m_Ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &Self::OnItemSelectionChanged);
  connect(m_Ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &Self::OnItemDoubleClicked);
  connect(this, &Self::finished, this, &Self::OnFinished);

  auto applyFilter = [this](const QString&) { this->ApplyFilter(); };

  connect(m_Ui->noLineEdit, &QLineEdit::textChanged, this, applyFilter);
  connect(m_Ui->nameLineEdit, &QLineEdit::textChanged, this, applyFilter);
  connect(m_Ui->statusComboBox, &QComboBox::currentTextChanged, applyFilter);

  auto prefs = GetPreferences();

  if (!prefs.geometry.empty())
    this->restoreGeometry(QByteArray(reinterpret_cast<const char*>(prefs.geometry.data()), prefs.geometry.size()));

  for (int column = 0; column < static_cast<int>(prefs.columnWidths.size()); ++column)
    m_Ui->tableWidget->setColumnWidth(column, prefs.columnWidths[column]);
}

QmitkFindSegmentationTaskDialog::~QmitkFindSegmentationTaskDialog()
{
  delete m_Ui;
}

void QmitkFindSegmentationTaskDialog::SetTaskList(const mitk::SegmentationTaskList* taskList)
{
  if (m_TaskList != taskList)
  {
    m_TaskList = taskList;

    auto* table = m_Ui->tableWidget;
    table->setSortingEnabled(false); // Do not sort while populating the table
    table->reset();

    if (m_TaskList.IsNotNull())
    {
      const int numberOfRows = static_cast<int>(m_TaskList->GetNumberOfTasks());
      const int numberOfColumns = table->columnCount();

      table->setRowCount(numberOfRows);

      for (int row = 0; row < numberOfRows; ++row)
      {
        for (int column = 0; column < numberOfColumns; ++column)
        {
          auto* item = new QTableWidgetItem();
          item->setData(Qt::UserRole, row);

          switch (column)
          {
          case Column::Number:
            item->setText(QString::number(row + 1));
            break;

          case Column::Name:
            if (m_TaskList->HasName(row))
              item->setText(QString::fromStdString(m_TaskList->GetName(row)));

            break;

          case Column::Status:
            item->setText(m_TaskList->IsDone(row) ? "Done" : "Not done");
            break;

          case Column::Image:
            if (m_TaskList->HasImage(row))
              item->setText(QString::fromStdString(m_TaskList->GetImage(row).string()));

            break;

          case Column::Segmentation:
            if (m_TaskList->HasSegmentation(row))
              item->setText(QString::fromStdString(m_TaskList->GetSegmentation(row).string()));

            break;

          case Column::Result:
            if (m_TaskList->HasResult(row))
              item->setText(QString::fromStdString(m_TaskList->GetResult(row).string()));

            break;

          case Column::Description:
            if (m_TaskList->HasDescription(row))
              item->setText(QString::fromStdString(m_TaskList->GetDescription(row)));

            break;
          }

          table->setItem(row, column, item);
        }
      }

      table->setSortingEnabled(true); // Sort fully populated table
    }
  }
}

std::optional<size_t> QmitkFindSegmentationTaskDialog::GetSelectedTask() const
{
  return m_SelectedTask;
}

bool QmitkFindSegmentationTaskDialog::LoadSelectedTask() const
{
  return m_Ui->loadCheckBox->isChecked();
}

void QmitkFindSegmentationTaskDialog::OnItemSelectionChanged()
{
  auto selectedItems = m_Ui->tableWidget->selectedItems();

  if (!selectedItems.empty())
  {
    m_SelectedTask = static_cast<size_t>(selectedItems.front()->data(Qt::UserRole).toInt());
  }
  else
  {
    m_SelectedTask.reset();
  }
}

void QmitkFindSegmentationTaskDialog::OnItemDoubleClicked(QTableWidgetItem*)
{
  this->done(m_SelectedTask.has_value() ? QDialog::Accepted : QDialog::Rejected);
}

void QmitkFindSegmentationTaskDialog::OnFinished(int)
{
  SaveGeometry(this->saveGeometry());
  SaveColumnWidths(m_Ui->tableWidget);
}

void QmitkFindSegmentationTaskDialog::ApplyFilter()
{
  auto* table = m_Ui->tableWidget;
  const int numberOfRows = table->rowCount();
  bool foundSomething = false;

  for (int row = 0; row < numberOfRows; ++row)
  {
    if (!this->ContainsNumber(row) || !this->ContainsName(row) || !this->HasStatus(row))
    {
      table->hideRow(row);
      continue;
    }

    table->showRow(row);

    if (!foundSomething)
    {
      // Select the *first* finding to accelerate user interaction. For example, in a fresh dialog, typing
      // '4', '2', '<Return>' will close the dialog and load task 42. In combination with the Ctrl+F shortcut
      // for this dialog, no mouse interaction is necessary at all to jump to specific tasks.
      table->selectRow(row);
      foundSomething = true;
    }
  }

  if (!foundSomething)
    table->clearSelection(); // Hidden rows do not automatically lose selection
}

bool QmitkFindSegmentationTaskDialog::ContainsNumber(int row) const
{
  auto numberText = m_Ui->noLineEdit->text().trimmed();
  return numberText.isEmpty() || m_Ui->tableWidget->item(row, Column::Number)->text().contains(numberText);
}

bool QmitkFindSegmentationTaskDialog::ContainsName(int row) const
{
  auto name = m_Ui->nameLineEdit->text().trimmed();
  return name.isEmpty() || m_Ui->tableWidget->item(row, Column::Name)->text().contains(name, Qt::CaseInsensitive);
}

bool QmitkFindSegmentationTaskDialog::HasStatus(int row) const
{
  auto status = m_Ui->statusComboBox->currentText();
  return status.isEmpty() || status == QStringLiteral("All") || m_Ui->tableWidget->item(row, Column::Status)->text() == status;
}
