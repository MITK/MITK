/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkTransferFunctionWidget.h"

#include <mitkTransferFunctionProperty.h>
#include <QPlastiqueStyle>

QmitkTransferFunctionWidget::QmitkTransferFunctionWidget(QWidget* parent,
    Qt::WindowFlags f) :
  QWidget(parent, f)
{
  this->setupUi(this);


                                                
  // signals and slots connections
  connect(m_XEditScalarOpacity, SIGNAL(returnPressed()), this, SLOT(SetXValueScalar()));
  connect(m_YEditScalarOpacity, SIGNAL(returnPressed()), this, SLOT(SetYValueScalar()));

  connect(m_XEditGradientOpacity, SIGNAL(returnPressed()), this, SLOT(SetXValueGradient()));
  connect(m_YEditGradientOpacity, SIGNAL(returnPressed()), this, SLOT(SetYValueGradient()));

  connect(m_XEditColor, SIGNAL(returnPressed()), this, SLOT(SetXValueColor()));
  
  QPlastiqueStyle *sliderStyle = new QPlastiqueStyle();

  m_RangeSlider->setMaximum(2048);
  m_RangeSlider->setMinimum(-2048);
  m_RangeSlider->setHandleMovementMode(QxtSpanSlider::NoOverlapping);
  m_RangeSlider->setStyle(sliderStyle);
  connect(m_RangeSlider, SIGNAL(spanChanged(int, int)  ),this, SLOT( OnSpanChanged(int , int ) ));
  
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

void QmitkTransferFunctionWidget::SetDataNode(mitk::DataNode* node)
{

  //MITK_INFO << "TransW called with" << (1&&node);
  

  if (node)
  {
    tfpToChange = dynamic_cast<mitk::TransferFunctionProperty*>(node->GetProperty("TransferFunction"));
    
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

    if( mitk::Image* image = dynamic_cast<mitk::Image*>( node->GetData() ) )
    {
      mitk::SimpleHistogram *h = histogramCache[image];

      m_RangeSliderMin= h->GetMin();
      m_RangeSliderMax= h->GetMax();
         
      m_ScalarOpacityFunctionCanvas->SetHistogram( h );
      m_GradientOpacityCanvas->SetHistogram( h );
    }

    OnUpdateCanvas();

    m_RangeSlider->setSpan( m_RangeSliderMin, m_RangeSliderMax);

    UpdateRanges();

    return; 
  }

  turnOff:

  m_ScalarOpacityFunctionCanvas->setEnabled(false);
  m_GradientOpacityCanvas->setEnabled(false);
  m_ColorTransferFunctionCanvas->setEnabled(false);
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

void QmitkTransferFunctionWidget::SetXValueScalar()
{
  m_ScalarOpacityFunctionCanvas->SetX(m_XEditScalarOpacity->text().toFloat());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkTransferFunctionWidget::SetYValueScalar()
{
  m_ScalarOpacityFunctionCanvas->SetY(m_YEditScalarOpacity->text().toFloat());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkTransferFunctionWidget::SetXValueGradient()
{
  m_GradientOpacityCanvas->SetX(m_XEditGradientOpacity->text().toFloat());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkTransferFunctionWidget::SetYValueGradient()
{
  m_GradientOpacityCanvas->SetY(m_YEditGradientOpacity->text().toFloat());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkTransferFunctionWidget::SetXValueColor()
{
  m_ColorTransferFunctionCanvas->SetX(m_XEditColor->text().toFloat());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkTransferFunctionWidget::UpdateRanges() 
{
  int lower =  m_RangeSlider->lowerValue();
  int upper =  m_RangeSlider->upperValue();

  //MITK_INFO << "UpdateRanges: lower: " << lower << " upper: " << upper;

  m_ScalarOpacityFunctionCanvas->SetMin(lower);
  m_ScalarOpacityFunctionCanvas->SetMax(upper);
  
  m_GradientOpacityCanvas->SetMin(lower);
  m_GradientOpacityCanvas->SetMax(upper);
  
  m_ColorTransferFunctionCanvas->SetMin(lower);
  m_ColorTransferFunctionCanvas->SetMax(upper);
}  


void QmitkTransferFunctionWidget::OnSpanChanged(int /*lower*/, int /*upper*/) 
{
  //MITK_INFO << "OnSpanChanged, m_RangeSlider: lowerValue: " << lower << "upperValue: " << upper;

  UpdateRanges();
  
  m_GradientOpacityCanvas->update();
  m_ColorTransferFunctionCanvas->update();
  m_ScalarOpacityFunctionCanvas->update();
  
  
}

void QmitkTransferFunctionWidget::OnResetSlider() 
{
  m_RangeSlider->setUpperValue(m_RangeSliderMax);
  m_RangeSlider->setLowerValue(m_RangeSliderMin);
  
  UpdateRanges();
  m_GradientOpacityCanvas->update();
  m_ColorTransferFunctionCanvas->update();
  m_ScalarOpacityFunctionCanvas->update();
}
