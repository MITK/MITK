/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-20 13:35:09 +0200 (Mi, 20 Mai 2009) $
Version:   $Revision: 17332 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkToFVisualisationSettingsWidget.h"

//QT headers
#include <QPlastiqueStyle>
#include <QString>

const std::string QmitkToFVisualisationSettingsWidget::VIEW_ID = "org.mitk.views.qmitktofvisualisationsettingswidget";

QmitkToFVisualisationSettingsWidget::QmitkToFVisualisationSettingsWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
, m_Controls(NULL)
, m_RangeSliderMin(0)
, m_RangeSliderMax(0)
, m_MitkDistanceImage(NULL)
, m_MitkAmplitudeImage(NULL)
, m_MitkIntensityImage(NULL)
, m_Widget1ColorTransferFunction(NULL)
, m_Widget2ColorTransferFunction(NULL)
, m_Widget3ColorTransferFunction(NULL)
, m_Widget1TransferFunctionType(1)
, m_Widget2TransferFunctionType(0)
, m_Widget3TransferFunctionType(0)
{
  CreateQtPartControl(this);
}

QmitkToFVisualisationSettingsWidget::~QmitkToFVisualisationSettingsWidget()
{
}

void QmitkToFVisualisationSettingsWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToFVisualisationSettingsWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}

void QmitkToFVisualisationSettingsWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_SelectWidgetCombobox), SIGNAL(currentIndexChanged(int)),(QObject*) this, SLOT(OnWidgetSelected(int)) );
    connect( (QObject*)(m_Controls->m_SelectTransferFunctionTypeCombobox), SIGNAL(currentIndexChanged(int)),(QObject*) this, SLOT(OnTransferFunctionTypeSelected(int)) );
    connect( (QObject*)(m_Controls->m_TransferFunctionResetButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnTransferFunctionReset()) );
    connect(m_Controls->m_XEditColor, SIGNAL(returnPressed()), this, SLOT(OnSetXValueColor()));
    connect(m_Controls->m_RangeSliderMaxEdit, SIGNAL(returnPressed()), this, SLOT(OnRangeSliderMaxChanged()));
    connect(m_Controls->m_RangeSliderMinEdit, SIGNAL(returnPressed()), this, SLOT(OnRangeSliderMinChanged()));
    connect(m_Controls->m_RangeSliderReset, SIGNAL(pressed()), this, SLOT(OnResetSlider()));
    connect(m_Controls->m_RangeSlider, SIGNAL(spanChanged(int, int)  ),this, SLOT( OnSpanChanged(int , int ) ));

    QPlastiqueStyle *sliderStyle = new QPlastiqueStyle();
    m_Controls->m_RangeSlider->setMaximum(2048);
    m_Controls->m_RangeSlider->setMinimum(-2048);
    m_Controls->m_RangeSlider->setHandleMovementMode(QxtSpanSlider::NoOverlapping);
    m_Controls->m_RangeSlider->setStyle(sliderStyle);
  
    m_Controls->m_ColorTransferFunctionCanvas->SetQLineEdits(m_Controls->m_XEditColor, 0);
    m_Controls->m_ColorTransferFunctionCanvas->SetTitle(""/*"Value -> Grayscale/Color"*/);
  }
}

void QmitkToFVisualisationSettingsWidget::OnSetXValueColor()
{
  m_Controls->m_ColorTransferFunctionCanvas->SetX(m_Controls->m_XEditColor->text().toFloat());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkToFVisualisationSettingsWidget::OnRangeSliderMaxChanged()
{
  m_Controls->m_RangeSlider->setMaximum(m_Controls->m_RangeSliderMaxEdit->text().toInt());
  UpdateRanges();
  m_Controls->m_ColorTransferFunctionCanvas->update();
}

void QmitkToFVisualisationSettingsWidget::OnRangeSliderMinChanged()
{
  m_Controls->m_RangeSlider->setMinimum(m_Controls->m_RangeSliderMinEdit->text().toInt());
  UpdateRanges();
  m_Controls->m_ColorTransferFunctionCanvas->update();
}

void QmitkToFVisualisationSettingsWidget::OnSpanChanged(int /*lower*/, int /*upper*/) 
{
  UpdateRanges();
  m_Controls->m_ColorTransferFunctionCanvas->update();
}

void QmitkToFVisualisationSettingsWidget::OnResetSlider() 
{
  m_Controls->m_RangeSlider->setUpperValue(m_RangeSliderMax);
  m_Controls->m_RangeSlider->setLowerValue(m_RangeSliderMin);
  
  UpdateRanges();
  m_Controls->m_ColorTransferFunctionCanvas->update();
}

void QmitkToFVisualisationSettingsWidget::UpdateRanges() 
{
  int lower =  m_Controls->m_RangeSlider->lowerValue();
  int upper =  m_Controls->m_RangeSlider->upperValue();

  m_Controls->m_ColorTransferFunctionCanvas->SetMin(lower);
  m_Controls->m_ColorTransferFunctionCanvas->SetMax(upper);
} 

void QmitkToFVisualisationSettingsWidget::Initialize(mitk::Image* distanceImage, mitk::Image* amplitudeImage, mitk::Image* intensityImage)
{
  this->m_MitkDistanceImage = distanceImage;
  this->m_MitkAmplitudeImage = amplitudeImage;
  this->m_MitkIntensityImage = intensityImage;

  if (!m_MitkDistanceImage && !m_MitkAmplitudeImage && !m_MitkIntensityImage)
  {
    m_Controls->m_ColorTransferFunctionCanvas->setEnabled(false);
  }
  else
  {
    m_Controls->m_ColorTransferFunctionCanvas->setEnabled(true);
    int numberOfImages = 0;
    if (m_MitkDistanceImage)
    {
      m_RangeSliderMin = distanceImage->GetScalarValueMin();
      m_RangeSliderMax = distanceImage->GetScalarValueMax();

      m_Widget1ColorTransferFunction = vtkColorTransferFunction::New();
      this->m_Widget1TransferFunctionType = 1;
      m_Controls->m_SelectTransferFunctionTypeCombobox->setCurrentIndex(this->m_Widget1TransferFunctionType);
      ResetTransferFunction(this->m_Widget1ColorTransferFunction, this->m_Widget1TransferFunctionType ,distanceImage->GetScalarValueMin(), distanceImage->GetScalarValueMax());
      UpdateCanvas();

      int border = (m_RangeSliderMax - m_RangeSliderMin) * 0.1;
      m_Controls->m_RangeSlider->setMinimum(m_RangeSliderMin - border);
      m_Controls->m_RangeSlider->setMaximum(m_RangeSliderMax + border);
      m_Controls->m_RangeSliderMinEdit->setText(QString("").setNum(m_RangeSliderMin - border));
      m_Controls->m_RangeSliderMaxEdit->setText(QString("").setNum(m_RangeSliderMax + border));
      m_Controls->m_RangeSlider->setSpan( m_RangeSliderMin, m_RangeSliderMax);

      UpdateRanges();
      numberOfImages++;
    }
    if (m_MitkAmplitudeImage)
    {
      m_Widget2ColorTransferFunction = vtkColorTransferFunction::New();
      this->m_Widget2TransferFunctionType = 0;
      ResetTransferFunction(this->m_Widget2ColorTransferFunction, this->m_Widget2TransferFunctionType, amplitudeImage->GetScalarValueMin(), amplitudeImage->GetScalarValueMax());
      numberOfImages++;
    }
    if (m_MitkIntensityImage)
    {
      m_Widget3ColorTransferFunction = vtkColorTransferFunction::New();
      this->m_Widget3TransferFunctionType = 0;
      ResetTransferFunction(this->m_Widget3ColorTransferFunction, this->m_Widget3TransferFunctionType, intensityImage->GetScalarValueMin(), intensityImage->GetScalarValueMax());
      numberOfImages++;
    }
    m_Controls->m_SelectWidgetCombobox->setMaxCount(numberOfImages);
  }
}

void QmitkToFVisualisationSettingsWidget::UpdateCanvas()
{
  m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->GetSelectedColorTransferFunction() );
  UpdateRanges();
  m_Controls->m_ColorTransferFunctionCanvas->update();
}

void QmitkToFVisualisationSettingsWidget::OnTransferFunctionTypeSelected(int index)
{
  int currentWidgetIndex = m_Controls->m_SelectWidgetCombobox->currentIndex();
  if (currentWidgetIndex == 0)
  {
    this->m_Widget1TransferFunctionType = index;
  }
  else if (currentWidgetIndex == 1)
  {
    this->m_Widget2TransferFunctionType = index;
  }
  else if (currentWidgetIndex == 2)
  {
    this->m_Widget3TransferFunctionType = index;
  }
  else 
  {
    return;
  }
}

void QmitkToFVisualisationSettingsWidget::OnWidgetSelected(int index)
{
  int currentWidgetIndex = m_Controls->m_SelectWidgetCombobox->currentIndex();

  double valMin[6];
  double valMax[6];
  int numPoints;
  if (currentWidgetIndex == 0)
  {
    m_Controls->m_SelectTransferFunctionTypeCombobox->setCurrentIndex(this->m_Widget1TransferFunctionType);
    numPoints = this->m_Widget1ColorTransferFunction->GetSize();
    this->m_Widget1ColorTransferFunction->GetNodeValue( 0, valMin );
    this->m_Widget1ColorTransferFunction->GetNodeValue( numPoints-1, valMax );
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_Widget1ColorTransferFunction );
  }
  else if (currentWidgetIndex == 1)
  {
    m_Controls->m_SelectTransferFunctionTypeCombobox->setCurrentIndex(this->m_Widget2TransferFunctionType);
    numPoints = this->m_Widget2ColorTransferFunction->GetSize();
    this->m_Widget2ColorTransferFunction->GetNodeValue( 0, valMin );
    this->m_Widget2ColorTransferFunction->GetNodeValue( numPoints-1, valMax );
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_Widget2ColorTransferFunction );
  }
  else if (currentWidgetIndex == 2)
  {
    m_Controls->m_SelectTransferFunctionTypeCombobox->setCurrentIndex(this->m_Widget3TransferFunctionType);
    numPoints = this->m_Widget3ColorTransferFunction->GetSize();
    this->m_Widget3ColorTransferFunction->GetNodeValue( 0, valMin );
    this->m_Widget3ColorTransferFunction->GetNodeValue( numPoints-1, valMax );
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_Widget3ColorTransferFunction );
  }
  else if (currentWidgetIndex == 3)
  {
  }
  else 
  {
    return;
  }
  m_RangeSliderMin = valMin[0];
  m_RangeSliderMax = valMax[0];
  int border = (m_RangeSliderMax - m_RangeSliderMin) * 0.1;
  m_Controls->m_RangeSlider->setMinimum(m_RangeSliderMin - border);
  m_Controls->m_RangeSlider->setMaximum(m_RangeSliderMax + border);
  m_Controls->m_RangeSliderMinEdit->setText(QString("").setNum(m_RangeSliderMin - border));
  m_Controls->m_RangeSliderMaxEdit->setText(QString("").setNum(m_RangeSliderMax + border));
  m_Controls->m_RangeSlider->setSpan( m_RangeSliderMin, m_RangeSliderMax);
  UpdateRanges();
  m_Controls->m_ColorTransferFunctionCanvas->update();
}

void QmitkToFVisualisationSettingsWidget::ResetTransferFunction(vtkColorTransferFunction* colorTransferFunction, int type, double min, double max)
{
  colorTransferFunction->RemoveAllPoints();
  if (type == 0)
  {
    colorTransferFunction->AddRGBPoint(min, 0, 0, 0);
    colorTransferFunction->AddRGBPoint(max, 1, 1, 1);  
  }
  else
  {
    if (min>0.01)
    {
      colorTransferFunction->AddRGBPoint(0.0, 0, 0, 0);
      colorTransferFunction->AddRGBPoint(min-0.01, 0, 0, 0);
    }
    colorTransferFunction->AddRGBPoint(min, 1, 0, 0);
    colorTransferFunction->AddRGBPoint(min+(max-min)/2, 1, 1, 0);  
    colorTransferFunction->AddRGBPoint(max, 0, 0, 1);  
  }
  colorTransferFunction->SetColorSpaceToHSV();
}

void QmitkToFVisualisationSettingsWidget::OnTransferFunctionReset()
{
  int currentTransferFunctionTypeIndex = m_Controls->m_SelectTransferFunctionTypeCombobox->currentIndex();
  int currentWidgetIndex = m_Controls->m_SelectWidgetCombobox->currentIndex();

  if (currentWidgetIndex == 0)
  {
    // use second minimum to draw 0 values (that are usually segmented) black
    m_RangeSliderMin = this->m_MitkDistanceImage->GetScalarValue2ndMin();
    m_RangeSliderMax = this->m_MitkDistanceImage->GetScalarValueMaxNoRecompute();
    ResetTransferFunction(this->m_Widget1ColorTransferFunction, currentTransferFunctionTypeIndex, this->m_RangeSliderMin, this->m_RangeSliderMax);
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_Widget1ColorTransferFunction );
  }
  else if (currentWidgetIndex == 1)
  {
    m_RangeSliderMin = this->m_MitkAmplitudeImage->GetScalarValueMin();
    m_RangeSliderMax = this->m_MitkAmplitudeImage->GetScalarValueMaxNoRecompute();
    ResetTransferFunction(this->m_Widget2ColorTransferFunction, currentTransferFunctionTypeIndex, this->m_RangeSliderMin, this->m_RangeSliderMax);
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_Widget2ColorTransferFunction );
  }
  else if (currentWidgetIndex == 2)
  {
    m_RangeSliderMin = this->m_MitkIntensityImage->GetScalarValueMin();
    m_RangeSliderMax = this->m_MitkIntensityImage->GetScalarValueMaxNoRecompute();
    ResetTransferFunction(this->m_Widget3ColorTransferFunction, currentTransferFunctionTypeIndex, this->m_RangeSliderMin, this->m_RangeSliderMax);
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_Widget3ColorTransferFunction );
  }
  else
  {
    return;
  }
  int border = (m_RangeSliderMax - m_RangeSliderMin) * 0.1;
  m_Controls->m_RangeSlider->setMinimum(m_RangeSliderMin - border);
  m_Controls->m_RangeSlider->setMaximum(m_RangeSliderMax + border);
  m_Controls->m_RangeSliderMinEdit->setText(QString("").setNum(m_RangeSliderMin - border));
  m_Controls->m_RangeSliderMaxEdit->setText(QString("").setNum(m_RangeSliderMax + border));
  m_Controls->m_RangeSlider->setSpan( m_RangeSliderMin, m_RangeSliderMax);
  UpdateRanges();
  m_Controls->m_ColorTransferFunctionCanvas->update();

}

vtkColorTransferFunction* QmitkToFVisualisationSettingsWidget::GetWidget1ColorTransferFunction()
{  
  return this->m_Widget1ColorTransferFunction;
}

vtkColorTransferFunction* QmitkToFVisualisationSettingsWidget::GetWidget2ColorTransferFunction()
{  
  return this->m_Widget2ColorTransferFunction;
}

vtkColorTransferFunction* QmitkToFVisualisationSettingsWidget::GetWidget3ColorTransferFunction()
{  
  return this->m_Widget3ColorTransferFunction;
}

vtkColorTransferFunction* QmitkToFVisualisationSettingsWidget::GetSelectedColorTransferFunction()
{
  int currentWidgetIndex = m_Controls->m_SelectWidgetCombobox->currentIndex();
  if (currentWidgetIndex==0)
  {
    return this->m_Widget1ColorTransferFunction;
  }
  else if (currentWidgetIndex==1)
  {
    return this->m_Widget2ColorTransferFunction;
  }
  else if (currentWidgetIndex==2)
  {
    return this->m_Widget3ColorTransferFunction;
  }
  else
  {
    return this->m_Widget3ColorTransferFunction;
  }
}

int QmitkToFVisualisationSettingsWidget::GetSelectedImageIndex()
{
  return this->m_Controls->m_SelectWidgetCombobox->currentIndex();
}
