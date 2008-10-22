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
#include "QmitkSurfaceCreatorComponent.h"
#include "QmitkSurfaceCreatorComponentGUI.h"

#include <QmitkDataTreeComboBox.h>
#include "QmitkCommonFunctionality.h"
#include "QmitkStdMultiWidget.h"

#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"
#include <mitkColorSequenceRainbow.h>
#include <mitkManualSegmentationToSurfaceFilter.h>
//#include <mitkImageToSurfaceFilter.h>

#include "QmitkSelectableGLWidget.h"

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qmessagebox.h>


/***************       CONSTRUCTOR      ***************/
QmitkSurfaceCreatorComponent::QmitkSurfaceCreatorComponent(QObject * parent, const char * parentName, bool updateSelector, bool showSelector, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, bool allowExpertMode)
: QmitkFunctionalityComponentContainer(parent, parentName, updateSelector, showSelector),
m_MultiWidget(mitkStdMultiWidget),
m_DataTreeIteratorClone(NULL),
m_SurfaceCreatorComponentGUI(NULL),
m_SurfaceNode(NULL)
{
  SetDataTreeIterator(it);
  SetComponentName("SurfaceCreator");

  m_Color.Set(1.0, 0.67, 0.0);
  m_RainbowColor = new mitk::ColorSequenceRainbow();
  m_AllowExpertMode = allowExpertMode;
  m_ShowExpertMode = true;
  m_Median3DFlag = false;
  m_InterpolateFlag = false;
  m_SmoothFlag = false;
  m_GaussFlag = false;
  m_DecimateFlag = false;
  m_MitkImage = NULL;
  m_MitkImageIterator = NULL;
  m_SurfaceCounter = 0;
  m_r = 0.75;
  m_g = 0.42;
  m_b = -0.75; 
  m_Threshold = 1;
}

/***************        DESTRUCTOR      ***************/
QmitkSurfaceCreatorComponent::~QmitkSurfaceCreatorComponent()
{

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

 /***************   GET IMAGE CONTENT   ***************/
QGroupBox* QmitkSurfaceCreatorComponent::GetImageContent()
{
  return (QGroupBox*) m_SurfaceCreatorComponentGUI->GetImageContent();
}

/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkSurfaceCreatorComponent::GetTreeNodeSelector()
{
  return m_SurfaceCreatorComponentGUI->GetTreeNodeSelector();
}

/***************       CONNECTIONS      ***************/
void QmitkSurfaceCreatorComponent::CreateConnections()
{
  {
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));        
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetSurfaceCreatorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowSurfaceCreatorContent(bool)));
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetSurfaceCreatorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowCreateSurface(bool)));
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool)));

    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetCreateSurfaceButton()), SIGNAL(released()), (QObject*) this, SLOT(CreateSurface()));
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetThresholdLineEdit()), SIGNAL(returnPressed()), (QObject*) this, SLOT(CreateSurface()));
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetShowExpertModeGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowExpertMode(bool)));
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetShowSurfaceParameterGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowSurfaceParameter(bool)));

    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetMedian3DCheckBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetMedian3DFlag(bool)));
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetInterpolateCheckBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetInterpolateFlag(bool)));
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetSmoothCheckBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetSmoothFlag(bool)));
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetGaussCheckBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetGaussFlag(bool)));
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetDecimateCheckBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetDecimateFlag(bool)));

    //to connect the toplevel checkable GroupBox with the method SetContentContainerVisibility to inform all containing komponent to shrink or to expand
    connect( (QObject*)(m_SurfaceCreatorComponentGUI->GetSurfaceCreatorGroupBox()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 
    connect((QObject*)(m_SurfaceCreatorComponentGUI->GetShowSurfaceContourCheckBox()), SIGNAL(clicked()), this, SLOT(ShowSurfaceContour()) );
  }
}

void QmitkSurfaceCreatorComponent::ImageSelected()
{
    const mitk::DataTreeFilter::Item* imageIt = m_SurfaceCreatorComponentGUI->GetTreeNodeSelector()->GetFilter()->GetSelectedItem();
    m_SelectedItem = imageIt;
    mitk::DataTreeFilter::Item* currentItem(NULL);
    if(m_SurfaceCreatorComponentGUI)
    {
        if(mitk::DataTreeFilter* filter = m_SurfaceCreatorComponentGUI->GetTreeNodeSelector()->GetFilter())
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
    if(m_SurfaceCreatorComponentGUI != NULL)
    {
        for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
        {
            QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
            if (functionalityComponent != NULL)
                functionalityComponent->ImageSelected(m_SelectedItem);
        }
    }
   

    if(m_SurfaceCreatorComponentGUI)
    {
        mitk::DataTreeFilter* filter = m_SurfaceCreatorComponentGUI->GetTreeNodeSelector()->GetFilter();
        m_MitkImageIterator = filter->GetIteratorToSelectedItem();

        if(m_MitkImageIterator.GetPointer())
        {
            m_MitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());
        }
    }
    m_SurfaceCounter = 0;
    m_SurfaceCreatorComponentGUI->GetReplaceExistingSurfaceCheckBox()->setChecked(false);

}

/***************     IMAGE SELECTED     ***************/
void QmitkSurfaceCreatorComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedItem = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_SurfaceCreatorComponentGUI)
  {
    if(mitk::DataTreeFilter* filter = m_SurfaceCreatorComponentGUI->GetTreeNodeSelector()->GetFilter())
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
  if(m_SurfaceCreatorComponentGUI != NULL)
  {
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        functionalityComponent->ImageSelected(m_SelectedItem);
    }
  }
  TreeChanged();

  if(m_SurfaceCreatorComponentGUI)
  {
    mitk::DataTreeFilter* filter = m_SurfaceCreatorComponentGUI->GetTreeNodeSelector()->GetFilter();
    m_MitkImageIterator = filter->GetIteratorToSelectedItem();

    if(m_MitkImageIterator.GetPointer())
    {
      m_MitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());
    }
  }
 m_SurfaceCounter = 0;
 m_SurfaceCreatorComponentGUI->GetReplaceExistingSurfaceCheckBox()->setChecked(false);

}

void QmitkSurfaceCreatorComponent::TreeChanged()
{
	UpdateDataTreeComboBoxes();

	for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
	{
		m_AddedChildList[i]->TreeChanged();
	} 

    ImageSelected();
	//if(m_MitkImageIterator.GetPointer())
	//{
	//	if(m_MitkImageIterator->Get())
	//	{
	//		m_MitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());
	//	}
	//}
}

/***************  CREATE CONTROL WIDGET  **************/
QWidget* QmitkSurfaceCreatorComponent::CreateControlWidget(QWidget* parent)
{
  m_SurfaceCreatorComponentGUI = new QmitkSurfaceCreatorComponentGUI(parent);
  m_GUI = m_SurfaceCreatorComponentGUI;

  m_SurfaceCreatorComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());

 	if(m_ShowSelector)
	{
		m_SurfaceCreatorComponentGUI->GetImageContent()->setShown(m_SurfaceCreatorComponentGUI->GetSelectDataGroupBox()->isChecked());
	}
	else
	{
		m_SurfaceCreatorComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
	}

  m_SurfaceCreatorComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("isComponentThresholdImage"));
  InitSurfaceGUI();
  ShowSurfaceParameter(false);

  InitSurfaceParamterFlags();

  if(!m_AllowExpertMode)
  {
   m_SurfaceCreatorComponentGUI->GetShowExpertModeGroupBox()->setCheckable(m_AllowExpertMode);
   m_SurfaceCreatorComponentGUI->GetShowExpertModeGroupBox()->setTitle("");
   m_SurfaceCreatorComponentGUI->GetShowExpertModeGroupBox()->setLineWidth(0);
  }
  return m_SurfaceCreatorComponentGUI;

}

/************** SET SELECTOR VISIBILITY ***************/
void QmitkSurfaceCreatorComponent::SetSelectorVisibility(bool /*visibility*/)
{
  if(m_SurfaceCreatorComponentGUI)
  {
    m_SurfaceCreatorComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
  }
  //m_ShowSelector = visibility;
}

void QmitkSurfaceCreatorComponent::SetExpertMode(bool visibility)
{
  if(m_AllowExpertMode)
  {
    m_ShowExpertMode = visibility;
    ShowExpertMode(visibility);
  }

}

///************ SHOW SURFACE CREATOR CONTENT ************/
void QmitkSurfaceCreatorComponent::ShowSurfaceCreatorContent(bool)
{
  if(m_ShowSelector)
  {
    m_SurfaceCreatorComponentGUI->GetSelectDataGroupBox()->setShown(m_SurfaceCreatorComponentGUI->GetSurfaceCreatorGroupBox()->isChecked());
  }
  else
  {
   m_SurfaceCreatorComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
  }

  for(unsigned int i = 0;  i < m_ParameterList.size(); i++)
  {
    m_ParameterList[i]->setEnabled(true);
  }
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkSurfaceCreatorComponent::GetContentContainer()
{
 return m_SurfaceCreatorComponentGUI->GetCreateSurfaceGroupBox();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkSurfaceCreatorComponent::GetMainCheckBoxContainer()
{
 return m_SurfaceCreatorComponentGUI->GetSurfaceCreatorGroupBox();
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkSurfaceCreatorComponent::ShowImageContent(bool)
{
	if(m_ShowSelector)
	{
		m_SurfaceCreatorComponentGUI->GetImageContent()->setShown(m_SurfaceCreatorComponentGUI->GetSelectDataGroupBox()->isChecked());
	}
	else
	{
		m_SurfaceCreatorComponentGUI->GetImageContent()->setShown(m_ShowSelector);
	}


}


///************* SHOW CREATE SRUFACE CONTENT ************/
void QmitkSurfaceCreatorComponent::ShowCreateSurface(bool)
{
  m_SurfaceCreatorComponentGUI->GetCreateSurfaceGroupBox()->setShown(m_SurfaceCreatorComponentGUI->GetSurfaceCreatorGroupBox()->isChecked());
}

void QmitkSurfaceCreatorComponent::ShowExpertMode(bool)
{
  if(m_AllowExpertMode)
  {
    if(m_ShowExpertMode)
    {
      for(unsigned int i = 0;  i < m_ExpertModeList.size(); i++)
      {
        m_ExpertModeList[i]->setShown(m_SurfaceCreatorComponentGUI->GetShowExpertModeGroupBox()->isChecked());
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
  m_SurfaceCreatorComponentGUI->GetShowExpertModeGroupBox()->setShown(m_SurfaceCreatorComponentGUI->GetShowSurfaceParameterGroupBox()->isChecked());
  if(m_SurfaceCreatorComponentGUI->GetShowSurfaceParameterGroupBox()->isChecked() == true)
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
  m_SurfaceCreatorComponentGUI->SetThreshold(threshold);
}

///***************       SET THRESHOLD     **************/
void QmitkSurfaceCreatorComponent::SetThreshold(const QString& threshold)
{
  m_SurfaceCreatorComponentGUI->SetThreshold(atoi(threshold));
}

///***************      INIT SURFACE GUI   **************/
void QmitkSurfaceCreatorComponent::InitSurfaceGUI()
{
  //Fill vector with all Widgets for the Parameters
  m_ParameterList.push_back(m_SurfaceCreatorComponentGUI->GetMedian3DCheckBox());
  m_ParameterList.push_back(m_SurfaceCreatorComponentGUI->GetInterpolateCheckBox());
  m_ParameterList.push_back(m_SurfaceCreatorComponentGUI->GetSmoothCheckBox());
  m_ParameterList.push_back(m_SurfaceCreatorComponentGUI->GetGaussCheckBox());
  m_ParameterList.push_back(m_SurfaceCreatorComponentGUI->GetDecimateCheckBox());

  for(unsigned int i = 0;  i < m_ParameterList.size(); i++)
  {
    m_ParameterList[i]->setEnabled(true);
  }

  //Fill vector with all widgets for the ExpertMode
  m_ExpertModeList.push_back(m_SurfaceCreatorComponentGUI->GetExpertModeMedianGroupBox());
  m_ExpertModeList.push_back(m_SurfaceCreatorComponentGUI->GetExpertModeInterpolateGoupBox());
  m_ExpertModeList.push_back(m_SurfaceCreatorComponentGUI->GetExpertModeSmoothGroupBox());
  m_ExpertModeList.push_back(m_SurfaceCreatorComponentGUI->GetExpertModeGaussGroupBox());
  m_ExpertModeList.push_back(m_SurfaceCreatorComponentGUI->GetExpertModeDecimateGroupBox());

  for(unsigned int i = 0;  i < m_ExpertModeList.size(); i++)
  {
    m_ExpertModeList[i]->setEnabled(true);
    m_ExpertModeList[i]->setShown(m_SurfaceCreatorComponentGUI->GetShowExpertModeGroupBox()->isChecked());
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
  if(m_SurfaceCreatorComponentGUI)
  {
    m_Median3DFlag = m_SurfaceCreatorComponentGUI->GetMedian3DCheckBox()->isChecked();
    m_InterpolateFlag = m_SurfaceCreatorComponentGUI->GetInterpolateCheckBox()->isChecked();
    m_SmoothFlag = m_SurfaceCreatorComponentGUI->GetSmoothCheckBox()->isChecked();
    m_GaussFlag = m_SurfaceCreatorComponentGUI->GetGaussCheckBox()->isChecked();
    m_DecimateFlag = m_SurfaceCreatorComponentGUI->GetDecimateCheckBox()->isChecked();
  }
}


///***************         SET VALUES      **************/
void QmitkSurfaceCreatorComponent::SetMedian3DValue(int x, int y, int z)
{
 m_SurfaceCreatorComponentGUI->SetMedian3DValue(x, y, z);
}

void QmitkSurfaceCreatorComponent::SetInterpolateValue(int x, int y, int z)
{
  m_SurfaceCreatorComponentGUI->SetInterpolationValue(x, y, z);
}

void QmitkSurfaceCreatorComponent::SetSmoothValue(int iteration, float relaxation)
{
  m_SurfaceCreatorComponentGUI->SetSmoothValue(iteration, relaxation);
}

void QmitkSurfaceCreatorComponent::SetSmoothIterationValue(int iteration)
{
  m_SurfaceCreatorComponentGUI->SetSmoothIterationValue(iteration);
}

void QmitkSurfaceCreatorComponent::SetSmoothRelaxationValue(float relaxation)
{
  m_SurfaceCreatorComponentGUI->SetSmoothRelaxationValue(relaxation);
}

void QmitkSurfaceCreatorComponent::SetGaussValue(float gsDev)
{
 m_SurfaceCreatorComponentGUI->SetGaussValue(gsDev);
}

void QmitkSurfaceCreatorComponent::SetDecimateValue(float targetReduction)
{
  m_SurfaceCreatorComponentGUI->SetDecimateValue(targetReduction);
}

///***************   GET VALUES FROM GUI   **************/


void QmitkSurfaceCreatorComponent::GetMedian3DValue(int& x, int& y, int& z)
{
 m_SurfaceCreatorComponentGUI->GetMedian3DValue(x, y, z);
}

void QmitkSurfaceCreatorComponent::GetInterpolateValue(int& x, int& y, int& z)
{
  m_SurfaceCreatorComponentGUI->GetInterpolationValue(x, y, z);
}

int QmitkSurfaceCreatorComponent::GetSmoothIterationValue()
{
  int iteration;
  iteration = m_SurfaceCreatorComponentGUI->GetSmoothIterationValue();

 return iteration;
}

float QmitkSurfaceCreatorComponent::GetSmoothRelaxationValue()
{
  float relaxation;
  relaxation = m_SurfaceCreatorComponentGUI->GetSmoothRelaxationValue();

  return relaxation;
}

float QmitkSurfaceCreatorComponent::GetGaussValue()
{
  float gsDev;
  gsDev = m_SurfaceCreatorComponentGUI->GetGaussValue();

  return gsDev;
}

float QmitkSurfaceCreatorComponent::GetDecimateValue()
{
  //Value for DecimatePro 
  float targetReduction;
  targetReduction = m_SurfaceCreatorComponentGUI->GetDecimateValue();

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
    filter->SetUseGaussianImageSmooth( m_GaussFlag );
    if(m_GaussFlag)
    {
      filter->SetGaussianStandardDeviation( gauss );
    }

    //THRESHOLD FROM GUI 
    filter->SetThreshold( m_SurfaceCreatorComponentGUI->GetThreshold()); 

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

    int numOfPolys = filter->GetOutput()->GetVtkPolyData()->GetNumberOfPolys();
    if(numOfPolys>2000000)
    {
      QApplication::restoreOverrideCursor();
      if(QMessageBox::question(NULL, "CAUTION!!!", "The number of polygons is greater than 2 000 000. If you continue, the program might crash. How do you want to go on?", "Proceed anyway!", "Cancel immediately! (maybe you want to insert an other threshold)!",QString::null,0 ,1)==1)
      {

        return;
      }
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    }

    mitk::DataTreeNode::Pointer surfaceNode = mitk::DataTreeNode::New(); 
    m_SurfaceNode = surfaceNode;
    surfaceNode->SetData( filter->GetOutput() );

    int layer = 0;


    ++m_SurfaceCounter;


    std::ostringstream buffer;
    QString surfaceDataName = m_SurfaceCreatorComponentGUI->GetTreeNodeSelector()->currentText();
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
    surfaceNode->SetProperty("Surface", mitk::BoolProperty::New(true));
    surfaceNode->SetProperty("name", mitk::StringProperty::New(surfaceNodeName));


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

    if(!(m_SurfaceCreatorComponentGUI->GetReplaceExistingSurfaceCheckBox()->isChecked()))
    {
      iteratorClone->Add(surfaceNode);
      isSurface = true;
    }

    else if(m_SurfaceCreatorComponentGUI->GetReplaceExistingSurfaceCheckBox()->isChecked())
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

    m_SurfaceCreatorComponentGUI->GetReplaceExistingSurfaceCheckBox()->setEnabled(isSurface);

    iteratorClone->GetTree()->Modified();


    mitk::Surface::Pointer surface = filter->GetOutput();

    //to show surfaceContur
    m_Color = m_RainbowColor->GetNextColor();
    surfaceNode->SetColor(m_Color);
    surfaceNode->SetVisibility(true);
    ShowSurfaceContour();
    m_MultiWidget->RequestUpdate();
}

mitk::DataTreeNode::Pointer QmitkSurfaceCreatorComponent::getSurfaceNode()
{
  return m_SurfaceNode;
}//end of getSurfaceNode()

//*****************************************SHOW SURFACE CONTUR******************************
//
void QmitkSurfaceCreatorComponent::ShowSurfaceContour()
{  
  if(m_SurfaceCreatorComponentGUI->GetShowSurfaceContourCheckBox()->isChecked() == true)
  {
    if(getSurfaceNode())
    {
      getSurfaceNode()->SetVisibility(true);
    }
  }

  if(m_SurfaceCreatorComponentGUI->GetShowSurfaceContourCheckBox()->isChecked()== false)
  {
    if(getSurfaceNode())
    {
      getSurfaceNode()->SetVisibility(false, NULL);
      getSurfaceNode()->SetVisibility( true, mitk::BaseRenderer::GetInstance( m_MultiWidget->mitkWidget4->GetRenderWindow() ) );
    }
  }
  m_MultiWidget->RequestUpdate();
}//end of ShowSurfaceContur



