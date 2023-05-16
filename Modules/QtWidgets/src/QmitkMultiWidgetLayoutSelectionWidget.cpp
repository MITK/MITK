/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiWidgetLayoutSelectionWidget.h"

#include <QFileDialog>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

QmitkMultiWidgetLayoutSelectionWidget::QmitkMultiWidgetLayoutSelectionWidget(QWidget* parent/* = 0*/)
  : QWidget(parent)
{
  Init();
}

void QmitkMultiWidgetLayoutSelectionWidget::Init()
{
  ui.setupUi(this);

  auto stylesheet = "QTableWidget::item{background-color: white;}\nQTableWidget::item:selected{background-color: #1C97EA;}";
  ui.tableWidget->setStyleSheet(stylesheet);

  connect(ui.tableWidget, &QTableWidget::itemSelectionChanged, this, &QmitkMultiWidgetLayoutSelectionWidget::OnTableItemSelectionChanged);
  connect(ui.setLayoutPushButton, &QPushButton::clicked, this, &QmitkMultiWidgetLayoutSelectionWidget::OnSetLayoutButtonClicked);
  connect(ui.loadLayoutPushButton, &QPushButton::clicked, this, &QmitkMultiWidgetLayoutSelectionWidget::OnLoadLayoutButtonClicked);
  connect(ui.saveLayoutPushButton, &QPushButton::clicked, this, &QmitkMultiWidgetLayoutSelectionWidget::OnSaveLayoutButtonClicked);
  connect(ui.selectDefaultLayoutComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QmitkMultiWidgetLayoutSelectionWidget::OnLayoutPresetSelected);

  ui.selectDefaultLayoutComboBox->addItem("Select a layout preset");
  auto presetResources = us::GetModuleContext()->GetModule()->FindResources("/", "mxnLayout_*.json", false);
  for (const auto& resource : presetResources)
  {
    us::ModuleResourceStream jsonStream(resource);
    auto data = nlohmann::json::parse(jsonStream);
    auto resourceName = data["name"].get<std::string>();
    ui.selectDefaultLayoutComboBox->addItem(QString::fromStdString(resourceName));
    m_PresetMap[ui.selectDefaultLayoutComboBox->count() - 1] = data;
  }
}

void QmitkMultiWidgetLayoutSelectionWidget::OnTableItemSelectionChanged()
{
  QItemSelectionModel* selectionModel = ui.tableWidget->selectionModel();

  int row = 0;
  int column = 0;
  QModelIndexList indices = selectionModel->selectedIndexes();
  if (indices.size() > 0)
  {
    row = indices[0].row();
    column = indices[0].column();

    QModelIndex topLeft = ui.tableWidget->model()->index(0, 0, QModelIndex());
    QModelIndex bottomRight = ui.tableWidget->model()->index(row, column, QModelIndex());

    QItemSelection cellSelection;
    cellSelection.select(topLeft, bottomRight);
    selectionModel->select(cellSelection, QItemSelectionModel::Select);
  }
}

void QmitkMultiWidgetLayoutSelectionWidget::OnSetLayoutButtonClicked()
{
  int row = 0;
  int column = 0;
  QModelIndexList indices = ui.tableWidget->selectionModel()->selectedIndexes();
  if (indices.size() > 0)
  {
    // find largest row and column
    for (const auto& modelIndex : qAsConst(indices))
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
  ui.selectDefaultLayoutComboBox->setCurrentIndex(0);
}

void QmitkMultiWidgetLayoutSelectionWidget::OnSaveLayoutButtonClicked()
{
  QString filename = QFileDialog::getSaveFileName(nullptr, "Select where to save the current layout", "", "MITK Window Layout (*.json)");
  if (filename.isEmpty())
    return;

  QString fileExt(".json");
  if (!filename.endsWith(fileExt))
    filename += fileExt;

  auto outStream = std::ofstream(filename.toStdString());
  emit SaveLayout(&outStream);
}

void QmitkMultiWidgetLayoutSelectionWidget::OnLoadLayoutButtonClicked()
{
  QString filename = QFileDialog::getOpenFileName(nullptr, "Load a layout file", "", "MITK Window Layouts (*.json)");
  if (filename.isEmpty())
    return;

  ui.selectDefaultLayoutComboBox->setCurrentIndex(0);

  std::ifstream f(filename.toStdString());
  auto jsonData = nlohmann::json::parse(f);
  emit LoadLayout(&jsonData);
}

void QmitkMultiWidgetLayoutSelectionWidget::OnLayoutPresetSelected(int index)
{
  if (index == 0)
  {
    // First entry is only for description
    return;
  }

  auto jsonData = m_PresetMap[index];
  close();
  emit LoadLayout(&jsonData);
}
