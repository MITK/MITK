/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiWidgetLayoutSelectionWidget.h"
#include <ui_QmitkMultiWidgetLayoutSelectionWidget.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QPalette>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

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

  m_AutomatedDataLayoutWidget = new QmitkAutomatedLayoutWidget(this);
  connect(m_AutomatedDataLayoutWidget, &QmitkAutomatedLayoutWidget::SetDataBasedLayout, this, &QmitkMultiWidgetLayoutSelectionWidget::SetDataBasedLayout);
  m_AutomatedDataLayoutWidget->hide();

  connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &QmitkMultiWidgetLayoutSelectionWidget::OnTableItemSelectionChanged);
  connect(ui->setLayoutPushButton, &QPushButton::clicked, this, &QmitkMultiWidgetLayoutSelectionWidget::OnSetLayoutButtonClicked);
  connect(ui->dataBasedLayoutButton, &QPushButton::clicked, this, &QmitkMultiWidgetLayoutSelectionWidget::OnDataBasedLayoutButtonClicked);

  // The presets ship as module resources; they are read once here and offered
  // by name, so the host can present them without knowing about the resources.
  auto presetResources = us::GetModuleContext()->GetModule()->FindResources("/", "mxnLayout_*.json", false);
  for (const auto& resource : presetResources)
  {
    us::ModuleResourceStream jsonStream(resource);
    auto data = nlohmann::json::parse(jsonStream);
    m_Presets.push_back({ QString::fromStdString(data["name"].get<std::string>()), data });
  }
}

void QmitkMultiWidgetLayoutSelectionWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_AutomatedDataLayoutWidget == nullptr)
    return;
  m_AutomatedDataLayoutWidget->SetDataStorage(dataStorage);
}

void QmitkMultiWidgetLayoutSelectionWidget::ResetSelection()
{
  ui->tableWidget->clearSelection();
}

QStringList QmitkMultiWidgetLayoutSelectionWidget::PresetNames() const
{
  QStringList names;
  names.reserve(static_cast<int>(m_Presets.size()));
  for (const auto& preset : m_Presets)
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

void QmitkMultiWidgetLayoutSelectionWidget::OnDataBasedLayoutButtonClicked()
{
  this->hide();
  m_AutomatedDataLayoutWidget->setWindowFlags(Qt::Popup);
  m_AutomatedDataLayoutWidget->move(this->pos().x() - m_AutomatedDataLayoutWidget->width() + this->width(), this->pos().y());
  m_AutomatedDataLayoutWidget->show();
}

void QmitkMultiWidgetLayoutSelectionWidget::RequestSave()
{
  QString filename = QFileDialog::getSaveFileName(nullptr, "Select where to save the current layout", "", "MITK Window Layout (*.json)");
  if (filename.isEmpty())
    return;

  QString fileExt(".json");
  if (!filename.endsWith(fileExt))
    filename += fileExt;

  // Wrap the save emit so any failure (engine layout invariant violation,
  // serializer pre-walk inconsistency, ...) surfaces as a user-facing
  // message rather than escaping into the Qt event dispatcher. Symmetric
  // with the load path below.
  try
  {
    auto outStream = std::ofstream(filename.toStdString());
    emit SaveLayout(&outStream);
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Layout save failed"),
                         QString::fromUtf8(e.what()));
  }
}

void QmitkMultiWidgetLayoutSelectionWidget::RequestLoad()
{
  QString filename = QFileDialog::getOpenFileName(nullptr, "Load a layout file", "", "MITK Window Layouts (*.json)");
  if (filename.isEmpty())
    return;

  // Wrap parse + apply in a single catch frame so any failure (file I/O,
  // JSON parse error, schema-shape violation, missing group reference,
  // unknown view_direction, ...) surfaces as a user-facing message rather
  // than letting the exception escape into the Qt event dispatcher.
  try
  {
    std::ifstream f(filename.toStdString());
    auto jsonData = nlohmann::json::parse(f);
    emit LoadLayout(&jsonData);
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Layout load failed"),
                         QString::fromUtf8(e.what()));
  }
}

void QmitkMultiWidgetLayoutSelectionWidget::ApplyPreset(int index)
{
  if (index < 0 || static_cast<std::size_t>(index) >= m_Presets.size())
  {
    return;
  }

  // Same catch frame as the file paths: a preset the engine rejects surfaces as
  // a message rather than escaping into the Qt event dispatcher.
  try
  {
    emit LoadLayout(&m_Presets[index].layout);
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, tr("Layout load failed"),
                         QString::fromUtf8(e.what()));
  }
}
