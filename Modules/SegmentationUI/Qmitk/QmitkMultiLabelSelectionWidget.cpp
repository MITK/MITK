/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelSelectionWidget.h>

// mitk
#include <mitkLabelSetImageConverter.h>

#include <QmitkMultiLabelInspector.h>
#include <QmitkMultiLabelSelectionButton.h>

// Qt
#include <QKeyEvent>
#include <QEvent>
#include <QEnterEvent>
#include <QApplication>
#include <QScreen>
#include <QPushButton>

#include <ui_QmitkMultiLabelSelectionWidget.h>


QmitkMultiLabelSelectionWidget::QmitkMultiLabelSelectionWidget(QWidget* parent/* = nullptr*/)
  : QWidget(parent),
  m_Controls(new Ui::QmitkMultiLabelSelectionWidget),
  m_Popup(new QmitkMultiLabelInspectorPopup(this))
{
  m_Controls->setupUi(this);

  m_Controls->labelSelection->SetEmptyInfo(QStringLiteral("<font class=\"warning\">Select label to proceed.</font>"));
  m_Popup->GetInspector()->SetMultiSelectionMode(false);

  connect(m_Controls->labelSelection, &QPushButton::clicked, this, &QmitkMultiLabelSelectionWidget::ShowPopup);

  connect(m_Popup.get(), &QmitkMultiLabelInspectorPopup::SelectionFinished,
    this, &QmitkMultiLabelSelectionWidget::OnSelectionFinished);

  connect(m_Popup->GetInspector(), &QmitkMultiLabelInspector::CurrentSelectionChanged,
    m_Controls->labelSelection, &QmitkMultiLabelSelectionButton::SetSelectedLabels);
}

QmitkMultiLabelSelectionWidget::~QmitkMultiLabelSelectionWidget()
{
}

void QmitkMultiLabelSelectionWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  if (!m_Popup->isVisible())
  {
    this->PositionPopup();
  }
}

void QmitkMultiLabelSelectionWidget::ShowPopup()
{
  if (!m_Popup->isVisible())
  {
    this->PositionPopup();
    m_Popup->show();
  }
}

void QmitkMultiLabelSelectionWidget::HidePopup()
{
  m_Popup->hide();
}

void QmitkMultiLabelSelectionWidget::OnSelectionFinished()
{
  this->HidePopup();
  emit CurrentSelectionChanged(this->GetSelectedLabels());
}

void QmitkMultiLabelSelectionWidget::PositionPopup()
{
  QPoint popupPos = mapToGlobal(QPoint(0, height()));

  // Ensure the popup doesn't go off screen
  QRect screenGeometry = QApplication::screenAt(popupPos)->geometry();
  int popupRight = popupPos.x() + width();
  int screenRight = screenGeometry.right();

  if (popupRight > screenRight) {
    popupPos.setX(screenRight - width());
  }

  m_Popup->setFixedWidth(width());
  m_Popup->move(popupPos);
}

void QmitkMultiLabelSelectionWidget::SetMultiSelectionMode(bool multiMode)
{
  m_Popup->GetInspector()->SetMultiSelectionMode(multiMode);
}

bool QmitkMultiLabelSelectionWidget::GetMultiSelectionMode() const
{
  return m_Popup->GetInspector()->GetMultiSelectionMode();
}

void QmitkMultiLabelSelectionWidget::SetHighlightingActivated(bool visiblityMod)
{
  m_Controls->labelSelection->SetHighlightingActivated(visiblityMod);
}

bool QmitkMultiLabelSelectionWidget::GetHighlightingActivated() const
{
  return m_Controls->labelSelection->GetHighlightingActivated();
}

void QmitkMultiLabelSelectionWidget::SetEmptyInfo(QString info)
{
  m_Controls->labelSelection->SetEmptyInfo(info);
}

QString QmitkMultiLabelSelectionWidget::GetEmptyInfo() const
{
  return m_Controls->labelSelection->GetEmptyInfo();
}

void QmitkMultiLabelSelectionWidget::SetMultiLabelSegmentation(mitk::LabelSetImage* segmentation)
{
  m_Controls->labelSelection->SetMultiLabelSegmentation(segmentation);
  m_Popup->GetInspector()->SetMultiLabelSegmentation(segmentation);
}

mitk::LabelSetImage* QmitkMultiLabelSelectionWidget::GetMultiLabelSegmentation() const
{
  return m_Controls->labelSelection->GetMultiLabelSegmentation();
}

void QmitkMultiLabelSelectionWidget::SetMultiLabelNode(mitk::DataNode* node)
{
  m_Controls->labelSelection->SetMultiLabelNode(node);
  m_Popup->GetInspector()->SetMultiLabelNode(node);
}

mitk::DataNode* QmitkMultiLabelSelectionWidget::GetMultiLabelNode() const
{
  return m_Controls->labelSelection->GetMultiLabelNode();
}

void QmitkMultiLabelSelectionWidget::SetSelectedLabels(const LabelValueVectorType& selectedLabels)
{
  m_Controls->labelSelection->SetSelectedLabels(selectedLabels);
  m_Popup->GetInspector()->SetSelectedLabels(selectedLabels);
}

void QmitkMultiLabelSelectionWidget::SetSelectedLabel(mitk::LabelSetImage::LabelValueType selectedLabel)
{
  this->SetSelectedLabels({ selectedLabel });
}

QmitkMultiLabelSelectionWidget::LabelValueVectorType QmitkMultiLabelSelectionWidget::GetSelectedLabels() const
{
  return m_Popup->GetInspector()->GetSelectedLabels();
}

mitk::Image::Pointer QmitkMultiLabelSelectionWidget::CreateSelectedLabelMask() const
{
  if (this->GetMultiSelectionMode())
  {
    mitkThrow() << "Currently CreateSelectedLabelMask is only implemented for QmitkMultiLabelSelectionWidget::GetMultiSelectionMode()==false.";
  }

  if (nullptr != this->GetMultiLabelSegmentation() && !this->GetSelectedLabels().empty())
    return mitk::CreateLabelMask(this->GetMultiLabelSegmentation(), this->GetSelectedLabels().front());

  return nullptr;
}



QmitkMultiLabelInspectorPopup::QmitkMultiLabelInspectorPopup(QWidget* parent)
  : QWidget(parent)
  , m_Inspector(new QmitkMultiLabelInspector)
{
  // Setup layout
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(1, 1, 1, 1);
  layout->addWidget(m_Inspector.get());

  m_Inspector->SetAllowLabelModification(false);
  m_Inspector->SetAllowLockModification(false);
  m_Inspector->SetAllowVisibilityModification(false);

  // Set size policy
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  // Set popup to be a top-level widget, but not an independent window
  setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

  connect(m_Inspector.get(), &QmitkMultiLabelInspector::CurrentSelectionChanged,
    this, &QmitkMultiLabelInspectorPopup::OnPopupLabelsChanged);

}

QmitkMultiLabelInspectorPopup::~QmitkMultiLabelInspectorPopup() = default;

QmitkMultiLabelInspector* QmitkMultiLabelInspectorPopup::GetInspector() const
{
  return m_Inspector.get();
}

void QmitkMultiLabelInspectorPopup::OnPopupLabelsChanged(const LabelValueVectorType& selectedLabels)
{
  if (!m_Inspector->GetMultiSelectionMode())
  { //if we only do single selection, we can emit finish as soon as the selection is changed.
    emit SelectionFinished();
  }
}

