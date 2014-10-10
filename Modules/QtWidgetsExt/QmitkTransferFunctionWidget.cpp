/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkTransferFunctionWidget.h"

#include <mitkTransferFunctionProperty.h>

QmitkTransferFunctionWidget::QmitkTransferFunctionWidget(QWidget* parent,
                                                         Qt::WindowFlags f) :
  QWidget(parent, f)
{
  this->setupUi(this);



  // signals and slots connections
  connect(m_XEditScalarOpacity, SIGNAL(textEdited ( const QString & )), this, SLOT(SetXValueScalar( const QString & )));
  connect(m_YEditScalarOpacity, SIGNAL(textEdited ( const QString & )), this, SLOT(SetYValueScalar( const QString & )));

  connect(m_XEditGradientOpacity, SIGNAL(textEdited ( const QString & )), this, SLOT(SetXValueGradient( const QString & )));
  connect(m_YEditGradientOpacity, SIGNAL(textEdited ( const QString & )), this, SLOT(SetYValueGradient( const QString & )));

  connect(m_XEditColor, SIGNAL( textEdited ( const QString & ) ), this, SLOT(SetXValueColor( const QString & )));

  m_RangeSlider->setMinimum(-2048);
  m_RangeSlider->setMaximum(2048);
  connect(m_RangeSlider, SIGNAL(valuesChanged(int,int)),this, SLOT(OnSpanChanged(int,int)));

  //reset button
  connect(m_RangeSliderReset, SIGNAL(pressed()), this, SLOT(OnResetSlider()));

  m_ScalarOpacityFunctionCanvas->SetQLineEdits(m_XEditScalarOpacity, m_YEditScalarOpacity);
  m_GradientOpacityCanvas->SetQLineEdits(m_XEditGradientOpacity, m_YEditGradientOpacity);
  m_ColorTransferFunctionCanvas->SetQLineEdits(m_XEditColor, 0);

  m_ScalarOpacityFunctionCanvas->SetTitle("Grayvalue -> Opacity");
  m_GradientOpacityCanvas->SetTitle("Grayvalue/Gradient -> Opacity");
  m_ColorTransferFunctionCanvas->SetTitle("Grayvalue -> Color");

}

QmitkTransferFunctionWidget::~QmitkTransferFunctionWidget()
{
}

void QmitkTransferFunctionWidget::SetScalarLabel(const QString& scalarLabel)
{
  m_textLabelX->setText(scalarLabel);
  m_textLabelX_2->setText(scalarLabel);
  m_textLabelX_3->setText(scalarLabel);

  m_ScalarOpacityFunctionCanvas->SetTitle(scalarLabel + " -> Opacity");
  m_GradientOpacityCanvas->SetTitle(scalarLabel + "/Gradient -> Opacity");
  m_ColorTransferFunctionCanvas->SetTitle(scalarLabel + " -> Color");
}

void QmitkTransferFunctionWidget::ShowScalarOpacityFunction(bool show)
{
  m_ScalarOpacityWidget->setVisible(show);
}

void QmitkTransferFunctionWidget::ShowColorFunction(bool show)
{
  m_ColorWidget->setVisible(show);
}

void QmitkTransferFunctionWidget::ShowGradientOpacityFunction(bool show)
{
  m_GradientOpacityWidget->setVisible(show);
}

void QmitkTransferFunctionWidget::SetScalarOpacityFunctionEnabled(bool enable)
{
  m_ScalarOpacityWidget->setEnabled(enable);
}

void QmitkTransferFunctionWidget::SetColorFunctionEnabled(bool enable)
{
  m_ColorWidget->setEnabled(enable);
}

void QmitkTransferFunctionWidget::SetGradientOpacityFunctionEnabled(bool enable)
{
  m_GradientOpacityWidget->setEnabled(enable);
}

void QmitkTransferFunctionWidget::SetDataNode(mitk::DataNode* node, const mitk::BaseRenderer* renderer)
{
  if (node)
  {
    tfpToChange = dynamic_cast<mitk::TransferFunctionProperty*>(node->GetProperty("TransferFunction", renderer));

    if(!tfpToChange)
    {
      if (! dynamic_cast<mitk::Image*>(node->GetData()))
      {
        MITK_WARN << "QmitkTransferFunctionWidget::SetDataNode called with non-image node";
        goto turnOff;
      }

      node->SetProperty("TransferFunction", tfpToChange = mitk::TransferFunctionProperty::New() );
    }

    mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

    if( mitk::BaseData* data = node->GetData() )
    {
      mitk::SimpleHistogram *h = histogramCache[data];

      m_RangeSliderMin= h->GetMin();
      m_RangeSliderMax= h->GetMax();

      m_RangeSlider->blockSignals(true);
      m_RangeSlider->setMinimum(m_RangeSliderMin);
      m_RangeSlider->setMaximum(m_RangeSliderMax);
      m_RangeSlider->setMinimumValue(m_RangeSliderMin);
      m_RangeSlider->setMaximumValue(m_RangeSliderMax);
      m_RangeSlider->blockSignals(false);

      m_ScalarOpacityFunctionCanvas->SetHistogram( h );
      m_GradientOpacityCanvas->SetHistogram( h );
      m_ColorTransferFunctionCanvas->SetHistogram( h );
    }

    OnUpdateCanvas();

    return;
  }

turnOff:

  m_ScalarOpacityFunctionCanvas->setEnabled(false);
  m_ScalarOpacityFunctionCanvas->SetHistogram(0);
  m_GradientOpacityCanvas->setEnabled(false);
  m_GradientOpacityCanvas->SetHistogram(0);
  m_ColorTransferFunctionCanvas->setEnabled(false);
  m_ColorTransferFunctionCanvas->SetHistogram(0);
  tfpToChange = 0;
}

void QmitkTransferFunctionWidget::OnUpdateCanvas()
{

  if(tfpToChange.IsNull())
    return;

  mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

  if(tf.IsNull())
    return;

  m_ScalarOpacityFunctionCanvas->SetPiecewiseFunction( tf->GetScalarOpacityFunction() );
  m_GradientOpacityCanvas->SetPiecewiseFunction( tf->GetGradientOpacityFunction() );
  m_ColorTransferFunctionCanvas->SetColorTransferFunction( tf->GetColorTransferFunction() );

  UpdateRanges();

  m_ScalarOpacityFunctionCanvas->update();
  m_GradientOpacityCanvas->update();
  m_ColorTransferFunctionCanvas->update();
}

void QmitkTransferFunctionWidget::SetXValueScalar( const QString text )
{
  if ( !text.endsWith( "." ))
  {
    m_ScalarOpacityFunctionCanvas->SetX(text.toFloat());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkTransferFunctionWidget::SetYValueScalar( const QString text )
{
  if ( !text.endsWith( "." ))
  {
    m_ScalarOpacityFunctionCanvas->SetY(text.toFloat());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkTransferFunctionWidget::SetXValueGradient( const QString text )
{
  if ( !text.endsWith( "." ))
  {
    m_GradientOpacityCanvas->SetX(text.toFloat());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkTransferFunctionWidget::SetYValueGradient( const QString text )
{
  if ( !text.endsWith( "." ))
  {
    m_GradientOpacityCanvas->SetY(text.toFloat());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkTransferFunctionWidget::SetXValueColor( const QString text )
{
  if ( !text.endsWith( "." ))
  {
    m_ColorTransferFunctionCanvas->SetX(text.toFloat());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void QmitkTransferFunctionWidget::UpdateRanges()
{
  int lower =  m_RangeSlider->minimumValue();
  int upper =  m_RangeSlider->maximumValue();

  m_ScalarOpacityFunctionCanvas->SetMin(lower);
  m_ScalarOpacityFunctionCanvas->SetMax(upper);

  m_GradientOpacityCanvas->SetMin(lower);
  m_GradientOpacityCanvas->SetMax(upper);

  m_ColorTransferFunctionCanvas->SetMin(lower);
  m_ColorTransferFunctionCanvas->SetMax(upper);
}


void QmitkTransferFunctionWidget::OnSpanChanged(int, int)
{
  UpdateRanges();

  m_GradientOpacityCanvas->update();
  m_ColorTransferFunctionCanvas->update();
  m_ScalarOpacityFunctionCanvas->update();
}

void QmitkTransferFunctionWidget::OnResetSlider()
{
  m_RangeSlider->blockSignals(true);
  m_RangeSlider->setMaximumValue(m_RangeSliderMax);
  m_RangeSlider->setMinimumValue(m_RangeSliderMin);
  m_RangeSlider->blockSignals(false);

  UpdateRanges();
  m_GradientOpacityCanvas->update();
  m_ColorTransferFunctionCanvas->update();
  m_ScalarOpacityFunctionCanvas->update();
}

