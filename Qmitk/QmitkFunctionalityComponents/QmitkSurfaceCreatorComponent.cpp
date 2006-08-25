/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include "QmitkSurfaceCreatorComponent.h"
#include "QmitkSurfaceCreatorComponentGUI.h"

#include <QmitkDataTreeComboBox.h>
#include "QmitkCommonFunctionality.h"
#include "QmitkStdMultiWidget.h"

#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include <mitkColorSequenceRainbow.h>
#include <mitkManualSegmentationToSurfaceFilter.h>
//#include <mitkImageToSurfaceFilter.h>

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qapplication.h>
#include <qcursor.h>


/***************       CONSTRUCTOR      ***************/
QmitkSurfaceCreatorComponent::QmitkSurfaceCreatorComponent(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, bool updateSelector, bool showSelector, bool allowExpertMode):
m_Name(name),
m_MultiWidget(mitkStdMultiWidget),
m_DataTreeIteratorClone(NULL),
m_UpdateSelector(updateSelector),
m_ShowSelector(showSelector),
m_AllowExpertMode(allowExpertMode),
m_Active(false),
m_ShowExpertMode(true),
m_Median3DFlag(false),
m_InterpolateFlag(false),
m_SmoothFlag(false),
m_GaussFlag(false),
m_DecimateFlag(false),
m_GUI(NULL),
m_SelectedImage(NULL),
m_MitkImage(NULL),
m_MitkImageIterator(NULL),
m_SurfaceCounter(0),
m_r(0.75),
m_g(0.42),
m_b(-0.75), 
m_Threshold(1)
{
  SetDataTreeIterator(it);
  m_Color.Set(1.0, 0.67, 0.0);
  m_RainbowColor = new mitk::ColorSequenceRainbow();
}

/***************        DESTRUCTOR      ***************/
QmitkSurfaceCreatorComponent::~QmitkSurfaceCreatorComponent()
{

}

/*************** GET FUNCTIONALITY NAME ***************/
QString QmitkSurfaceCreatorComponent::GetFunctionalityName()
{
  return m_Name;
}

/*************** SET FUNCTIONALITY NAME ***************/
void QmitkSurfaceCreatorComponent::SetFunctionalityName(QString name)
{
  m_Name = name;
}

/*************** GET FUNCCOMPONENT NAME ***************/
QString QmitkSurfaceCreatorComponent::GetFunctionalityComponentName()
{
  return m_Name;
}

/*************** GET DATA TREE ITERATOR ***************/
mitk::DataTreeIteratorBase* QmitkSurfaceCreatorComponent::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkSurfaceCreatorComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/***************         GET GUI        ***************/
QWidget* QmitkSurfaceCreatorComponent::GetGUI()
{
  return m_GUI;
}

/*************** TREE CHANGED (       ) ***************/
void QmitkSurfaceCreatorComponent::TreeChanged()
{

}

/***************       CONNECTIONS      ***************/
void QmitkSurfaceCreatorComponent::CreateConnections()
{
  if ( m_GUI )
  {
    connect( (QObject*)(m_GUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));        
    connect( (QObject*)(m_GUI->GetSurfaceCreatorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowSurfaceCreatorContent(bool)));
    connect( (QObject*)(m_GUI->GetSurfaceCreatorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowCreateSurface(bool)));
    connect( (QObject*)(m_GUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool)));

    connect( (QObject*)(m_GUI->GetCreateSurfaceButton()), SIGNAL(pressed()), (QObject*) this, SLOT(CreateSurface()));
    connect( (QObject*)(m_GUI->GetThresholdLineEdit()), SIGNAL(returnPressed()), (QObject*) this, SLOT(CreateSurface()));
    connect( (QObject*)(m_GUI->GetShowExpertModeGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowExpertMode(bool)));
    connect( (QObject*)(m_GUI->GetShowSurfaceParameterGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowSurfaceParameter(bool)));

    connect( (QObject*)(m_GUI->GetMedian3DCheckBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetMedian3DFlag(bool)));
    connect( (QObject*)(m_GUI->GetInterpolateCheckBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetInterpolateFlag(bool)));
    connect( (QObject*)(m_GUI->GetSmoothCheckBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetSmoothFlag(bool)));
    connect( (QObject*)(m_GUI->GetGaussCheckBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetGaussFlag(bool)));
    connect( (QObject*)(m_GUI->GetDecimateCheckBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetDecimateFlag(bool)));
  }
}

/***************     IMAGE SELECTED     ***************/
void QmitkSurfaceCreatorComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedImage = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_GUI)
  {
    if(mitk::DataTreeFilter* filter = m_GUI->GetTreeNodeSelector()->GetFilter())
    {
      if(imageIt)
      {
        currentItem = const_cast <mitk::DataTreeFilter::Item*> ( filter->FindItem( imageIt->GetNode() ) );
      }
    }
  }
  if(currentItem)
  {
    currentItem->SetSelected(true);
  }
  if(m_GUI != NULL)
  {

    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      m_AddedChildList[i]->ImageSelected(m_SelectedImage);
    }
  }
  TreeChanged();

  if(m_GUI)
  {
    mitk::DataTreeFilter* filter = m_GUI->GetTreeNodeSelector()->GetFilter();
    m_MitkImageIterator = filter->GetIteratorToSelectedItem();

    if(m_MitkImageIterator.GetPointer())
    {
      m_MitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());
    }
  }
 m_SurfaceCounter = 0;
 m_GUI->GetReplaceExistingSurfaceCheckBox()->setChecked(false);

}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkSurfaceCreatorComponent::CreateContainerWidget(QWidget* parent)
{
  m_GUI = new QmitkSurfaceCreatorComponentGUI(parent);
  m_GUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());

  if(!m_ShowSelector)
  {
    m_GUI->GetSelectDataGroupBox()->setShown(false);
  }

  InitSurfaceGUI();
  ShowSurfaceParameter(false);

  InitSurfaceParamterFlags();

  if(!m_AllowExpertMode)
  {
   m_GUI->GetShowExpertModeGroupBox()->setCheckable(m_AllowExpertMode);
   m_GUI->GetShowExpertModeGroupBox()->setTitle("");
   m_GUI->GetShowExpertModeGroupBox()->setLineWidth(0);
  }

  return m_GUI;

}

/************** SET SELECTOR VISIBILITY ***************/
void QmitkSurfaceCreatorComponent::SetSelectorVisibility(bool visibility)
{
  if(m_GUI)
  {
    m_GUI->GetSelectDataGroupBox()->setShown(visibility);
  }
  m_ShowSelector = visibility;
}

void QmitkSurfaceCreatorComponent::SetExpertMode(bool visibility)
{
  if(m_AllowExpertMode)
  {
    m_ShowExpertMode = visibility;
    ShowExpertMode(visibility);
  }

}

/***************        ACTIVATED       ***************/
void QmitkSurfaceCreatorComponent::Activated()
{
  m_Active = true;
}

/***************       DEACTIVATED      ***************/
void QmitkSurfaceCreatorComponent::Deactivated()
{
  m_Active = false;
}

/**********************VISIBILITY************************************/

///************ SHOW SURFACE CREATOR CONTENT ************/
void QmitkSurfaceCreatorComponent::ShowSurfaceCreatorContent(bool)
{
  if(m_ShowSelector)
  {
    m_GUI->GetSelectDataGroupBox()->setShown(m_GUI->GetSurfaceCreatorGroupBox()->isChecked());
  }

  for(unsigned int i = 0;  i < m_ParameterList.size(); i++)
  {
    m_ParameterList[i]->setEnabled(true);
  }
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkSurfaceCreatorComponent::ShowImageContent(bool)
{
  m_GUI->GetImageContent()->setShown(m_GUI->GetSelectDataGroupBox()->isChecked());
}


///************* SHOW CREATE SRUFACE CONTENT ************/
void QmitkSurfaceCreatorComponent::ShowCreateSurface(bool)
{
  m_GUI->GetCreateSurfaceGroupBox()->setShown(m_GUI->GetSurfaceCreatorGroupBox()->isChecked());
}

void QmitkSurfaceCreatorComponent::ShowExpertMode(bool)
{
  if(m_AllowExpertMode)
  {
    if(m_ShowExpertMode)
    {
      for(unsigned int i = 0;  i < m_ExpertModeList.size(); i++)
      {
        m_ExpertModeList[i]->setShown(m_GUI->GetShowExpertModeGroupBox()->isChecked());
      }   
    }
  }
  else
  {
    for(unsigned int i = 0;  i < m_ExpertModeList.size(); i++)
    {
      m_ExpertModeList[i]->setShown(false);
    }
  }

  for(unsigned int i = 0;  i < m_ParameterList.size(); i++)
  {
    m_ParameterList[i]->setEnabled(true);
  }
}

///*************    SHOW SURFACE PARAMETER   ************/
void QmitkSurfaceCreatorComponent::ShowSurfaceParameter(bool)
{
  m_GUI->GetShowExpertModeGroupBox()->setShown(m_GUI->GetShowSurfaceParameterGroupBox()->isChecked());
  if(m_GUI->GetShowSurfaceParameterGroupBox()->isChecked() == true)
  {
    for(unsigned int i = 0;  i < m_ParameterList.size(); i++)
    {
      m_ParameterList[i]->setEnabled(true);
    }
  }
}

/****************************************************************************SURFACE CREATOR***************************************************************************/

///***************       SET THRESHOLD     **************/
void QmitkSurfaceCreatorComponent::SetThreshold(int threshold)
{
  m_Threshold = threshold;
  m_GUI->SetThreshold(threshold);
}

///***************       SET THRESHOLD     **************/
void QmitkSurfaceCreatorComponent::SetThreshold(const QString& threshold)
{
  m_GUI->SetThreshold(atoi(threshold));
}

///***************      INIT SURFACE GUI   **************/
void QmitkSurfaceCreatorComponent::InitSurfaceGUI()
{
  //Fill vector with all Widgets for the Parameters
  m_ParameterList.push_back(m_GUI->GetMedian3DCheckBox());
  m_ParameterList.push_back(m_GUI->GetInterpolateCheckBox());
  m_ParameterList.push_back(m_GUI->GetSmoothCheckBox());
  m_ParameterList.push_back(m_GUI->GetGaussCheckBox());
  m_ParameterList.push_back(m_GUI->GetDecimateCheckBox());

  for(unsigned int i = 0;  i < m_ParameterList.size(); i++)
  {
    m_ParameterList[i]->setEnabled(true);
  }

  //Fill vector with all widgets for the ExpertMode
  m_ExpertModeList.push_back(m_GUI->GetExpertModeMedianGroupBox());
  m_ExpertModeList.push_back(m_GUI->GetExpertModeInterpolateGoupBox());
  m_ExpertModeList.push_back(m_GUI->GetExpertModeSmoothGroupBox());
  m_ExpertModeList.push_back(m_GUI->GetExpertModeGaussGroupBox());
  m_ExpertModeList.push_back(m_GUI->GetExpertModeDecimateGroupBox());

  for(unsigned int i = 0;  i < m_ExpertModeList.size(); i++)
  {
    m_ExpertModeList[i]->setEnabled(true);
    m_ExpertModeList[i]->setShown(m_GUI->GetShowExpertModeGroupBox()->isChecked());
  }
}

void QmitkSurfaceCreatorComponent::SetMedian3DFlag(bool flag)
{
  m_Median3DFlag = flag;
}

void QmitkSurfaceCreatorComponent::SetInterpolateFlag(bool flag)
{
  m_InterpolateFlag = flag;
}

void QmitkSurfaceCreatorComponent::SetSmoothFlag(bool flag)
{
  m_SmoothFlag = flag;
}

void QmitkSurfaceCreatorComponent::SetGaussFlag(bool flag)
{
  m_GaussFlag = flag;
}

void QmitkSurfaceCreatorComponent::SetDecimateFlag(bool flag)
{
  m_DecimateFlag = flag;
}



void QmitkSurfaceCreatorComponent::InitSurfaceParamterFlags()
{ 
  if(m_GUI)
  {
    m_Median3DFlag = m_GUI->GetMedian3DCheckBox()->isChecked();
    m_InterpolateFlag = m_GUI->GetInterpolateCheckBox()->isChecked();
    m_SmoothFlag = m_GUI->GetSmoothCheckBox()->isChecked();
    m_GaussFlag = m_GUI->GetGaussCheckBox()->isChecked();
    m_DecimateFlag = m_GUI->GetDecimateCheckBox()->isChecked();
  }
}

void QmitkSurfaceCreatorComponent::GetMedian3DValue(int& x, int& y, int& z)
{
  m_GUI->GetMedian3DValue(x, y, z);
}

void QmitkSurfaceCreatorComponent::GetInterpolateValue(int& x, int& y, int& z)
{
  m_GUI->GetInterpolationValue(x, y, z);
}

int QmitkSurfaceCreatorComponent::GetSmoothIterationValue()
{
  int iteration;
  iteration = m_GUI->GetSmoothIterationValue();

 return iteration;
}

float QmitkSurfaceCreatorComponent::GetSmoothRelaxationValue()
{
  float relaxation;
  relaxation = m_GUI->GetSmoothRelaxationValue();

  return relaxation;
}

float QmitkSurfaceCreatorComponent::GetGaussValue()
{
  float gsDev;
  gsDev = m_GUI->GetGaussValue();

  return gsDev;
}

float QmitkSurfaceCreatorComponent::GetDecimateValue()
{
  //Value for DecimatePro 
  float targetReduction;
  targetReduction = m_GUI->GetDecimateValue();

  return targetReduction;
}

///***************      CREATE SURFACE     **************/
void QmitkSurfaceCreatorComponent::CreateSurface()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  if(m_MitkImage != NULL)
  {
    //call the function to get the Median 3D-Values from GUI
    int median3DXValue;
    int median3DYValue;
    int median3DZValue;
    GetMedian3DValue(median3DXValue, median3DYValue, median3DZValue);

    //call the function to get the Interpolation-Values from GUI
    int interpolationXValue;
    int interpolationYValue;
    int interpolationZValue;
    GetInterpolateValue(interpolationXValue, interpolationYValue, interpolationZValue);

    //Get other values from GUI
    int smoothIterations = GetSmoothIterationValue();
    float smoothRelaxation = GetSmoothRelaxationValue();
    float gauss = GetGaussValue();
    float decimate = GetDecimateValue();

    //ImageToSurface Instance
    mitk::DataTreeIteratorClone  iteratorOnImageToBeSkinExtracted;
    if(m_MitkImageIterator.IsNotNull())
    {

      iteratorOnImageToBeSkinExtracted = m_MitkImageIterator;
    }
    else 
    {
      iteratorOnImageToBeSkinExtracted = CommonFunctionality::GetIteratorToFirstImage(m_DataTreeIterator.GetPointer());
    }
    
    //create surface-object, called filter
    mitk::ManualSegmentationToSurfaceFilter::Pointer filter = mitk::ManualSegmentationToSurfaceFilter::New();

    if (filter.IsNull())
    {
      std::cout<<"NULL Pointer for ManualSegmentationToSurfaceFilter"<<std::endl;
      return;
    }

    //image from that the surface shall be created
    filter->SetInput( m_MitkImage ); 

    //GAUSS FILTER (must be set befor threshold)
    filter->SetUseStandardDeviation( m_GaussFlag );
    if(m_GaussFlag)
    {
      filter->SetStandardDeviation( gauss );
    }

    //THRESHOLD FROM GUI 
    filter->SetThreshold( m_GUI->GetThreshold()); 

    //MEDIAN 3D FILTER
    filter->SetMedianFilter3D(m_Median3DFlag);
    if(m_Median3DFlag)
    {
      filter->SetMedianKernelSize(median3DXValue, median3DYValue, median3DZValue);
    }

    //INTERFOLATION FILTER
    if(m_InterpolateFlag)
    {
      filter->SetInterpolation(interpolationXValue, interpolationYValue, interpolationZValue);
    }
    filter->SetInterpolation(m_InterpolateFlag);
 
    //SMOOTH FILTER
    if(m_SmoothFlag)
    {
      filter->SetSmoothIteration(smoothIterations);
      filter->SetSmoothRelaxation(smoothRelaxation);
    }
    filter->SetSmooth(m_SmoothFlag);
    

    //DECIMATE FILTER
    if(m_DecimateFlag)
    {
      filter->SetTargetReduction( decimate );
      filter->SetDecimate(mitk::ImageToSurfaceFilter::DecimatePro);
    }
    else
    {
      filter->SetDecimate(mitk::ImageToSurfaceFilter::NoDecimation );
    }


    InsertSurfaceIntoDataTree(filter, iteratorOnImageToBeSkinExtracted);
    

  }//if m_MitkImage != NULL

  QApplication::restoreOverrideCursor();
}

void QmitkSurfaceCreatorComponent::InsertSurfaceIntoDataTree(mitk::ManualSegmentationToSurfaceFilter * ft, mitk::DataTreeIteratorClone  iT)
{

    mitk::ManualSegmentationToSurfaceFilter::Pointer filter = ft;
    mitk::DataTreeIteratorClone iteratorOnImageToBeSkinExtracted = iT;

    mitk::DataTreeNode::Pointer surfaceNode = mitk::DataTreeNode::New(); 
    surfaceNode->SetData( filter->GetOutput() );

    int layer = 0;


    ++m_SurfaceCounter;


    std::ostringstream buffer;
    QString surfaceDataName = m_GUI->GetTreeNodeSelector()->currentText();
    buffer << surfaceDataName.ascii();
    buffer << "_";
    buffer << m_SurfaceCounter;
    
    std::string surfaceNodeName = "Surface_" + buffer.str();

    m_r += 0.25; if(m_r > 1){m_r = m_r - 1;}
    m_g += 0.25; if(m_g > 1){m_g = m_g - 1;}
    m_b += 0.25; if(m_b > 1){m_b = m_b - 1;}

    iteratorOnImageToBeSkinExtracted->Get()->GetIntProperty("layer", layer);
    mitk::DataTreeNodeFactory::SetDefaultSurfaceProperties(surfaceNode);
    surfaceNode->SetIntProperty("layer", layer+1);
    surfaceNode->SetProperty("Surface", new mitk::BoolProperty(true));
    surfaceNode->SetProperty("name", new mitk::StringProperty(surfaceNodeName));


    mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
    bool isSurface = false;

    while(!(iteratorClone->IsAtEnd())&&(isSurface == false))
    {
      iteratorClone->Get()->GetBoolProperty("Surface", isSurface);
      if(isSurface == false)
      {
        ++iteratorClone;
      }
    }
    iteratorClone= iteratorOnImageToBeSkinExtracted; 

    if(!(m_GUI->GetReplaceExistingSurfaceCheckBox()->isChecked()))
    {
      iteratorClone->Add(surfaceNode);
      isSurface = true;
    }

    else if(m_GUI->GetReplaceExistingSurfaceCheckBox()->isChecked())
    {
      if(isSurface)
      {
        ++iteratorClone;
        bool stillIsSurface = false;
        if((!iteratorClone->IsAtEnd()) && (iteratorClone->Get().IsNotNull()))
        {
        iteratorClone->Get()->GetBoolProperty("Surface", stillIsSurface);
        }
        if(stillIsSurface)
        {
          iteratorClone->Set(surfaceNode);
        }
      }
    }

    m_GUI->GetReplaceExistingSurfaceCheckBox()->setEnabled(isSurface);

    iteratorClone->GetTree()->Modified();


    mitk::Surface::Pointer surface = filter->GetOutput();

    //to show surfaceContur
    m_Color = m_RainbowColor->GetNextColor();
    surfaceNode->SetColor(m_Color);
    surfaceNode->SetVisibility(true);

    m_MultiWidget->RequestUpdate();
}
