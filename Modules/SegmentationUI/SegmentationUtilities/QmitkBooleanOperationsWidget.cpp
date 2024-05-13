/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkBooleanOperationsWidget.h"
#include <ui_QmitkBooleanOperationsWidgetControls.h>

#include <mitkDataStorage.h>
#include <mitkException.h>
#include <mitkRenderingManager.h>
#include <mitkMultiLabelPredicateHelper.h>
#include <mitkBooleanOperation.h>
#include <mitkProgressBar.h>
#include <mitkLabelSetImageHelper.h>


QmitkBooleanOperationsWidget::QmitkBooleanOperationsWidget(mitk::DataStorage* dataStorage, QWidget* parent)
  : QWidget(parent)
{
  m_Controls = new Ui::QmitkBooleanOperationsWidgetControls;
  m_Controls->setupUi(this);

  m_Controls->label1st->setText("<img width=16 height=16 src=\":/Qmitk/BooleanLabelA_32x32.png\"/>");
  m_Controls->label1st->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  m_Controls->label2nd->setText("<img width=16 height=16 src=\":/Qmitk/BooleanLabelB_32x32.png\"/>");
  m_Controls->label2nd->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

  m_Controls->segNodeSelector->SetDataStorage(dataStorage);
  m_Controls->segNodeSelector->SetNodePredicate(mitk::GetMultiLabelSegmentationPredicate());
  m_Controls->segNodeSelector->SetSelectionIsOptional(false);
  m_Controls->segNodeSelector->SetInvalidInfo(QStringLiteral("Please select segmentation for boolean operations."));
  m_Controls->segNodeSelector->SetPopUpTitel(QStringLiteral("Select segmentation"));
  m_Controls->segNodeSelector->SetPopUpHint(QStringLiteral("Select the segmentation that should be used as source for boolean operations."));

  m_Controls->labelInspector->SetMultiSelectionMode(true);

  connect(m_Controls->segNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkBooleanOperationsWidget::OnSegSelectionChanged);

  connect(m_Controls->labelInspector, &QmitkMultiLabelInspector::CurrentSelectionChanged,
    this, &QmitkBooleanOperationsWidget::OnLabelSelectionChanged);

  connect(m_Controls->differenceButton, &QToolButton::clicked, this, &QmitkBooleanOperationsWidget::OnDifferenceButtonClicked);
  connect(m_Controls->intersectionButton, &QToolButton::clicked, this, &QmitkBooleanOperationsWidget::OnIntersectionButtonClicked);
  connect(m_Controls->unionButton, &QToolButton::clicked, this, &QmitkBooleanOperationsWidget::OnUnionButtonClicked);

  m_Controls->segNodeSelector->SetAutoSelectNewNodes(true);
  this->ConfigureWidgets();
}

QmitkBooleanOperationsWidget::~QmitkBooleanOperationsWidget()
{
  m_Controls->labelInspector->SetMultiLabelNode(nullptr);
}

void QmitkBooleanOperationsWidget::OnSegSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/)
{
  auto node = m_Controls->segNodeSelector->GetSelectedNode();
  m_Controls->labelInspector->SetMultiLabelNode(node);

  this->ConfigureWidgets();
}

void QmitkBooleanOperationsWidget::OnLabelSelectionChanged(mitk::LabelSetImage::LabelValueVectorType /*labels*/)
{
  this->ConfigureWidgets();
}

namespace
{
  std::string GenerateLabelHTML(const mitk::Label* label)
  {
    std::stringstream stream;
    auto color = label->GetColor();
    stream << "<span style='color: #" << std::hex << std::setfill('0')
      << std::setw(2) << static_cast<int>(color.GetRed()*255)
      << std::setw(2) << static_cast<int>(color.GetGreen()*255)
      << std::setw(2) << static_cast<int>(color.GetBlue()*255)
      << "; font-size: 20px '>&#x25A0;</span>" << std::dec;

    stream << "<font class=\"normal\"> " << label->GetName()<< "</font>";
    return stream.str();
  }
}
void QmitkBooleanOperationsWidget::ConfigureWidgets()
{
  auto selectedLabelValues = m_Controls->labelInspector->GetSelectedLabels();
  auto seg = m_Controls->labelInspector->GetMultiLabelSegmentation();

  auto styleSheet = qApp->styleSheet();

  m_Controls->line1stLabel->document()->setDefaultStyleSheet(styleSheet);
  m_Controls->lineOtherLabels->document()->setDefaultStyleSheet(styleSheet);

  if (selectedLabelValues.empty())
  {
    m_Controls->line1stLabel->setHtml(QStringLiteral("<font class=\"warning\">Select 1st label to proceed.</font>"));
  }
  else
  {
    mitk::Label::ConstPointer label = seg->GetLabel(selectedLabelValues.front());
    if (label.IsNotNull())
      m_Controls->line1stLabel->setText(QString::fromStdString(GenerateLabelHTML(label)));
    else
      m_Controls->line1stLabel->setText(QString("Unknown label ID ")+QString::number(selectedLabelValues.front()));
  }

  if (selectedLabelValues.size() < 2)
  {
    m_Controls->lineOtherLabels->setHtml(QStringLiteral("<font class=\"warning\">Select secondary label(s) to proceed.</font>"));
  }
  else
  {
    std::stringstream stream;
    for (const auto& labelValue : selectedLabelValues)
    {
      mitk::Label::ConstPointer label = seg->GetLabel(labelValue);
      if (stream.rdbuf()->in_avail() != 0) stream << "; ";
      if (label.IsNotNull())
        stream << GenerateLabelHTML(label);
      else
        stream << "Unknown label ID " << labelValue;
    }

    m_Controls->lineOtherLabels->setText(QString::fromStdString(stream.str()));
  }

  m_Controls->differenceButton->setEnabled(selectedLabelValues.size()>1);
  m_Controls->intersectionButton->setEnabled(selectedLabelValues.size() > 1);
  m_Controls->unionButton->setEnabled(selectedLabelValues.size() > 1);
}

void QmitkBooleanOperationsWidget::OnDifferenceButtonClicked()
{
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  mitk::ProgressBar::GetInstance()->Reset();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(110);
  unsigned int currentProgress = 0;

  auto progressCallback = [&currentProgress](float filterProgress)
    {
      auto delta = (filterProgress * 100) - currentProgress;
      if (delta > 0)
      {
        currentProgress += delta;
        mitk::ProgressBar::GetInstance()->Progress(delta);
      }
    };

  auto selectedLabelValues = m_Controls->labelInspector->GetSelectedLabels();
  auto minuend = selectedLabelValues.front();
  auto subtrahends = mitk::LabelSetImage::LabelValueVectorType(selectedLabelValues.begin() + 1, selectedLabelValues.end());

  auto seg = m_Controls->labelInspector->GetMultiLabelSegmentation();

  auto resultMask = mitk::BooleanOperation::GenerateDifference(seg, minuend, subtrahends, progressCallback);

  std::stringstream name;
  name << "Difference " << seg->GetLabel(minuend)->GetName() << " -";
  for (auto label : subtrahends)
  {
    name << " " << seg->GetLabel(label)->GetName();
  }

  this->SaveResultLabelMask(resultMask, name.str());

  mitk::ProgressBar::GetInstance()->Reset();
  QApplication::restoreOverrideCursor();
}

void QmitkBooleanOperationsWidget::OnIntersectionButtonClicked()
{
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  mitk::ProgressBar::GetInstance()->Reset();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(110);
  unsigned int currentProgress = 0;

  auto progressCallback = [&currentProgress](float filterProgress)
    {
      auto delta = (filterProgress * 100) - currentProgress;
      if (delta > 0)
      {
        currentProgress += delta;
        mitk::ProgressBar::GetInstance()->Progress(delta);
      }
    };

  auto selectedLabelValues = m_Controls->labelInspector->GetSelectedLabels();

  auto seg = m_Controls->labelInspector->GetMultiLabelSegmentation();

  auto resultMask = mitk::BooleanOperation::GenerateIntersection(seg, selectedLabelValues, progressCallback);

  std::stringstream name;
  name << "Intersection";
  for (auto label : selectedLabelValues)
  {
    name << " " << seg->GetLabel(label)->GetName();
  }
  this->SaveResultLabelMask(resultMask, name.str());

  mitk::ProgressBar::GetInstance()->Reset();
  QApplication::restoreOverrideCursor();
}

void QmitkBooleanOperationsWidget::OnUnionButtonClicked()
{
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  mitk::ProgressBar::GetInstance()->Reset();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(110);
  unsigned int currentProgress = 0;

  auto progressCallback = [&currentProgress](float filterProgress)
    {
      auto delta = (filterProgress * 100) - currentProgress;
      if (delta > 0)
      {
        currentProgress += delta;
        mitk::ProgressBar::GetInstance()->Progress(delta);
      }
    };

  auto selectedLabelValues = m_Controls->labelInspector->GetSelectedLabels();

  auto seg = m_Controls->labelInspector->GetMultiLabelSegmentation();

  auto resultMask = mitk::BooleanOperation::GenerateUnion(seg, selectedLabelValues, progressCallback);

  std::stringstream name;
  name << "Union";
  for (auto label : selectedLabelValues)
  {
    name << " " << seg->GetLabel(label)->GetName();
  }

  this->SaveResultLabelMask(resultMask, name.str());

  mitk::ProgressBar::GetInstance()->Reset();
  QApplication::restoreOverrideCursor();
}

void QmitkBooleanOperationsWidget::SaveResultLabelMask(const mitk::Image* resultMask, const std::string& labelName) const
{
  auto seg = m_Controls->labelInspector->GetMultiLabelSegmentation();
  if (seg == nullptr) mitkThrow() << "Widget is in invalid state. Processing was triggered with no segmentation selected.";

  auto labels = m_Controls->labelInspector->GetSelectedLabels();
  if (labels.empty()) mitkThrow() << "Widget is in invalid state. Processing was triggered with no label selected.";

  auto groupID = seg->AddLayer();
  auto newLabel = mitk::LabelSetImageHelper::CreateNewLabel(seg, labelName, true);
  seg->AddLabelWithContent(newLabel, resultMask, groupID, 1);

  m_Controls->labelInspector->GetMultiLabelSegmentation()->Modified();
  m_Controls->labelInspector->GetMultiLabelNode()->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
