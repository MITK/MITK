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

#define _USE_MATH_DEFINES
#include <limits>
#include "QmitkToFVisualisationSettingsWidget.h"

//QT headers
#include <qmessagebox.h>
#include <QPlastiqueStyle>
#include <QString>

//mitk headers
#include "mitkToFImageGrabber.h"
#include "mitkNodePredicateDataType.h"
#include <mitkInstantiateAccessFunctions.h>

#include <mitkTransferFunctionProperty.h>

//itk headers

//vtk headers

//Qmitk headers
#include "QmitkStdMultiWidget.h"

const std::string QmitkToFVisualisationSettingsWidget::VIEW_ID = "org.mitk.views.qmitktofvisualisationsettingswidget";

QmitkToFVisualisationSettingsWidget::QmitkToFVisualisationSettingsWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  InitMember();

  m_Controls = NULL;
  CreateQtPartControl(this);

  //this->setupUi(this);
                                               
}

void QmitkToFVisualisationSettingsWidget::InitMember()
{
  this->m_Channel = 0;
  this->m_Type = 0;
  this->m_CurrentImageNode = NULL;

  //SetImageType(this->m_Widget1ImageType, 0);
  //SetImageType(this->m_Widget2ImageType, 1);
  //SetImageType(this->m_Widget3ImageType, 2);
  //SetImageType(this->m_Widget4ImageType, 3);

  this->m_ToFVisualizationFilter = NULL;
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

void QmitkToFVisualisationSettingsWidget::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  //m_MultiWidget = &stdMultiWidget;
}

void QmitkToFVisualisationSettingsWidget::StdMultiWidgetNotAvailable()
{
  //m_MultiWidget = NULL;
}

void QmitkToFVisualisationSettingsWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_SelectWidgetCombobox), SIGNAL(currentIndexChanged(int)),(QObject*) this, SLOT(OnWidgetSelected(int)) );

    connect( (QObject*)(m_Controls->m_SelectImageTypeCombobox), SIGNAL(currentIndexChanged(int)),(QObject*) this, SLOT(OnImageTypeSelected(int)) );

    connect( (QObject*)(m_Controls->m_SelectTransferFunctionTypeCombobox), SIGNAL(currentIndexChanged(int)),(QObject*) this, SLOT(OnTransferFunctionTypeSelected(int)) );

    connect( (QObject*)(m_Controls->m_TransferFunctionResetButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnTransferFunctionReset()) );

    connect(m_Controls->m_XEditColor, SIGNAL(returnPressed()), this, SLOT(SetXValueColor()));

    connect(m_Controls->m_RangeSliderMaxEdit, SIGNAL(returnPressed()), this, SLOT(OnRangeSliderMaxChanged()));
  
    connect(m_Controls->m_RangeSliderMinEdit, SIGNAL(returnPressed()), this, SLOT(OnRangeSliderMinChanged()));

    QPlastiqueStyle *sliderStyle = new QPlastiqueStyle();

    m_Controls->m_RangeSlider->setMaximum(2048);
    m_Controls->m_RangeSlider->setMinimum(-2048);
    m_Controls->m_RangeSlider->setHandleMovementMode(QxtSpanSlider::NoOverlapping);
    m_Controls->m_RangeSlider->setStyle(sliderStyle);
    connect(m_Controls->m_RangeSlider, SIGNAL(spanChanged(int, int)  ),this, SLOT( OnSpanChanged(int , int ) ));
  
    //reset button
    connect(m_Controls->m_RangeSliderReset, SIGNAL(pressed()), this, SLOT(OnResetSlider()));
  
    m_Controls->m_ColorTransferFunctionCanvas->SetQLineEdits(m_Controls->m_XEditColor, 0);

    m_Controls->m_ColorTransferFunctionCanvas->SetTitle(""/*"Value -> Grayscale/Color"*/);

  }
}

void QmitkToFVisualisationSettingsWidget::Initialize(mitk::DataStorage* dataStorage, QmitkStdMultiWidget* multiWidget)
{
  this->m_DataStorage = dataStorage;
  this->m_MultiWidget = multiWidget;
}

void QmitkToFVisualisationSettingsWidget::SetParameter(mitk::ToFVisualizationFilter* ToFVisualizationFilter)
{
  this->m_ToFVisualizationFilter = ToFVisualizationFilter;
}

void QmitkToFVisualisationSettingsWidget::Reset()
{
}

mitk::ToFVisualizationFilter* QmitkToFVisualisationSettingsWidget::GetToFVisualizationFilter()
{
  if (this->m_ToFVisualizationFilter.IsNull())
  {
    this->m_ToFVisualizationFilter = mitk::ToFVisualizationFilter::New();
  }
  return this->m_ToFVisualizationFilter;
}

void QmitkToFVisualisationSettingsWidget::SetXValueColor()
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

void QmitkToFVisualisationSettingsWidget::SetDataNode(mitk::DataNode* node)
{
  if (node)
  {
    m_TfpToChange = dynamic_cast<mitk::TransferFunctionProperty*>(node->GetProperty("TransferFunction"));
    
    if(!m_TfpToChange)
    {
      if (! dynamic_cast<mitk::Image*>(node->GetData()))
      {
        MITK_WARN << "QmitkTransferFunctionWidget::SetDataNode called with non-image node";
        goto turnOff;
      }
      
      m_TfpToChange = mitk::TransferFunctionProperty::New();
      mitk::TransferFunction::Pointer ctf = mitk::TransferFunction::New();
      ctf->InitializeByMitkImage(dynamic_cast<mitk::Image*>(node->GetData()));
      m_TfpToChange->SetValue(ctf);
      node->SetProperty("TransferFunction", m_TfpToChange );
    }

    mitk::TransferFunction::Pointer tf = m_TfpToChange->GetValue();

    if( mitk::Image* image = dynamic_cast<mitk::Image*>( node->GetData() ) )
    {
      mitk::SimpleHistogram *h = histogramCache[image];

      m_RangeSliderMin= h->GetMin();
      m_RangeSliderMax= h->GetMax();
         
    }

    OnUpdateCanvas();

    m_Controls->m_RangeSlider->setSpan( m_RangeSliderMin, m_RangeSliderMax);

    UpdateRanges();

    return; 
  }

  turnOff:

  m_Controls->m_ColorTransferFunctionCanvas->setEnabled(false);
  m_TfpToChange = 0;
}

void QmitkToFVisualisationSettingsWidget::InitializeTransferFunction(mitk::Image* distanceImage, mitk::Image* amplitudeImage, mitk::Image* intensityImage)
{
  this->m_MitkDistanceImage = distanceImage;
  this->m_MitkAmplitudeImage = amplitudeImage;
  this->m_MitkIntensityImage = intensityImage;

  if (distanceImage && distanceImage && intensityImage)
  {
    m_RangeSliderMin = distanceImage->GetScalarValueMin();
    m_RangeSliderMax = distanceImage->GetScalarValueMax();

    //m_Widget1ColorTransferFunction = vtkColorTransferFunction::New();
    ////m_Widget1ColorTransferFunction->RemoveAllPoints();
    ////m_Widget1ColorTransferFunction->AddRGBPoint(m_RangeSliderMin, 1, 0, 0);
    ////m_Widget1ColorTransferFunction->AddRGBPoint((m_RangeSliderMin+(m_RangeSliderMax-m_RangeSliderMin)/2, 0, 1, 0);  
    ////m_Widget1ColorTransferFunction->AddRGBPoint(m_RangeSliderMax, 0, 0, 1);  
    ////m_Widget1ColorTransferFunction->SetColorSpaceToHSV();
    //this->m_Widget1TransferFunctionType = 1;
    //ResetTransferFunction(this->m_Widget1ColorTransferFunction, this->m_Widget1TransferFunctionType ,distanceImage->GetScalarValueMin(), distanceImage->GetScalarValueMax());

    //m_Widget2ColorTransferFunction = vtkColorTransferFunction::New();
    ////m_Widget2ColorTransferFunction->RemoveAllPoints();
    ////m_Widget2ColorTransferFunction->AddRGBPoint(amplitudeImage->GetScalarValueMin(), 1, 1, 1);
    ////m_Widget2ColorTransferFunction->AddRGBPoint(amplitudeImage->GetScalarValueMax(), 0, 0, 0);  
    ////m_Widget2ColorTransferFunction->SetColorSpaceToHSV();
    //this->m_Widget2TransferFunctionType = 0;
    //ResetTransferFunction(this->m_Widget2ColorTransferFunction, this->m_Widget2TransferFunctionType, amplitudeImage->GetScalarValueMin(), amplitudeImage->GetScalarValueMax());

    //m_Widget3ColorTransferFunction = vtkColorTransferFunction::New();
    ////m_Widget3ColorTransferFunction->RemoveAllPoints();
    ////m_Widget3ColorTransferFunction->AddRGBPoint(intensityImage->GetScalarValueMin(), 1, 1, 1);
    ////m_Widget3ColorTransferFunction->AddRGBPoint(intensityImage->GetScalarValueMax(), 0, 0, 0);  
    ////m_Widget3ColorTransferFunction->SetColorSpaceToHSV();
    //this->m_Widget3TransferFunctionType = 0;
    //ResetTransferFunction(this->m_Widget3ColorTransferFunction, this->m_Widget3TransferFunctionType, intensityImage->GetScalarValueMin(), intensityImage->GetScalarValueMax());

    OnUpdateCanvas();

    int border = (m_RangeSliderMax - m_RangeSliderMin) * 0.1;
    m_Controls->m_RangeSlider->setMinimum(m_RangeSliderMin - border);
    m_Controls->m_RangeSlider->setMaximum(m_RangeSliderMax + border);
    m_Controls->m_RangeSliderMinEdit->setText(QString("").setNum(m_RangeSliderMin - border));
    m_Controls->m_RangeSliderMaxEdit->setText(QString("").setNum(m_RangeSliderMax + border));
    m_Controls->m_RangeSlider->setSpan( m_RangeSliderMin, m_RangeSliderMax);

    UpdateRanges();

  }
  else
  {
    m_Controls->m_ColorTransferFunctionCanvas->setEnabled(false);
  }
}

void QmitkToFVisualisationSettingsWidget::OnUpdateCanvas()
{
  //m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_Widget1ColorTransferFunction );
  m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_ToFVisualizationFilter->GetWidget1ColorTransferFunction() );
  UpdateRanges();
  m_Controls->m_ColorTransferFunctionCanvas->update();
}

std::string QmitkToFVisualisationSettingsWidget::GetImageType(int index)
{
  if (index == 0)
  {
    return "Distance";
  }
  else if (index == 1)
  {
    return "Amplitude";
  }
  else if (index == 2)
  {
    return"Intensity";
  }
  else if (index == 3)
  {
    return "Video";
  }
  else if (index == 5)
  {
    return "Surface";
  }
  return "";
}

void QmitkToFVisualisationSettingsWidget::OnImageTypeSelected(int index)
{
  int currentWidgetIndex = m_Controls->m_SelectWidgetCombobox->currentIndex();
  if (currentWidgetIndex == 0)
  {
    //SetImageType(this->m_ToFVisualizationFilter->GetWidget1ImageType(), index);
    this->m_ToFVisualizationFilter->SetWidget1ImageType(GetImageType(index));
    m_Controls->m_SelectTransferFunctionTypeCombobox->setCurrentIndex(this->m_ToFVisualizationFilter->GetWidget1TransferFunctionType());
  }
  else if (currentWidgetIndex == 1)
  {
    //SetImageType(this->m_ToFVisualizationFilter->GetWidget2ImageType(), index);
    this->m_ToFVisualizationFilter->SetWidget2ImageType(GetImageType(index));
    m_Controls->m_SelectTransferFunctionTypeCombobox->setCurrentIndex(this->m_ToFVisualizationFilter->GetWidget2TransferFunctionType());
  }
  else if (currentWidgetIndex == 2)
  {
    //SetImageType(this->m_ToFVisualizationFilter->GetWidget3ImageType(), index);
    this->m_ToFVisualizationFilter->SetWidget3ImageType(GetImageType(index));
    m_Controls->m_SelectTransferFunctionTypeCombobox->setCurrentIndex(this->m_ToFVisualizationFilter->GetWidget3TransferFunctionType());
  }
  //else if (currentWidgetIndex == 3)
  //{
  //  SetImageType(this->m_Widget4ImageType, index);
  //}
  else 
  {
    return;
  }

  if (index==3)
  {
    m_Controls->m_MappingGroupBox->setEnabled(false);
    m_Controls->m_FitScaleGroupBox->setEnabled(false);
  }
  else
  {
    m_Controls->m_MappingGroupBox->setEnabled(true);
    m_Controls->m_FitScaleGroupBox->setEnabled(true);
  }
}

void QmitkToFVisualisationSettingsWidget::OnTransferFunctionTypeSelected(int index)
{
  int currentWidgetIndex = m_Controls->m_SelectWidgetCombobox->currentIndex();
  if (currentWidgetIndex == 0)
  {
    this->m_ToFVisualizationFilter->SetWidget1TransferFunctionType(index);
  }
  else if (currentWidgetIndex == 1)
  {
    this->m_ToFVisualizationFilter->SetWidget2TransferFunctionType(index);
  }
  else if (currentWidgetIndex == 2)
  {
    this->m_ToFVisualizationFilter->SetWidget3TransferFunctionType(index);
  }
  else 
  {
    return;
  }
}

int QmitkToFVisualisationSettingsWidget::GetImageTypeIndex(std::string imageType)
{
  if (imageType.compare("Distance") == 0)
  {
    return 0;
  }
  else if (imageType.compare("Amplitude") == 0)
  {
    return 1;
  }
  else if (imageType.compare("Intensity") == 0)
  {
    return 2;
  }
  else if (imageType.compare("Video") == 0)
  {
    return 3;
  }
  else if (imageType.compare("Surface") == 0)
  {
    return 4;
  }
  return 5;
}

void QmitkToFVisualisationSettingsWidget::OnWidgetSelected(int index)
{
  int currentWidgetIndex = m_Controls->m_SelectWidgetCombobox->currentIndex();

  double valMin[6];
  double valMax[6];
  int numPoints;
  if (currentWidgetIndex == 0)
  {
    m_Controls->m_SelectImageTypeCombobox->setCurrentIndex(GetImageTypeIndex(this->m_ToFVisualizationFilter->GetWidget1ImageType()));
    numPoints = this->m_ToFVisualizationFilter->GetWidget1ColorTransferFunction()->GetSize();
    this->m_ToFVisualizationFilter->GetWidget1ColorTransferFunction()->GetNodeValue( 0, valMin );
    this->m_ToFVisualizationFilter->GetWidget1ColorTransferFunction()->GetNodeValue( numPoints-1, valMax );
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_ToFVisualizationFilter->GetWidget1ColorTransferFunction() );
  }
  else if (currentWidgetIndex == 1)
  {
    m_Controls->m_SelectImageTypeCombobox->setCurrentIndex(GetImageTypeIndex(this->m_ToFVisualizationFilter->GetWidget2ImageType()));
    numPoints = this->m_ToFVisualizationFilter->GetWidget2ColorTransferFunction()->GetSize();
    this->m_ToFVisualizationFilter->GetWidget2ColorTransferFunction()->GetNodeValue( 0, valMin );
    this->m_ToFVisualizationFilter->GetWidget2ColorTransferFunction()->GetNodeValue( numPoints-1, valMax );
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_ToFVisualizationFilter->GetWidget2ColorTransferFunction() );
  }
  else if (currentWidgetIndex == 2)
  {
    m_Controls->m_SelectImageTypeCombobox->setCurrentIndex(GetImageTypeIndex(this->m_ToFVisualizationFilter->GetWidget3ImageType()));
    numPoints = this->m_ToFVisualizationFilter->GetWidget3ColorTransferFunction()->GetSize();
    this->m_ToFVisualizationFilter->GetWidget3ColorTransferFunction()->GetNodeValue( 0, valMin );
    this->m_ToFVisualizationFilter->GetWidget3ColorTransferFunction()->GetNodeValue( numPoints-1, valMax );
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_ToFVisualizationFilter->GetWidget3ColorTransferFunction() );
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
  //TODO implement the Enum Conversion type
  int currentTransferFunctionTypeIndex = m_Controls->m_SelectTransferFunctionTypeCombobox->currentIndex();
  int currentWidgetIndex = m_Controls->m_SelectWidgetCombobox->currentIndex();
  int currentImageTypeIndex = m_Controls->m_SelectImageTypeCombobox->currentIndex();
  int numOfPixel = this->m_MitkDistanceImage->GetDimension(0) * this->m_MitkDistanceImage->GetDimension(1);

  if (currentImageTypeIndex == 0)
  {
    ComputeMinMax((float*)this->m_MitkDistanceImage->GetSliceData(0,0,0)->GetData(), numOfPixel, m_RangeSliderMin, m_RangeSliderMax);
  }
  else if (currentImageTypeIndex == 1)
  {
    ComputeMinMax((float*)this->m_MitkAmplitudeImage->GetSliceData(0,0,0)->GetData(), numOfPixel, m_RangeSliderMin, m_RangeSliderMax);
  }
  else if (currentImageTypeIndex == 2)
  {
    ComputeMinMax((float*)this->m_MitkIntensityImage->GetSliceData(0,0,0)->GetData(), numOfPixel, m_RangeSliderMin, m_RangeSliderMax);
  }
  else
  {
    return;
  }

  if (currentWidgetIndex == 0)
  {
    this->m_ToFVisualizationFilter->SetWidget1TransferFunctionType(currentTransferFunctionTypeIndex);
    ResetTransferFunction(this->m_ToFVisualizationFilter->GetWidget1ColorTransferFunction(), this->m_ToFVisualizationFilter->GetWidget1TransferFunctionType(), 
      this->m_RangeSliderMin, this->m_RangeSliderMax);
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_ToFVisualizationFilter->GetWidget1ColorTransferFunction() );
  }
  else if (currentWidgetIndex == 1)
  {
    this->m_ToFVisualizationFilter->SetWidget2TransferFunctionType(currentTransferFunctionTypeIndex);
    ResetTransferFunction(this->m_ToFVisualizationFilter->GetWidget2ColorTransferFunction(), this->m_ToFVisualizationFilter->GetWidget2TransferFunctionType(), 
      this->m_RangeSliderMin, this->m_RangeSliderMax);
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_ToFVisualizationFilter->GetWidget2ColorTransferFunction() );
  }
  else if (currentWidgetIndex == 2)
  {
    this->m_ToFVisualizationFilter->SetWidget1TransferFunctionType(currentTransferFunctionTypeIndex);
    ResetTransferFunction(this->m_ToFVisualizationFilter->GetWidget3ColorTransferFunction(), this->m_ToFVisualizationFilter->GetWidget3TransferFunctionType(), 
      this->m_RangeSliderMin, this->m_RangeSliderMax);
    m_Controls->m_ColorTransferFunctionCanvas->SetColorTransferFunction( this->m_ToFVisualizationFilter->GetWidget3ColorTransferFunction() );
  }
  else if (currentWidgetIndex == 3)
  {
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

vtkColorTransferFunction* QmitkToFVisualisationSettingsWidget::GetColorTransferFunctionByImageType(char* imageType)
{
/*
  if (this->m_Widget1ImageType.compare(imageType) == 0)
  {
<<<<<<< .mine
    return this->m_ToFVisualizationFilter->GetWidget1ColorTransferFunction();
    //return this->m_Widget1ColorTransferFunction;
=======
    //return this->m_ToFVisualizationFilter->GetWidget1ColorTransferFunction();
    return this->m_Widget1ColorTransferFunction;
>>>>>>> .r29631
  }
  else if (this->m_Widget2ImageType.compare(imageType) == 0)
  {
    return this->m_Widget2ColorTransferFunction;
  }
  else if (this->m_Widget3ImageType.compare(imageType) == 0)
  {
    return this->m_Widget3ColorTransferFunction;
  }
  else
  {
    return this->m_Widget3ColorTransferFunction;
  }
*/
  if (this->m_ToFVisualizationFilter->GetWidget1ImageType().compare(imageType) == 0)
  {
    return this->m_ToFVisualizationFilter->GetWidget1ColorTransferFunction();
  }
  else if (this->m_ToFVisualizationFilter->GetWidget2ImageType().compare(imageType) == 0)
  {
    return this->m_ToFVisualizationFilter->GetWidget2ColorTransferFunction();
  }
  else if (this->m_ToFVisualizationFilter->GetWidget3ImageType().compare(imageType) == 0)
  {
    return this->m_ToFVisualizationFilter->GetWidget3ColorTransferFunction();
  }
  else
  {
    return this->m_ToFVisualizationFilter->GetWidget3ColorTransferFunction();
  }

}

mitk::TransferFunction::Pointer QmitkToFVisualisationSettingsWidget::GetTransferFunction()
{
  mitk::TransferFunction::Pointer tf = m_TfpToChange->GetValue();
  return tf;
}

vtkColorTransferFunction* QmitkToFVisualisationSettingsWidget::GetWidget1ColorTransferFunction()
{  
  return this->m_ToFVisualizationFilter->GetWidget1ColorTransferFunction();
}

void QmitkToFVisualisationSettingsWidget::SetWidget1ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
{  
  this->m_ToFVisualizationFilter->SetWidget1ColorTransferFunction(colorTransferFunction);
}

vtkColorTransferFunction* QmitkToFVisualisationSettingsWidget::GetWidget2ColorTransferFunction()
{  
  return this->m_ToFVisualizationFilter->GetWidget2ColorTransferFunction();
}

void QmitkToFVisualisationSettingsWidget::SetWidget2ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
{  
  this->m_ToFVisualizationFilter->SetWidget2ColorTransferFunction(colorTransferFunction);
}

vtkColorTransferFunction* QmitkToFVisualisationSettingsWidget::GetWidget3ColorTransferFunction()
{  
  return this->m_ToFVisualizationFilter->GetWidget3ColorTransferFunction();
}

void QmitkToFVisualisationSettingsWidget::SetWidget3ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
{  
  this->m_ToFVisualizationFilter->SetWidget3ColorTransferFunction(colorTransferFunction);
}

//vtkColorTransferFunction* QmitkToFVisualisationSettingsWidget::GetWidget4ColorTransferFunction()
//{  
//  return m_Widget4ColorTransferFunction;
//}

//void QmitkToFVisualisationSettingsWidget::SetWidget4ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction)
//{  
//  m_Widget4ColorTransferFunction = colorTransferFunction;
//}

std::string QmitkToFVisualisationSettingsWidget::GetWidget1ImageType()
{
  return this->m_ToFVisualizationFilter->GetWidget1ImageType();
}

std::string QmitkToFVisualisationSettingsWidget::GetWidget2ImageType()
{  
  return this->m_ToFVisualizationFilter->GetWidget2ImageType();
}

std::string QmitkToFVisualisationSettingsWidget::GetWidget3ImageType()
{  
  return this->m_ToFVisualizationFilter->GetWidget3ImageType();
}

void QmitkToFVisualisationSettingsWidget::ComputeMinMax(float* data, int num, int& min, int& max)
{
  min = std::numeric_limits<int>::max();
  max = 0;

  for(int i=0; i<num; i++)
  {
    if (data[i] < min) min = data[i];
    if (data[i] > max) max = data[i];
  }

}

