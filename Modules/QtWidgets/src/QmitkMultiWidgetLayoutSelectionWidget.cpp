/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiWidgetLayoutSelectionWidget.h"
#include <ui_QmitkMultiWidgetLayoutSelectionWidget.h>

#include <QmitkMxNPresets.h>

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPalette>
#include <QSaveFile>

#include <sstream>


QmitkMultiWidgetLayoutSelectionWidget::QmitkMultiWidgetLayoutSelectionWidget(QWidget* parent/* = 0*/)
  : QWidget(parent)
  , ui(std::make_unique<Ui::QmitkMultiWidgetLayoutSelectionWidget>())
{
  this->Init();
}

QmitkMultiWidgetLayoutSelectionWidget::~QmitkMultiWidgetLayoutSelectionWidget()
{
}

void QmitkMultiWidgetLayoutSelectionWidget::Init()
{
  ui->setupUi(this);

  // The table is a grid preview, not a data view: its cells are painted flat
  // and the picked extent is marked by the selection color. Both come from the
  // palette so the preview follows the active theme.
  const QPalette& palette = ui->tableWidget->palette();
  ui->tableWidget->setStyleSheet(
    QStringLiteral("QTableWidget::item { background-color: %1; }\n"
                   "QTableWidget::item:selected { background-color: %2; }")
      .arg(palette.color(QPalette::Base).name(), palette.color(QPalette::Highlight).name()));

  connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &QmitkMultiWidgetLayoutSelectionWidget::OnTableItemSelectionChanged);
  connect(ui->setLayoutPushButton, &QPushButton::clicked, this, &QmitkMultiWidgetLayoutSelectionWidget::OnSetLayoutButtonClicked);
  connect(ui->dynamicLayoutButton, &QPushButton::clicked, this, &QmitkMultiWidgetLayoutSelectionWidget::DynamicLayoutRequested);

}

void QmitkMultiWidgetLayoutSelectionWidget::ResetSelection()
{
  ui->tableWidget->clearSelection();
}

QStringList QmitkMultiWidgetLayoutSelectionWidget::PresetNames() const
{
  QStringList names;
  for (const auto& preset : QmitkMxNPresets::All())
  {
    names.append(preset.name);
  }
  return names;
}

void QmitkMultiWidgetLayoutSelectionWidget::OnTableItemSelectionChanged()
{
  QItemSelectionModel* selectionModel = ui->tableWidget->selectionModel();

  int row = 0;
  int column = 0;
  QModelIndexList indices = selectionModel->selectedIndexes();
  if (indices.size() > 0)
  {
    row = indices[0].row();
    column = indices[0].column();

    QModelIndex topLeft = ui->tableWidget->model()->index(0, 0, QModelIndex());
    QModelIndex bottomRight = ui->tableWidget->model()->index(row, column, QModelIndex());

    QItemSelection cellSelection;
    cellSelection.select(topLeft, bottomRight);
    selectionModel->select(cellSelection, QItemSelectionModel::Select);
  }
}

void QmitkMultiWidgetLayoutSelectionWidget::OnSetLayoutButtonClicked()
{
  int row = 0;
  int column = 0;
  QModelIndexList indices = ui->tableWidget->selectionModel()->selectedIndexes();
  if (indices.size() > 0)
  {
    // find largest row and column
    for (const auto& modelIndex : std::as_const(indices))
    {
      if (modelIndex.row() > row)
      {
        row = modelIndex.row();
      }
      if (modelIndex.column() > column)
      {
        column = modelIndex.column();
      }
    }

    close();
    emit LayoutSet(row+1, column+1);
  }
}

void QmitkMultiWidgetLayoutSelectionWidget::RequestSave()
{
  QString filename = QFileDialog::getSaveFileName(nullptr, "Select where to save the current layout", "", "MITK Window Layout (*.json)");
  if (filename.isEmpty())
    return;

  QString fileExt(".json");
  if (!filename.endsWith(fileExt))
    filename += fileExt;

  // Serialized in full before the file is touched, so a failing serializer
  // (a layout invariant violation) leaves an existing file intact. The handler
  // serializes synchronously, so its exceptions surface here.
  std::ostringstream buffer;
  try
  {
    emit SaveLayout(&buffer);
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Layout save failed"),
                         QString::fromUtf8(e.what()));
    return;
  }

  const auto content = buffer.str();
  if (content.empty())
  {
    return;
  }

  QSaveFile file(filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)
      || file.write(content.data(), static_cast<qint64>(content.size())) < 0
      || !file.commit())
  {
    QMessageBox::warning(this, tr("Layout save failed"),
                         tr("Cannot write %1: %2").arg(filename, file.errorString()));
  }
}

void QmitkMultiWidgetLayoutSelectionWidget::RequestLoad()
{
  QString filename = QFileDialog::getOpenFileName(nullptr, "Load a layout file", "", "MITK Window Layouts (*.json)");
  if (filename.isEmpty())
    return;

  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
  {
    QMessageBox::warning(this, tr("Layout load failed"),
                         tr("Cannot read %1: %2").arg(filename, file.errorString()));
    return;
  }
  const QByteArray content = file.readAll();

  // Only the file's JSON syntax is checked here; the receiver of LoadLayout
  // validates and applies the document and reports its own failures.
  nlohmann::json jsonData;
  try
  {
    jsonData = nlohmann::json::parse(content.constData(), content.constData() + content.size());
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Layout load failed"),
                         QString::fromUtf8(e.what()));
    return;
  }
  emit LoadLayout(&jsonData);
}

void QmitkMultiWidgetLayoutSelectionWidget::ApplyPreset(int index, const QString& widgetName)
{
  const auto& presets = QmitkMxNPresets::All();
  if (index < 0 || static_cast<std::size_t>(index) >= presets.size())
  {
    return;
  }

  auto layout = QmitkMxNPresets::DocumentFor(presets[index], widgetName);
  emit LoadLayout(&layout);
}
