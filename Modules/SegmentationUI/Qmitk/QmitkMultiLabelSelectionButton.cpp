/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiLabelSelectionButton.h"

#include <mitkRenderingManager.h>
#include <mitkLabelSetImageHelper.h>

// Qmitk
#include <QmitkStyleManager.h>

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QTextDocument>


QmitkMultiLabelSelectionButton::QmitkMultiLabelSelectionButton(QWidget *parent)
  : QPushButton(parent),
  m_SegmentationNodeDataMTime(0)
{
  m_LabelHighlightGuard.SetHighlightInvisibleLabels(true);
  m_EmptyInfo = QStringLiteral("<font class=\"warning\">Select label(s) to proceed.</font>");
}

QmitkMultiLabelSelectionButton::~QmitkMultiLabelSelectionButton()
{
}

void QmitkMultiLabelSelectionButton::SetHighlightingActivated(bool visibilityMod)
{
  m_HighlightingActivated = visibilityMod;
}

bool QmitkMultiLabelSelectionButton::GetHighlightingActivated() const
{
  return m_HighlightingActivated;
}

void QmitkMultiLabelSelectionButton::SetMultiLabelSegmentation(mitk::MultiLabelSegmentation* segmentation)
{
  if (segmentation != m_Segmentation)
  {
    m_Segmentation = segmentation;
    this->update();
  }
}

mitk::MultiLabelSegmentation* QmitkMultiLabelSelectionButton::GetMultiLabelSegmentation() const
{
  return m_Segmentation;
}

void QmitkMultiLabelSelectionButton::SetMultiLabelNode(mitk::DataNode* node)
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
            auto newSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(node->GetData());
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

mitk::DataNode* QmitkMultiLabelSelectionButton::GetMultiLabelNode() const
{
  return m_SegmentationNode;
}

void QmitkMultiLabelSelectionButton::SetEmptyInfo(QString info)
{
  if (info == m_EmptyInfo)
    return;

  m_EmptyInfo = info;
  this->update();
}

QString QmitkMultiLabelSelectionButton::GetEmptyInfo() const
{
  return m_EmptyInfo;
}

void QmitkMultiLabelSelectionButton::SetSelectedLabels(const LabelValueVectorType& selectedLabels)
{
  if (mitk::Equal(this->GetSelectedLabels(), selectedLabels))
  {
    return;
  }

  m_LastValidSelectedLabels = selectedLabels;
  this->update();
}

void QmitkMultiLabelSelectionButton::paintEvent(QPaintEvent *p)
{
  QPushButton::paintEvent(p);

  auto styleSheet = qApp->styleSheet();

  QPainter painter(this);
  QTextDocument td(this);
  td.setDefaultStyleSheet(styleSheet);

  if (m_Segmentation.IsNotNull() && !m_LastValidSelectedLabels.empty())
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

    if (this->isEnabled())
    {
      td.setHtml(QString("<font class=\"normal\">") + QString::fromStdString(stream.str()) + QString("</font>"));
    }
    else
    {
      td.setHtml(QString("<font class=\"disabled\">") + QString::fromStdString(stream.str()) + QString("</font>"));
    }
  }
  else
  {
    if (this->isEnabled())
    {
      td.setHtml(QString("<font class=\"warning\">") + m_EmptyInfo + QString("</font>"));
    }
    else
    {
      td.setHtml(QString("<font class=\"disabled\">") + m_EmptyInfo + QString("</font>"));
    }
  }

  auto widgetSize = this->size();
  auto textSize = td.size();
  QPoint origin = QPoint(5, -5 + (widgetSize.height() - textSize.height()) / 2.);

  painter.translate(origin);
  td.drawContents(&painter);
}

void QmitkMultiLabelSelectionButton::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::EnabledChange)
  {
    this->update();
  }
}

void QmitkMultiLabelSelectionButton::SetSelectedLabel(mitk::MultiLabelSegmentation::LabelValueType selectedLabel)
{
  this->SetSelectedLabels({ selectedLabel });
}

QmitkMultiLabelSelectionButton::LabelValueVectorType QmitkMultiLabelSelectionButton::GetSelectedLabels() const
{
  return m_LastValidSelectedLabels;
}

void QmitkMultiLabelSelectionButton::enterEvent(QEnterEvent* /*event*/)
{
  if (m_SegmentationNode.IsNotNull() && m_HighlightingActivated)
  {
    m_LabelHighlightGuard.SetHighlightedLabels(m_LastValidSelectedLabels);
  }
}

void QmitkMultiLabelSelectionButton::leaveEvent(QEvent* /*event*/)
{
  if (m_SegmentationNode.IsNotNull())
  {
    m_LabelHighlightGuard.SetHighlightedLabels({});
  }
}
