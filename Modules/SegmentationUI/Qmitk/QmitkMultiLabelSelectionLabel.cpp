/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelSelectionLabel.h>

// mitk
#include <mitkRenderingManager.h>
#include <mitkLabelSetImageHelper.h>

// Qmitk
#include <QmitkStyleManager.h>


#include <ui_QmitkMultiLabelSelectionLabel.h>


QmitkMultiLabelSelectionLabel::QmitkMultiLabelSelectionLabel(QWidget* parent/* = nullptr*/)
  : QWidget(parent),
  m_Controls(new Ui::QmitkMultiLabelSelectionLabel),
  m_SegmentationNodeDataMTime(0)
{
  m_Controls->setupUi(this);
  m_LabelHighlightGuard.SetHighlightInvisibleLabels(true);
  m_EmptyInfo = QStringLiteral("<font class=\"warning\">Select label(s) to proceed.</font>");
}

QmitkMultiLabelSelectionLabel::~QmitkMultiLabelSelectionLabel()
{
}

void QmitkMultiLabelSelectionLabel::Initialize()
{
  m_LastValidSelectedLabels = {};
  auto styleSheet = qApp->styleSheet();
  m_Controls->labelText->document()->setDefaultStyleSheet(styleSheet);

  this->UpdateWidget();
}

void QmitkMultiLabelSelectionLabel::SetHighlightingActivated(bool visibilityMod)
{
  m_HighlightingActivated = visibilityMod;
}

bool QmitkMultiLabelSelectionLabel::GetHighlightingActivated() const
{
  return m_HighlightingActivated;
}

void QmitkMultiLabelSelectionLabel::SetMultiLabelSegmentation(mitk::LabelSetImage* segmentation)
{
  if (segmentation != m_Segmentation)
  {
    m_Segmentation = segmentation;
    this->Initialize();
  }
}

mitk::LabelSetImage* QmitkMultiLabelSelectionLabel::GetMultiLabelSegmentation() const
{
  return m_Segmentation;
}

void QmitkMultiLabelSelectionLabel::SetMultiLabelNode(mitk::DataNode* node)
{
  if (node != this->m_SegmentationNode.GetPointer())
  {
    m_SegmentationObserver.Reset();
    m_SegmentationNode = node;
    m_LabelHighlightGuard.SetSegmentationNode(m_SegmentationNode);
    m_SegmentationNodeDataMTime = 0;

    if (m_SegmentationNode.IsNotNull())
    {
      auto& widget = *this;
      auto checkAndSetSeg = [&widget, node](const itk::EventObject&)
        {
          if (widget.m_SegmentationNodeDataMTime < node->GetDataReferenceChangedTime())
          {
            auto newSeg = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
            if (nullptr == newSeg) mitkThrow() << "Invalid usage. Node set does not contain a segmentation. Invalid node name: " << node->GetName();

            widget.m_SegmentationNodeDataMTime = node->GetDataReferenceChangedTime();
            widget.SetMultiLabelSegmentation(newSeg);
          }
        };

      m_SegmentationObserver.Reset(node, itk::ModifiedEvent(), checkAndSetSeg);
      checkAndSetSeg(itk::ModifiedEvent());
    }
    else
    {
      this->SetMultiLabelSegmentation(nullptr);
    }
  }
}

mitk::DataNode* QmitkMultiLabelSelectionLabel::GetMultiLabelNode() const
{
  return m_SegmentationNode;
}

void QmitkMultiLabelSelectionLabel::SetEmptyInfo(QString info)
{
  if (info == m_EmptyInfo)
    return;

  m_EmptyInfo = info;
  this->UpdateWidget();
}

QString QmitkMultiLabelSelectionLabel::GetEmptyInfo() const
{
  return m_EmptyInfo;
}

void QmitkMultiLabelSelectionLabel::SetSelectedLabels(const LabelValueVectorType& selectedLabels)
{
  if (mitk::Equal(this->GetSelectedLabels(), selectedLabels))
  {
    return;
  }

  m_LastValidSelectedLabels = selectedLabels;
  this->UpdateWidget();
}

void QmitkMultiLabelSelectionLabel::UpdateWidget()
{
  if (m_LastValidSelectedLabels.empty())
  {
    m_Controls->labelText->setHtml(m_EmptyInfo);
  }
  else
  {
    std::stringstream stream;
    for (const auto& labelValue : m_LastValidSelectedLabels)
    {
      mitk::Label::ConstPointer label = this->GetMultiLabelSegmentation()->GetLabel(labelValue);
      if (stream.rdbuf()->in_avail() != 0) stream << "; ";
      if (label.IsNotNull())
        stream << mitk::LabelSetImageHelper::CreateHTMLLabelName(label, this->GetMultiLabelSegmentation());
      else
        stream << "Unknown label ID " << labelValue;
    }

    m_Controls->labelText->setText(QString::fromStdString(stream.str()));
  }
}

void QmitkMultiLabelSelectionLabel::SetSelectedLabel(mitk::LabelSetImage::LabelValueType selectedLabel)
{
  this->SetSelectedLabels({ selectedLabel });
}

QmitkMultiLabelSelectionLabel::LabelValueVectorType QmitkMultiLabelSelectionLabel::GetSelectedLabels() const
{
  return m_LastValidSelectedLabels;
}

void QmitkMultiLabelSelectionLabel::enterEvent(QEnterEvent* /*event*/)
{
  if (m_SegmentationNode.IsNotNull() && m_HighlightingActivated)
  {
    m_LabelHighlightGuard.SetHighlightedLabels(m_LastValidSelectedLabels);
  }
}

void QmitkMultiLabelSelectionLabel::leaveEvent(QEvent* /*event*/ )
{
  if (m_SegmentationNode.IsNotNull())
  {
    m_LabelHighlightGuard.SetHighlightedLabels({});
  }
}
