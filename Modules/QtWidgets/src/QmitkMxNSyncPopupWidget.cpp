/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMxNSyncPopupWidget.h>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QToolButton>
#include <QToolTip>

namespace
{
  QString NotLinkedEntry()
  {
    return QmitkMxNSyncPopupWidget::tr("(not linked)");
  }
}

QmitkMxNSyncPopupWidget::QmitkMxNSyncPopupWidget(QWidget* parent)
  : QWidget(parent)
{
  auto* grid = new QGridLayout(this);
  grid->setContentsMargins(4, 4, 4, 4);

  const struct
  {
    QmitkMxNSyncDimension dimension;
    const char* label;
  } rowSpecs[] = {
    { QmitkMxNSyncDimension::Pan, "Pan" },
    { QmitkMxNSyncDimension::Zoom, "Zoom" },
    { QmitkMxNSyncDimension::Slice, "Slice" },
    { QmitkMxNSyncDimension::Crosshair, "Crosshair" },
    { QmitkMxNSyncDimension::Orientation, "Orientation" },
    { QmitkMxNSyncDimension::Windowing, "Windowing" },
    { QmitkMxNSyncDimension::Lut, "LUT" },
  };

  int gridRow = 0;
  for (const auto& spec : rowSpecs)
  {
    Row row;
    row.dimension = spec.dimension;

    grid->addWidget(new QLabel(spec.label, this), gridRow, 0);

    row.groupSelector = new QComboBox(this);
    row.groupSelector->setEditable(true);
    row.groupSelector->setMinimumContentsLength(8);
    row.groupSelector->setInsertPolicy(QComboBox::NoInsert);
    row.groupSelector->addItem(NotLinkedEntry());
    row.groupSelector->setToolTip(
      tr("Pick a group to link this cell, or type a new group name to create one.\n"
         "Group names use letters, digits, and _ . - (no spaces).\n"
         "Pick \"(not linked)\" to unlink."));
    grid->addWidget(row.groupSelector, gridRow, 1);

    switch (spec.dimension)
    {
      case QmitkMxNSyncDimension::Pan:
        row.panOffsetX = new QDoubleSpinBox(this);
        row.panOffsetY = new QDoubleSpinBox(this);
        for (auto* spin : { row.panOffsetX, row.panOffsetY })
        {
          spin->setRange(-10000.0, 10000.0);
          spin->setDecimals(1);
          spin->setSuffix(tr(" mm"));
        }
        row.panOffsetX->setToolTip(tr("In-plane offset x (world mm), applied on converge"));
        row.panOffsetY->setToolTip(tr("In-plane offset y (world mm), applied on converge"));
        grid->addWidget(row.panOffsetX, gridRow, 2);
        grid->addWidget(row.panOffsetY, gridRow, 3);
        break;
      case QmitkMxNSyncDimension::Zoom:
        row.zoomOffset = new QDoubleSpinBox(this);
        row.zoomOffset->setRange(0.01, 100.0);
        row.zoomOffset->setSingleStep(0.1);
        row.zoomOffset->setValue(1.0);
        row.zoomOffset->setToolTip(tr("Zoom factor relative to the group's seed cell"));
        grid->addWidget(row.zoomOffset, gridRow, 2, 1, 2);
        break;
      case QmitkMxNSyncDimension::Slice:
        row.sliceOffset = new QSpinBox(this);
        row.sliceOffset->setRange(-999, 999);
        row.sliceOffset->setToolTip(tr("Slice-step offset relative to the group's seed cell"));
        grid->addWidget(row.sliceOffset, gridRow, 2, 1, 2);
        break;
      default:
        break;
    }

    // Only the offset dimensions carry convergence bookkeeping.
    if (QmitkMxNSyncDimension::Pan == spec.dimension || QmitkMxNSyncDimension::Zoom == spec.dimension
        || QmitkMxNSyncDimension::Slice == spec.dimension)
    {
      row.reconvergeButton = new QToolButton(this);
      row.reconvergeButton->setText(tr("Re-converge"));
      row.reconvergeButton->setToolTip(
        tr("Re-establish every group member at the seed cell's state plus its offset"));
      grid->addWidget(row.reconvergeButton, gridRow, 4);
    }

    m_Rows.push_back(row);
    ++gridRow;
  }

  auto* reinitButton = new QToolButton(this);
  reinitButton->setText(tr("Reinit group geometry"));
  reinitButton->setToolTip(
    tr("Re-initialize this cell's slice/orientation link neighborhood to one shared geometry"));
  connect(reinitButton, &QToolButton::clicked, this, [this]() { emit ReinitGeometryRequested(); });
  grid->addWidget(reinitButton, gridRow, 0, 1, 2);

  // Wire after all rows exist; m_Rows is stable from here on, so capturing
  // element references stays valid.
  for (auto& row : m_Rows)
  {
    auto emitChange = [this, &row]() { this->EmitLinkChange(row); };

    // Commit on explicit selection and on finishing a typed edit; reacting
    // to every keystroke would create/drop groups while the name is still
    // being typed.
    connect(row.groupSelector, QOverload<int>::of(&QComboBox::activated), this, emitChange);
    connect(row.groupSelector->lineEdit(), &QLineEdit::editingFinished, this, emitChange);

    for (auto* spin : { row.panOffsetX, row.panOffsetY, row.zoomOffset })
    {
      if (nullptr != spin)
      {
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, emitChange);
      }
    }
    if (nullptr != row.sliceOffset)
    {
      connect(row.sliceOffset, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChange);
    }
    if (nullptr != row.reconvergeButton)
    {
      connect(row.reconvergeButton, &QToolButton::clicked, this, [this, &row]()
      {
        const auto group = row.groupSelector->currentText().trimmed();
        if (!group.isEmpty() && group != NotLinkedEntry())
        {
          emit ReconvergeRequested(row.dimension, group);
        }
      });
    }
  }
}

void QmitkMxNSyncPopupWidget::SetKnownGroups(QmitkMxNSyncDimension dimension, const QStringList& groups)
{
  auto* row = this->FindRow(dimension);
  if (nullptr == row)
  {
    return;
  }
  const QSignalBlocker blocker(row->groupSelector);
  const auto currentText = row->groupSelector->currentText();
  row->groupSelector->clear();
  row->groupSelector->addItem(NotLinkedEntry());
  row->groupSelector->addItems(groups);
  row->groupSelector->setCurrentText(currentText);
}

void QmitkMxNSyncPopupWidget::SetLinkState(QmitkMxNSyncDimension dimension,
                                           const QString& group,
                                           const QmitkMxNMultiWidget::SyncOffset& offset)
{
  auto* row = this->FindRow(dimension);
  if (nullptr == row)
  {
    return;
  }

  {
    const QSignalBlocker blocker(row->groupSelector);
    if (group.isEmpty())
    {
      row->groupSelector->setCurrentIndex(0); // the "(not linked)" entry
    }
    else
    {
      row->groupSelector->setCurrentText(group);
    }
  }
  if (nullptr != row->sliceOffset)
  {
    const QSignalBlocker blocker(row->sliceOffset);
    row->sliceOffset->setValue(std::holds_alternative<int>(offset) ? std::get<int>(offset) : 0);
  }
  if (nullptr != row->zoomOffset)
  {
    const QSignalBlocker blocker(row->zoomOffset);
    row->zoomOffset->setValue(std::holds_alternative<double>(offset) ? std::get<double>(offset) : 1.0);
  }
  if (nullptr != row->panOffsetX)
  {
    const auto panOffset = std::holds_alternative<mitk::Vector2D>(offset)
      ? std::get<mitk::Vector2D>(offset) : mitk::Vector2D(0.0);
    const QSignalBlocker blockerX(row->panOffsetX);
    const QSignalBlocker blockerY(row->panOffsetY);
    row->panOffsetX->setValue(panOffset[0]);
    row->panOffsetY->setValue(panOffset[1]);
  }
}

QmitkMxNSyncPopupWidget::Row* QmitkMxNSyncPopupWidget::FindRow(QmitkMxNSyncDimension dimension)
{
  for (auto& row : m_Rows)
  {
    if (row.dimension == dimension)
    {
      return &row;
    }
  }
  return nullptr;
}

QmitkMxNMultiWidget::SyncOffset QmitkMxNSyncPopupWidget::CurrentOffset(const Row& row) const
{
  switch (row.dimension)
  {
    case QmitkMxNSyncDimension::Slice:
      return row.sliceOffset->value();
    case QmitkMxNSyncDimension::Zoom:
      return row.zoomOffset->value();
    case QmitkMxNSyncDimension::Pan:
    {
      mitk::Vector2D panOffset;
      panOffset[0] = row.panOffsetX->value();
      panOffset[1] = row.panOffsetY->value();
      return panOffset;
    }
    default:
      return {};
  }
}

void QmitkMxNSyncPopupWidget::EmitLinkChange(Row& row)
{
  auto group = row.groupSelector->currentText().trimmed();
  if (group == NotLinkedEntry())
  {
    group.clear();
  }
  emit LinkChangeRequested(row.dimension, group, this->CurrentOffset(row));
}

void QmitkMxNSyncPopupWidget::ShowLinkError(QmitkMxNSyncDimension dimension, const QString& message)
{
  auto* row = this->FindRow(dimension);
  if (nullptr == row)
  {
    return;
  }
  QToolTip::showText(row->groupSelector->mapToGlobal(QPoint(0, row->groupSelector->height())),
                     message, row->groupSelector);
}
