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

#include "QmitkToFVisualisationSettingsWidget.h"

#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkImageStatisticsHolder.h>

//QT headers
#include <QString>
#include <mitkTransferFunctionProperty.h>
#include <mitkTransferFunction.h>

const std::string QmitkToFVisualisationSettingsWidget::VIEW_ID = "org.mitk.views.qmitktofvisualisationsettingswidget";

QmitkToFVisualisationSettingsWidget::QmitkToFVisualisationSettingsWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
, m_Controls(NULL)
, m_RangeSliderMin(0)
, m_RangeSliderMax(0)
, m_MitkDistanceImageNode(NULL)
, m_MitkAmplitudeImageNode(NULL)
, m_MitkIntensityImageNode(NULL)
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
    connect(m_Controls->m_AdvancedOptionsCheckbox, SIGNAL(toggled(bool)  ),this, SLOT( OnShowAdvancedOptionsCheckboxChecked(bool) ));

    m_Controls->m_RangeSlider->setMaximum(2048);
    m_Controls->m_RangeSlider->setMinimum(-2048);

    m_Controls->m_ColorTransferFunctionCanvas->SetQLineEdits(m_Controls->m_XEditColor, 0);
    m_Controls->m_ColorTransferFunctionCanvas->SetTitle(""/*"Value -> Grayscale/Color"*/);

    this->OnShowAdvancedOptionsCheckboxChecked(false);
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
  m_Controls->m_RangeSlider->setMaximumValue(m_RangeSliderMax);
  m_Controls->m_RangeSlider->setMinimumValue(m_RangeSliderMin);

  UpdateRanges();
  m_Controls->m_ColorTransferFunctionCanvas->update();
}

void QmitkToFVisualisationSettingsWidget::UpdateRanges()
{
  int lower =  m_Controls->m_RangeSlider->minimumValue();
  int upper =  m_Controls->m_RangeSlider->maximumValue();

  m_Controls->m_ColorTransferFunctionCanvas->SetMin(lower);
  m_Controls->m_ColorTransferFunctionCanvas->SetMax(upper);
}

void QmitkToFVisualisationSettingsWidget::UpdateSurfaceProperty()
{
  if(this->m_MitkSurfaceNode.IsNotNull())
  {
    mitk::TransferFunction::Pointer transferFunction = mitk::TransferFunction::New();
    transferFunction->SetColorTransferFunction(this->GetSelectedColorTransferFunction());

    this->m_MitkSurfaceNode->SetProperty("Surface.TransferFunction", mitk::TransferFunctionProperty::New(transferFunction));
  }
}

void QmitkToFVisualisationSettingsWidget::Initialize(mitk::DataNode* distanceImageNode, mitk::DataNode* amplitudeImageNode,
                                                     mitk::DataNode* intensityImageNode, mitk::DataNode* surfaceNode)
{
  this->m_MitkDistanceImageNode = distanceImageNode;
  this->m_MitkAmplitudeImageNode = amplitudeImageNode;
  this->m_MitkIntensityImageNode = intensityImageNode;
  this->m_MitkSurfaceNode = surfaceNode;

  // Initialize transfer functions for image DataNodes such that:
  // Widget1 (Distance): color from red (2nd min) to blue (max)
  // Widget2 (Amplitude): grey value from black (2nd min) to white (max)
  // Widget3 (Intensity): grey value from black (2nd min) to white (max)
  if (!m_MitkDistanceImageNode && !m_MitkAmplitudeImageNode && !m_MitkIntensityImageNode)
  {
    m_Controls->m_ColorTransferFunctionCanvas->setEnabled(false);
  }
  else
  {
    m_Controls->m_ColorTransferFunctionCanvas->setEnabled(true);
    int numberOfImages = 0;
    if (m_MitkDistanceImageNode)
    {
      m_Widget1ColorTransferFunction = vtkColorTransferFunction::New();
      this->m_Widget1TransferFunctionType = 1;
      m_Controls->m_SelectTransferFunctionTypeCombobox->setCurrentIndex(this->m_Widget1TransferFunctionType);
      numberOfImages++;
    }
    if (m_MitkAmplitudeImageNode)
    {
      m_Widget2ColorTransferFunction = vtkColorTransferFunction::New();
      this->m_Widget2TransferFunctionType = 0;
      numberOfImages++;
    }
    if (m_MitkIntensityImageNode)
    {
      m_Widget3ColorTransferFunction = vtkColorTransferFunction::New();
      this->m_Widget3TransferFunctionType = 0;
      numberOfImages++;
    }
    m_Controls->m_SelectWidgetCombobox->setMaxCount(numberOfImages);
  }
  this->ReinitTransferFunction(0,1);
  this->ReinitTransferFunction(1,0);
  this->ReinitTransferFunction(2,0);
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
  this->UpdateSurfaceProperty();
}

void QmitkToFVisualisationSettingsWidget::OnShowAdvancedOptionsCheckboxChecked(bool checked)
{
  this->m_Controls->m_MappingGroupBox->setVisible(checked);
  this->m_Controls->m_SelectTransferFunctionTypeCombobox->setVisible(checked);
  this->m_Controls->m_SelectWidgetCombobox->setVisible(checked);
  this->m_Controls->m_TransferFunctionResetButton->setVisible(checked);
}

void QmitkToFVisualisationSettingsWidget::OnWidgetSelected(int index)
{
  int currentWidgetIndex = index;

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
  m_Controls->m_RangeSlider->setRange( m_RangeSliderMin, m_RangeSliderMax);
  UpdateRanges();
  m_Controls->m_ColorTransferFunctionCanvas->update();
  this->UpdateSurfaceProperty();
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

void QmitkToFVisualisationSettingsWidget::ReinitTransferFunction(int widget, int type)
{
  switch (widget)
  {
  case 0:
  {
    mitk::Image::Pointer distanceImage = dynamic_cast<mitk::Image*>(m_MitkDistanceImageNode->GetData());
    // use second minimum to draw 0 values (that are usually segmented) black
    m_RangeSliderMin = distanceImage->GetStatistics()->GetScalarValue2ndMin();
    m_RangeSliderMax = distanceImage->GetStatistics()->GetScalarValueMax();
    MITK_INFO<<"Distance Min: "<<m_RangeSliderMin;
    MITK_INFO<<"Distance Max: "<<m_RangeSliderMax;
    ResetTransferFunction(this->m_Widget1ColorTransferFunction, type, this->m_RangeSliderMin, this->m_RangeSliderMax);
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_Widget1ColorTransferFunction );
    mitk::TransferFunction::Pointer tf1 = mitk::TransferFunction::New();
    tf1->SetColorTransferFunction( m_Widget1ColorTransferFunction );
    m_MitkDistanceImageNode->SetProperty("Image Rendering.Transfer Function",mitk::TransferFunctionProperty::New(tf1));
    break;
  }
  case 1:
  {
    if (m_MitkAmplitudeImageNode)
    {
      mitk::Image::Pointer amplitudeImage = dynamic_cast<mitk::Image*>(m_MitkAmplitudeImageNode->GetData());
      if (amplitudeImage.IsNotNull())
      {
        m_RangeSliderMin = amplitudeImage->GetStatistics()->GetScalarValueMin();
        m_RangeSliderMax = amplitudeImage->GetStatistics()->GetScalarValueMax();
        MITK_INFO<<"Amplitude Min: "<<m_RangeSliderMin;
        MITK_INFO<<"Amplitude Max: "<<m_RangeSliderMax;
        ResetTransferFunction(this->m_Widget2ColorTransferFunction, type, this->m_RangeSliderMin, this->m_RangeSliderMax);
        m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_Widget2ColorTransferFunction );
        mitk::TransferFunction::Pointer tf2 = mitk::TransferFunction::New();
        tf2->SetColorTransferFunction( m_Widget2ColorTransferFunction );
        m_MitkAmplitudeImageNode->SetProperty("Image Rendering.Transfer Function",mitk::TransferFunctionProperty::New(tf2));
      }
    }
    break;
  }
  case 2:
  {
    if (m_MitkIntensityImageNode)
    {
      mitk::Image::Pointer intensityImage = dynamic_cast<mitk::Image*>(m_MitkIntensityImageNode->GetData());
      if (intensityImage.IsNotNull())
      {
        m_RangeSliderMin = intensityImage->GetStatistics()->GetScalarValueMin();
        m_RangeSliderMax = intensityImage->GetStatistics()->GetScalarValueMax();
        MITK_INFO<<"Intensity Min: "<<m_RangeSliderMin;
        MITK_INFO<<"Intensity Max: "<<m_RangeSliderMax;
        ResetTransferFunction(this->m_Widget3ColorTransferFunction, type, this->m_RangeSliderMin, this->m_RangeSliderMax);
        m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_Widget3ColorTransferFunction );
        mitk::TransferFunction::Pointer tf3 = mitk::TransferFunction::New();
        tf3->SetColorTransferFunction( m_Widget3ColorTransferFunction );
        m_MitkIntensityImageNode->SetProperty("Image Rendering.Transfer Function",mitk::TransferFunctionProperty::New(tf3));
      }
    }
    break;
  }
  default:
    break;
  }
  this->UpdateSurfaceProperty();
}

void QmitkToFVisualisationSettingsWidget::OnTransferFunctionReset()
{
  int currentTransferFunctionTypeIndex = m_Controls->m_SelectTransferFunctionTypeCombobox->currentIndex();
  int currentWidgetIndex = m_Controls->m_SelectWidgetCombobox->currentIndex();

  this->ReinitTransferFunction(currentWidgetIndex,currentTransferFunctionTypeIndex);

  int border = (m_RangeSliderMax - m_RangeSliderMin) * 0.1;
  m_Controls->m_RangeSlider->setMinimum(m_RangeSliderMin - border);
  m_Controls->m_RangeSlider->setMaximum(m_RangeSliderMax + border);
  m_Controls->m_RangeSliderMinEdit->setText(QString("").setNum(m_RangeSliderMin - border));
  m_Controls->m_RangeSliderMaxEdit->setText(QString("").setNum(m_RangeSliderMax + border));
  m_Controls->m_RangeSlider->setRange( m_RangeSliderMin, m_RangeSliderMax);
  UpdateRanges();
  m_Controls->m_ColorTransferFunctionCanvas->update();
  this->UpdateSurfaceProperty();
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
