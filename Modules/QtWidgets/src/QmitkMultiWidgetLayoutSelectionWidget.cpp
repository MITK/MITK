/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiWidgetLayoutSelectionWidget.h"
#include <ui_QmitkMultiWidgetLayoutSelectionWidget.h>

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPalette>
#include <QSaveFile>

#include <sstream>

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
  // Taken before the host reacts to DataBasedLayoutStarted, while this picker is
  // still on screen. A popup is a top-level window, so it is placed in global
  // coordinates.
  const QPoint topRight = this->mapToGlobal(QPoint(this->width(), 0));
  emit DataBasedLayoutStarted();

  m_AutomatedDataLayoutWidget->setWindowFlags(Qt::Popup);
  // A widget that was never shown has no laid-out size yet.
  m_AutomatedDataLayoutWidget->adjustSize();
  m_AutomatedDataLayoutWidget->move(topRight.x() - m_AutomatedDataLayoutWidget->width(), topRight.y());
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

void QmitkMultiWidgetLayoutSelectionWidget::ApplyPreset(int index)
{
  if (index < 0 || static_cast<std::size_t>(index) >= m_Presets.size())
  {
    return;
  }

  emit LoadLayout(&m_Presets[index].layout);
}
