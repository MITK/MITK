/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTransferFunctionWidget.h"
#include <ui_QmitkTransferFunctionWidget.h>
#include "mitkImageTimeSelector.h"

#include <mitkTransferFunctionProperty.h>

QmitkTransferFunctionWidget::QmitkTransferFunctionWidget(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
  m_Controls = std::make_unique<Ui::QmitkTransferFunctionWidget>();
  m_Controls->setupUi(this);

  // signals and slots connections
  connect(m_Controls->m_XEditScalarOpacity, SIGNAL(textEdited(const QString &)), this, SLOT(SetXValueScalar(const QString &)));
  connect(m_Controls->m_YEditScalarOpacity, SIGNAL(textEdited(const QString &)), this, SLOT(SetYValueScalar(const QString &)));

  connect(m_Controls->m_XEditGradientOpacity, SIGNAL(textEdited(const QString &)), this, SLOT(SetXValueGradient(const QString &)));
  connect(m_Controls->m_YEditGradientOpacity, SIGNAL(textEdited(const QString &)), this, SLOT(SetYValueGradient(const QString &)));

  connect(m_Controls->m_XEditColor, SIGNAL(textEdited(const QString &)), this, SLOT(SetXValueColor(const QString &)));

  m_Controls->m_RangeSlider->setMinimum(-2048);
  m_Controls->m_RangeSlider->setMaximum(2048);
  UpdateStepSize();
  connect(m_Controls->m_RangeSlider, SIGNAL(valuesChanged(double, double)), this, SLOT(OnSpanChanged(double, double)));

  // reset button
  connect(m_Controls->m_RangeSliderReset, SIGNAL(pressed()), this, SLOT(OnResetSlider()));

  m_Controls->m_ScalarOpacityFunctionCanvas->SetQLineEdits(m_Controls->m_XEditScalarOpacity, m_Controls->m_YEditScalarOpacity);
  m_Controls->m_GradientOpacityCanvas->SetQLineEdits(m_Controls->m_XEditGradientOpacity, m_Controls->m_YEditGradientOpacity);
  m_Controls->m_ColorTransferFunctionCanvas->SetQLineEdits(m_Controls->m_XEditColor, nullptr);

  m_Controls->m_ScalarOpacityFunctionCanvas->SetTitle("Grayvalue -> Opacity");
  m_Controls->m_GradientOpacityCanvas->SetTitle("Grayvalue/Gradient -> Opacity");
  m_Controls->m_ColorTransferFunctionCanvas->SetTitle("Grayvalue -> Color");
}

QmitkTransferFunctionWidget::~QmitkTransferFunctionWidget()
{
}

void QmitkTransferFunctionWidget::SetScalarLabel(const QString &scalarLabel)
{
  m_Controls->m_textLabelX->setText(scalarLabel);
  m_Controls->m_textLabelX_2->setText(scalarLabel);
  m_Controls->m_textLabelX_3->setText(scalarLabel);

  m_Controls->m_ScalarOpacityFunctionCanvas->SetTitle(scalarLabel + " -> Opacity");
  m_Controls->m_GradientOpacityCanvas->SetTitle(scalarLabel + "/Gradient -> Opacity");
  m_Controls->m_ColorTransferFunctionCanvas->SetTitle(scalarLabel + " -> Color");
}

void QmitkTransferFunctionWidget::ShowScalarOpacityFunction(bool show)
{
  m_Controls->m_ScalarOpacityWidget->setVisible(show);
}

void QmitkTransferFunctionWidget::ShowColorFunction(bool show)
{
  m_Controls->m_ColorWidget->setVisible(show);
}

void QmitkTransferFunctionWidget::ShowGradientOpacityFunction(bool show)
{
  m_Controls->m_GradientOpacityWidget->setVisible(show);
}

void QmitkTransferFunctionWidget::SetScalarOpacityFunctionEnabled(bool enable)
{
  m_Controls->m_ScalarOpacityWidget->setEnabled(enable);
}

void QmitkTransferFunctionWidget::SetColorFunctionEnabled(bool enable)
{
  m_Controls->m_ColorWidget->setEnabled(enable);
}

void QmitkTransferFunctionWidget::SetGradientOpacityFunctionEnabled(bool enable)
{
  m_Controls->m_GradientOpacityWidget->setEnabled(enable);
}

void QmitkTransferFunctionWidget::SetDataNode(mitk::DataNode *node, mitk::TimeStepType timestep, const mitk::BaseRenderer *renderer)
{
  if (node)
  {
    tfpToChange = dynamic_cast<mitk::TransferFunctionProperty *>(node->GetProperty("TransferFunction", renderer));
    if (!tfpToChange)
    {
      if (!dynamic_cast<mitk::Image *>(node->GetData()))
      {
        MITK_WARN << "QmitkTransferFunctionWidget::SetDataNode called with non-image node";
        goto turnOff;
      }

      node->SetProperty("TransferFunction", tfpToChange = mitk::TransferFunctionProperty::New());
    }

    mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

    if (mitk::Image *data = dynamic_cast<mitk::Image *>(node->GetData()))
    {
      mitk::SimpleHistogram *h = nullptr;
      if (data->GetTimeSteps() > 1)
      {
        if (!data->GetTimeGeometry()->IsValidTimeStep(timestep))
        {
          return;
        }
        mitk::ImageTimeSelector::Pointer timeselector = mitk::ImageTimeSelector::New();
        timeselector->SetInput(data);
        timeselector->SetTimeNr(timestep);
        timeselector->UpdateLargestPossibleRegion();
        auto inputImage = timeselector->GetOutput();
        h = histogramCache[inputImage];
      }
      else
      {
        h = histogramCache[data];
      }

      auto rangeSliderMin = h->GetMin();
      auto rangeSliderMax = h->GetMax();
      UpdateStepSize();

      m_Controls->m_RangeSlider->blockSignals(true);
      m_Controls->m_RangeSlider->setMinimum(rangeSliderMin);
      m_Controls->m_RangeSlider->setMaximum(rangeSliderMax);
      m_Controls->m_RangeSlider->setMinimumValue(rangeSliderMin);
      m_Controls->m_RangeSlider->setMaximumValue(rangeSliderMax);
      m_Controls->m_RangeSlider->blockSignals(false);

      m_Controls->m_ScalarOpacityFunctionCanvas->SetHistogram(h);
      m_Controls->m_GradientOpacityCanvas->SetHistogram(h);
      m_Controls->m_ColorTransferFunctionCanvas->SetHistogram(h);
    }

    OnUpdateCanvas();

    return;
  }

turnOff:

  m_Controls->m_ScalarOpacityFunctionCanvas->setEnabled(false);
  m_Controls->m_ScalarOpacityFunctionCanvas->SetHistogram(nullptr);
  m_Controls->m_GradientOpacityCanvas->setEnabled(false);
  m_Controls->m_GradientOpacityCanvas->SetHistogram(nullptr);
  m_Controls->m_ColorTransferFunctionCanvas->setEnabled(false);
  m_Controls->m_ColorTransferFunctionCanvas->SetHistogram(nullptr);
  tfpToChange = nullptr;
}

void QmitkTransferFunctionWidget::OnUpdateCanvas()
{
  if (tfpToChange.IsNull())
    return;

  mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

  if (tf.IsNull())
    return;

  m_Controls->m_ScalarOpacityFunctionCanvas->SetPiecewiseFunction(tf->GetScalarOpacityFunction());
  m_Controls->m_GradientOpacityCanvas->SetPiecewiseFunction(tf->GetGradientOpacityFunction());
  m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction(tf->GetColorTransferFunction());

  UpdateRanges();

  m_Controls->m_ScalarOpacityFunctionCanvas->update();
  m_Controls->m_GradientOpacityCanvas->update();
  m_Controls->m_ColorTransferFunctionCanvas->update();
}

void QmitkTransferFunctionWidget::SetXValueScalar(const QString text)
{
  if (!text.endsWith("."))
  {
    m_Controls->m_ScalarOpacityFunctionCanvas->SetX(text.toFloat());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkTransferFunctionWidget::SetYValueScalar(const QString text)
{
  if (!text.endsWith("."))
  {
    m_Controls->m_ScalarOpacityFunctionCanvas->SetY(text.toFloat());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkTransferFunctionWidget::SetXValueGradient(const QString text)
{
  if (!text.endsWith("."))
  {
    m_Controls->m_GradientOpacityCanvas->SetX(text.toFloat());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkTransferFunctionWidget::SetYValueGradient(const QString text)
{
  if (!text.endsWith("."))
  {
    m_Controls->m_GradientOpacityCanvas->SetY(text.toFloat());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkTransferFunctionWidget::SetXValueColor(const QString text)
{
  if (!text.endsWith("."))
  {
    m_Controls->m_ColorTransferFunctionCanvas->SetX(text.toFloat());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkTransferFunctionWidget::UpdateRanges()
{
  double lower = m_Controls->m_RangeSlider->minimumValue();
  double upper = m_Controls->m_RangeSlider->maximumValue();

  m_Controls->m_ScalarOpacityFunctionCanvas->SetMin(lower);
  m_Controls->m_ScalarOpacityFunctionCanvas->SetMax(upper);

  m_Controls->m_GradientOpacityCanvas->SetMin(lower);
  m_Controls->m_GradientOpacityCanvas->SetMax(upper);

  m_Controls->m_ColorTransferFunctionCanvas->SetMin(lower);
  m_Controls->m_ColorTransferFunctionCanvas->SetMax(upper);
}

void QmitkTransferFunctionWidget::UpdateStepSize()
{
  double step = (m_Controls->m_RangeSlider->maximum() - m_Controls->m_RangeSlider->minimum()) / 1000.;
  m_Controls->m_RangeSlider->setSingleStep(step);
}

void QmitkTransferFunctionWidget::OnSpanChanged(double, double)
{
  UpdateRanges();

  m_Controls->m_GradientOpacityCanvas->update();
  m_Controls->m_ColorTransferFunctionCanvas->update();
  m_Controls->m_ScalarOpacityFunctionCanvas->update();
}

void QmitkTransferFunctionWidget::OnResetSlider()
{
  m_Controls->m_RangeSlider->blockSignals(true);
  m_Controls->m_RangeSlider->setMaximumValue(m_Controls->m_RangeSlider->maximum());
  m_Controls->m_RangeSlider->setMinimumValue(m_Controls->m_RangeSlider->minimum());
  m_Controls->m_RangeSlider->blockSignals(false);

  UpdateRanges();
  m_Controls->m_GradientOpacityCanvas->update();
  m_Controls->m_ColorTransferFunctionCanvas->update();
  m_Controls->m_ScalarOpacityFunctionCanvas->update();
}
