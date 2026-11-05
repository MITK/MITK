/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMxNDynamicLayoutDialog.h>

#include <QmitkIconTheme.h>
#include <QmitkMultiNodeSelectionWidget.h>
#include <QmitkSingleNodeSelectionWidget.h>

#include <mitkExceptionMacro.h>
#include <mitkNodePredicateFunction.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QScopedValueRollback>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
  QString IconOf(QmitkMxNDynamicTemplate templateId)
  {
    switch (templateId)
    {
      case QmitkMxNDynamicTemplate::DatasetsByPlanes:  return QStringLiteral(":/Qmitk/mxn-dynamic-datasets-planes.svg");
      case QmitkMxNDynamicTemplate::OneViewPerDataset: return QStringLiteral(":/Qmitk/mxn-dynamic-one-view.svg");
      case QmitkMxNDynamicTemplate::SharedBase:        return QStringLiteral(":/Qmitk/mxn-dynamic-shared-base.svg");
      default:                                         return QStringLiteral(":/Qmitk/mxn-dynamic-fusion.svg");
    }
  }

  void ClearRows(QFormLayout* rows)
  {
    while (rows->rowCount() > 0)
    {
      rows->removeRow(0);  // deletes the row's widgets
    }
  }
}

QmitkMxNDynamicLayoutDialog::QmitkMxNDynamicLayoutDialog(mitk::DataStorage* dataStorage,
                                                         const QList<mitk::DataNode::Pointer>& selection,
                                                         const QString& widgetName,
                                                         double editorAspectRatio,
                                                         QmitkMxNDynamicTemplate initialTemplate,
                                                         QWidget* parent)
  : QDialog(parent)
  , m_DataStorage(dataStorage)
  , m_Selection(selection.begin(), selection.end())
  , m_WidgetName(widgetName)
  , m_EditorAspectRatio(editorAspectRatio)
  , m_Template(initialTemplate)
{
  if (nullptr == dataStorage)
  {
    mitkThrow() << "QmitkMxNDynamicLayoutDialog: the data storage must not be null.";
  }

  this->setWindowTitle(tr("Dynamic layout"));
  auto* layout = new QVBoxLayout(this);

  auto* templateRow = new QHBoxLayout;
  templateRow->addWidget(new QLabel(tr("Template"), this));
  m_TemplateButtons = new QButtonGroup(this);
  int index = 0;
  for (const auto& description : QmitkMxNDynamicLayouts::Templates())
  {
    auto* button = new QRadioButton(description.name, this);
    button->setObjectName(QStringLiteral("template%1").arg(index++));
    button->setIcon(QmitkIconTheme::GetIcon(IconOf(description.id)));
    button->setIconSize(QSize(32, 32));
    m_TemplateButtons->addButton(button, static_cast<int>(description.id));
    templateRow->addWidget(button);
  }
  templateRow->addStretch(1);
  layout->addLayout(templateRow);

  m_SlotRows = new QFormLayout;
  layout->addLayout(m_SlotRows);
  m_SwapButton = new QPushButton(tr("Swap roles"), this);
  m_SwapButton->setObjectName(QStringLiteral("swapRoles"));
  connect(m_SwapButton, &QPushButton::clicked, this, &QmitkMxNDynamicLayoutDialog::SwapRoles);
  layout->addWidget(m_SwapButton, 0, Qt::AlignRight);

  m_OptionRows = new QFormLayout;
  layout->addLayout(m_OptionRows);
  layout->addStretch(1);

  auto* bottom = new QHBoxLayout;
  m_Message = new QLabel(this);
  m_Message->setObjectName(QStringLiteral("message"));
  bottom->addWidget(m_Message, 1);
  auto* buttons = new QDialogButtonBox(this);
  buttons->addButton(QDialogButtonBox::Cancel);
  m_ApplyButton = buttons->addButton(tr("Apply"), QDialogButtonBox::AcceptRole);
  m_ApplyButton->setObjectName(QStringLiteral("apply"));
  // Accepting goes through OnApply, which generates the layout first.
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(m_ApplyButton, &QPushButton::clicked, this, &QmitkMxNDynamicLayoutDialog::OnApply);
  bottom->addWidget(buttons);
  layout->addLayout(bottom);

  connect(m_TemplateButtons, &QButtonGroup::idClicked, this, [this](int id)
  {
    this->SelectTemplate(static_cast<QmitkMxNDynamicTemplate>(id));
  });
  this->SelectTemplate(initialTemplate);
}

QmitkMxNDynamicLayoutDialog::~QmitkMxNDynamicLayoutDialog() = default;

const QmitkMxNDynamicLayout& QmitkMxNDynamicLayoutDialog::GetLayout() const
{
  return m_Layout;
}

QmitkMxNDynamicTemplate QmitkMxNDynamicLayoutDialog::GetTemplate() const
{
  return m_Template;
}

void QmitkMxNDynamicLayoutDialog::SelectTemplate(QmitkMxNDynamicTemplate templateId)
{
  m_Template = templateId;
  m_TemplateButtons->button(static_cast<int>(templateId))->setChecked(true);
  m_Touched.clear();
  this->BuildSlotRows();
  this->BuildOptionRows();

  const auto bindings = QmitkMxNDynamicLayouts::Prefill(templateId, m_Selection, *m_DataStorage);
  {
    const QScopedValueRollback<bool> updating(m_Updating, true);
    for (const auto& [slot, widget] : m_SlotWidgets)
    {
      QmitkAbstractNodeSelectionWidget::NodeList nodes;
      if (const auto bound = bindings.find(slot); bound != bindings.end())
      {
        nodes = QmitkAbstractNodeSelectionWidget::NodeList(bound->second.begin(), bound->second.end());
      }
      widget->SetCurrentSelection(nodes);
    }
  }
  this->OnSlotsChanged();
}

void QmitkMxNDynamicLayoutDialog::BuildSlotRows()
{
  ClearRows(m_SlotRows);
  m_SlotWidgets.clear();

  const auto& description = QmitkMxNDynamicLayouts::Template(m_Template);
  int index = 0;
  for (const auto& slot : description.slotDescriptions)
  {
    QmitkAbstractNodeSelectionWidget* widget = nullptr;
    if (1 == slot.max)
    {
      auto* single = new QmitkSingleNodeSelectionWidget(this);
      single->SetAutoSelectNewNodes(false);
      widget = single;
    }
    else
    {
      widget = new QmitkMultiNodeSelectionWidget(this);
    }
    widget->setObjectName(QStringLiteral("slot%1").arg(index++));
    widget->SetDataStorage(m_DataStorage);
    // A node another slot holds is not offered: the generator rejects a node
    // in two roles. Read live, so the predicate needs no update when another
    // slot changes.
    const auto dataClass = slot.dataClass;
    widget->SetNodePredicate(mitk::NodePredicateFunction::New([this, widget, dataClass](const mitk::DataNode* node) {
      if (nullptr == node || !QmitkMxNDynamicLayouts::IsOfDataClass(*node, dataClass))
      {
        return false;
      }
      return std::none_of(m_SlotWidgets.begin(), m_SlotWidgets.end(), [widget, node](const auto& entry) {
        if (entry.second == widget)
        {
          return false;
        }
        const auto held = entry.second->GetSelectedNodes();
        return std::any_of(held.begin(), held.end(), [node](const auto& other) { return other.GetPointer() == node; });
      });
    }));
    widget->SetSelectionIsOptional(true);
    widget->SetEmptyInfo(tr("Select %1").arg(slot.role.toLower()));
    widget->SetPopUpTitel(tr("Select %1").arg(slot.role.toLower()));
    connect(widget, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, [this]()
    {
      if (!m_Updating)
      {
        this->OnSlotsChanged();
      }
    });
    m_SlotRows->addRow(slot.role, widget);
    m_SlotWidgets.emplace_back(slot.slot, widget);
  }
  m_SwapButton->setVisible(description.swappableSlots);
}

void QmitkMxNDynamicLayoutDialog::BuildOptionRows()
{
  ClearRows(m_OptionRows);
  m_Aligned = m_ThreeDColumn = m_DatasetsAsColumns = m_ThreeDRow = m_AllOverlaysWindow = m_MipWindow = nullptr;
  m_View = m_Order = m_Colormap = m_Opacity = nullptr;

  const auto check = [this](const char* name, const QString& text, const QString& toolTip) {
    auto* box = new QCheckBox(text, this);
    box->setObjectName(QString::fromLatin1(name));
    box->setToolTip(toolTip);
    this->TrackTouched(box);
    m_OptionRows->addRow(box);
    return box;
  };
  const auto combo = [this](const char* name, const QString& label, const std::vector<std::pair<QString, int>>& items) {
    auto* box = new QComboBox(this);
    box->setObjectName(QString::fromLatin1(name));
    for (const auto& [text, value] : items)
    {
      box->addItem(text, value);
    }
    this->TrackTouched(box);
    m_OptionRows->addRow(label, box);
    return box;
  };
  const QString alignedToolTip =
    tr("Link slice, zoom, pan and crosshair across the datasets. On when they share one space: identical "
       "geometry, or one Frame of Reference.");

  switch (m_Template)
  {
    case QmitkMxNDynamicTemplate::DatasetsByPlanes:
      m_Aligned = check("aligned", tr("Aligned"), alignedToolTip);
      m_ThreeDColumn = check("threeDColumn", tr("3D column"), tr("A 3D view per dataset, for its segmentations and surfaces."));
      m_DatasetsAsColumns = check("datasetsAsColumns", tr("Datasets as columns"), tr("One column per dataset instead of one row."));
      break;
    case QmitkMxNDynamicTemplate::OneViewPerDataset:
      m_Aligned = check("aligned", tr("Aligned"), alignedToolTip);
      m_View = combo("view", tr("View"),
                     { { tr("Axial"), static_cast<int>(QmitkMxNDatasetView::Axial) },
                       { tr("Sagittal"), static_cast<int>(QmitkMxNDatasetView::Sagittal) },
                       { tr("Coronal"), static_cast<int>(QmitkMxNDatasetView::Coronal) },
                       { tr("3D"), static_cast<int>(QmitkMxNDatasetView::ThreeD) } });
      m_Order = combo("order", tr("Order"),
                      { { tr("Selection order"), static_cast<int>(QmitkMxNDatasetOrder::Selection) },
                        { tr("Acquisition date"), static_cast<int>(QmitkMxNDatasetOrder::AcquisitionDate) } });
      break;
    case QmitkMxNDynamicTemplate::SharedBase:
      m_ThreeDRow = check("threeDRow", tr("3D row"), tr("A 3D view under every window, for the segmentations and surfaces."));
      m_AllOverlaysWindow = check("allOverlaysWindow", tr("All-overlays window"), tr("One more window that shows every overlay."));
      break;
    case QmitkMxNDynamicTemplate::FusionPair:
      m_Colormap = combo("colormap", tr("Fused colormap"),
                         { { tr("PET colour"), static_cast<int>(QmitkMxNFusionColormap::PetColour) },
                           { tr("Hot iron"), static_cast<int>(QmitkMxNFusionColormap::HotIron) },
                           { tr("Jet, transparent floor"), static_cast<int>(QmitkMxNFusionColormap::JetTransparentFloor) } });
      m_MipWindow = check("mipWindow", tr("MIP window"), tr("A maximum intensity projection of the fused image."));
      m_Opacity = combo("opacity", tr("Fusion opacity"),
                        { { tr("30 %"), static_cast<int>(QmitkMxNFusionOpacity::Percent30) },
                          { tr("50 %"), static_cast<int>(QmitkMxNFusionOpacity::Percent50) },
                          { tr("70 %"), static_cast<int>(QmitkMxNFusionOpacity::Percent70) } });
      break;
  }
}

void QmitkMxNDynamicLayoutDialog::TrackTouched(QCheckBox* control)
{
  connect(control, &QCheckBox::toggled, this, [this, control]()
  {
    if (!m_Updating)
    {
      m_Touched.insert(control);
    }
  });
}

void QmitkMxNDynamicLayoutDialog::TrackTouched(QComboBox* control)
{
  connect(control, &QComboBox::currentIndexChanged, this, [this, control]()
  {
    if (!m_Updating)
    {
      m_Touched.insert(control);
    }
  });
}

bool QmitkMxNDynamicLayoutDialog::IsTouched(const QWidget* control) const
{
  return m_Touched.count(control) > 0;
}

QmitkMxNSlotBindings QmitkMxNDynamicLayoutDialog::GetBindings() const
{
  QmitkMxNSlotBindings bindings;
  for (const auto& [slot, widget] : m_SlotWidgets)
  {
    const auto nodes = widget->GetSelectedNodes();
    if (!nodes.isEmpty())
    {
      bindings[slot] = std::vector<mitk::DataNode::Pointer>(nodes.begin(), nodes.end());
    }
  }
  return bindings;
}

QmitkMxNDynamicOptions QmitkMxNDynamicLayoutDialog::GetOptions() const
{
  switch (m_Template)
  {
    case QmitkMxNDynamicTemplate::DatasetsByPlanes:
      return QmitkMxNDatasetsByPlanesOptions{ m_Aligned->isChecked(), m_ThreeDColumn->isChecked(),
                                              m_DatasetsAsColumns->isChecked() };
    case QmitkMxNDynamicTemplate::OneViewPerDataset:
      return QmitkMxNOneViewPerDatasetOptions{ m_Aligned->isChecked(),
                                               static_cast<QmitkMxNDatasetView>(m_View->currentData().toInt()),
                                               static_cast<QmitkMxNDatasetOrder>(m_Order->currentData().toInt()) };
    case QmitkMxNDynamicTemplate::SharedBase:
      return QmitkMxNSharedBaseOptions{ m_ThreeDRow->isChecked(), m_AllOverlaysWindow->isChecked() };
    case QmitkMxNDynamicTemplate::FusionPair:
    default:
      return QmitkMxNFusionPairOptions{ static_cast<QmitkMxNFusionColormap>(m_Colormap->currentData().toInt()),
                                        m_MipWindow->isChecked(),
                                        static_cast<QmitkMxNFusionOpacity>(m_Opacity->currentData().toInt()) };
  }
}

void QmitkMxNDynamicLayoutDialog::OnSlotsChanged()
{
  const auto bindings = this->GetBindings();
  const auto defaults = QmitkMxNDynamicLayouts::DefaultOptions(m_Template, bindings, *m_DataStorage);
  {
    const QScopedValueRollback<bool> updating(m_Updating, true);
    const auto follow = [this](QCheckBox* box, bool value) {
      if (nullptr != box && !this->IsTouched(box))
      {
        box->setChecked(value);
      }
    };
    const auto followCombo = [this](QComboBox* box, int value) {
      if (nullptr != box && !this->IsTouched(box))
      {
        box->setCurrentIndex(box->findData(value));
      }
    };

    switch (m_Template)
    {
      case QmitkMxNDynamicTemplate::DatasetsByPlanes:
      {
        const auto& options = std::get<QmitkMxNDatasetsByPlanesOptions>(defaults);
        follow(m_Aligned, options.aligned);
        follow(m_ThreeDColumn, options.threeDColumn);
        follow(m_DatasetsAsColumns, options.datasetsAsColumns);
        break;
      }
      case QmitkMxNDynamicTemplate::OneViewPerDataset:
      {
        const auto& options = std::get<QmitkMxNOneViewPerDatasetOptions>(defaults);
        follow(m_Aligned, options.aligned);
        followCombo(m_View, static_cast<int>(options.view));
        followCombo(m_Order, static_cast<int>(options.order));

        // The 3D view needs a 3D representation for every dataset.
        const auto datasets = bindings.find(QmitkMxNSlot::Datasets);
        const bool threeDPossible =
          datasets != bindings.end() && std::all_of(datasets->second.begin(), datasets->second.end(), [this](const auto& dataset) {
            return QmitkMxNDynamicLayouts::HasThreeDRepresentation(*dataset, *m_DataStorage);
          });
        const int threeD = m_View->findData(static_cast<int>(QmitkMxNDatasetView::ThreeD));
        if (auto* model = qobject_cast<QStandardItemModel*>(m_View->model()); nullptr != model)
        {
          model->item(threeD)->setEnabled(threeDPossible);
        }
        if (!threeDPossible && threeD == m_View->currentIndex())
        {
          m_View->setCurrentIndex(m_View->findData(static_cast<int>(QmitkMxNDatasetView::Axial)));
        }
        break;
      }
      case QmitkMxNDynamicTemplate::SharedBase:
      {
        const auto& options = std::get<QmitkMxNSharedBaseOptions>(defaults);
        follow(m_ThreeDRow, options.threeDRow);
        follow(m_AllOverlaysWindow, options.allOverlaysWindow);
        break;
      }
      case QmitkMxNDynamicTemplate::FusionPair:
      {
        const auto& options = std::get<QmitkMxNFusionPairOptions>(defaults);
        followCombo(m_Colormap, static_cast<int>(options.colormap));
        follow(m_MipWindow, options.mipWindow);
        followCombo(m_Opacity, static_cast<int>(options.opacity));
        break;
      }
    }
  }
  this->UpdateApplyState();
}

void QmitkMxNDynamicLayoutDialog::UpdateApplyState()
{
  const auto bindings = this->GetBindings();
  QString message;
  for (const auto& slot : QmitkMxNDynamicLayouts::Template(m_Template).slotDescriptions)
  {
    const auto bound = bindings.find(slot.slot);
    const int count = bound != bindings.end() ? static_cast<int>(bound->second.size()) : 0;
    const auto role = slot.role.toLower();
    if (0 == count && slot.min > 0)
    {
      message = tr("Select the %1").arg(role);
    }
    else if (count < slot.min)
    {
      message = tr("Select at least %1 %2").arg(slot.min).arg(role);
    }
    else if (count > slot.max)
    {
      message = tr("Select at most %1 %2").arg(slot.max).arg(role);
    }
    if (!message.isEmpty())
    {
      break;
    }
  }
  m_Message->setText(message);
  m_ApplyButton->setEnabled(message.isEmpty());
}

void QmitkMxNDynamicLayoutDialog::SwapRoles()
{
  if (2 != m_SlotWidgets.size())
  {
    return;
  }
  auto* first = m_SlotWidgets[0].second;
  auto* second = m_SlotWidgets[1].second;
  const auto firstNodes = first->GetSelectedNodes();
  const auto secondNodes = second->GetSelectedNodes();
  {
    const QScopedValueRollback<bool> updating(m_Updating, true);
    // Emptied first: a slot refuses a node the other one still holds.
    first->SetCurrentSelection({});
    second->SetCurrentSelection({});
    first->SetCurrentSelection(secondNodes);
    second->SetCurrentSelection(firstNodes);
  }
  this->OnSlotsChanged();
}

void QmitkMxNDynamicLayoutDialog::OnApply()
{
  try
  {
    m_Layout = QmitkMxNDynamicLayouts::Generate(this->GetBindings(), this->GetOptions(), m_WidgetName,
                                                m_EditorAspectRatio, *m_DataStorage);
  }
  catch (const mitk::Exception& e)
  {
    m_Message->setText(QString::fromStdString(e.GetDescription()));
    return;
  }
  this->accept();
}
