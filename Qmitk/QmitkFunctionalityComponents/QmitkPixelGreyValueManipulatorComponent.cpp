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
#include "QmitkPixelGreyValueManipulatorComponent.h"
#include "QmitkPixelGreyValueManipulatorComponentGUI.h"

#include <QmitkDataTreeComboBox.h>
#include <QmitkPointListWidget.h>//for the PointWidget

#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"

#include <mitkImageAccessByItk.h>

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcursor.h>
#include <qapplication.h>


/***************       CONSTRUCTOR      ***************/
QmitkPixelGreyValueManipulatorComponent::QmitkPixelGreyValueManipulatorComponent(QObject *parent, const char *parentName, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, bool updateSelector, bool showSelector):
QmitkFunctionalityComponentContainer(parent, parentName),
m_ParentName(parentName),
m_ComponentName("SurfaceCreator"),
m_MultiWidget(mitkStdMultiWidget),
m_DataTreeIteratorClone(NULL),
m_UpdateSelector(updateSelector),
m_ShowSelector(showSelector),
m_Active(false),
m_PixelGreyValueManipulatorComponentGUI(NULL),
m_SelectedImage(NULL),
m_Spacer(NULL)
{
  SetDataTreeIterator(it);
  m_MitkImage = NULL;
  m_MitkImageIterator = NULL;
  m_Segmentation = NULL; 
  m_PixelChangedImage = NULL;
  m_PixelChangedImageNode = NULL;
  m_SegmentedShiftResultNode = NULL;
  m_ItNewBuildSeg = NULL;
  m_SegmentationNode = NULL;
  m_PixelChangedImageCounter = NULL;
  m_ManipulationMode = 0;
  m_ManipulationArea = 0;
}

/***************        DESTRUCTOR      ***************/
QmitkPixelGreyValueManipulatorComponent::~QmitkPixelGreyValueManipulatorComponent()
{

}

/*************** GET FUNCTIONALITY NAME ***************/
QString QmitkPixelGreyValueManipulatorComponent::GetFunctionalityName()
{
  return m_ParentName;
}

/*************** SET FUNCTIONALITY NAME ***************/
void QmitkPixelGreyValueManipulatorComponent::SetFunctionalityName(QString parentName)
{
  m_ParentName = parentName;
}

/***************   GET COMPONENT NAME   ***************/
QString QmitkPixelGreyValueManipulatorComponent::GetComponentName()
{
  return m_ComponentName;
}

/*************** GET DATA TREE ITERATOR ***************/
mitk::DataTreeIteratorBase* QmitkPixelGreyValueManipulatorComponent::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkPixelGreyValueManipulatorComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/***************         GET GUI        ***************/
QWidget* QmitkPixelGreyValueManipulatorComponent::GetGUI()
{
  return m_PixelGreyValueManipulatorComponentGUI;
}

/*************** TREE CHANGED (       ) ***************/
void QmitkPixelGreyValueManipulatorComponent::TreeChanged()
{
  if(m_PixelGreyValueManipulatorComponentGUI)
  {
    m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->Update();
    m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->Update();

    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
    {
      m_AddedChildList[i]->TreeChanged();
    } 
  }
}

/***************       CONNECTIONS      ***************/
void QmitkPixelGreyValueManipulatorComponent::CreateConnections()
{
  if ( m_PixelGreyValueManipulatorComponentGUI )
  {
    connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));        
    connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(SegmentationSelected(const mitk::DataTreeFilter::Item *)));        


    connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetPixelGreyValueManipulatorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowPixelGreyValueManipulatorContent(bool)));
    connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool)));

    //Button "create new manipulated image" pressed
    connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetCreateNewManipulatedImageButton()), SIGNAL(clicked()), (QObject*) this, SLOT(PipelineControllerToCreateManipulatedImage()));
    connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()), SIGNAL(activated (int)), (QObject*) this, SLOT(HideOrShowValue2(int)));    
  }
}

/***************     IMAGE SELECTED     ***************/
void QmitkPixelGreyValueManipulatorComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedImage = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_PixelGreyValueManipulatorComponentGUI)
  {
    if(mitk::DataTreeFilter* filter = m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->GetFilter())
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
  if(m_PixelGreyValueManipulatorComponentGUI)
  {
    mitk::DataTreeFilter* filter = m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->GetFilter();
    m_MitkImageIterator = filter->GetIteratorToSelectedItem();

    if(m_MitkImageIterator.GetPointer())
    {
      m_MitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());
    }
  }

  if(m_PixelGreyValueManipulatorComponentGUI != NULL)
  {

    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      m_AddedChildList[i]->ImageSelected(m_SelectedImage);
    }
  }
  TreeChanged();
}

/*************** SEGMENTATION SELECTED  ***************/
void QmitkPixelGreyValueManipulatorComponent::SegmentationSelected(const mitk::DataTreeFilter::Item * segmentationIt)
{
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_PixelGreyValueManipulatorComponentGUI)
  {
    if(mitk::DataTreeFilter* filter = m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter())
    {
      if(segmentationIt)
      {
        currentItem = const_cast <mitk::DataTreeFilter::Item*> ( filter->FindItem( segmentationIt->GetNode() ) );
      }
    }
  }
  if(currentItem)
  {
    currentItem->SetSelected(true);
  }

  if(m_PixelGreyValueManipulatorComponentGUI)
  {
    mitk::DataTreeFilter* filter = m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter();

    mitk::DataTreeIteratorClone segmentationIterator = filter->GetIteratorToSelectedItem();

    if(segmentationIterator.GetPointer())
    {
      m_Segmentation = static_cast<mitk::Image*> (segmentationIterator->Get()->GetData());
    }
  }

}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkPixelGreyValueManipulatorComponent::CreateContainerWidget(QWidget* parent)
{
  m_PixelGreyValueManipulatorComponentGUI = new QmitkPixelGreyValueManipulatorComponentGUI(parent);
  m_GUI = m_PixelGreyValueManipulatorComponentGUI;
  m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
  m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->SetDataTree(GetDataTreeIterator());

  //m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->insertItem("-- Segmentation / Mask --", 0);

  if(!m_ShowSelector)
  {
    m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()->setShown(false);
  }
  m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("isComponentThresholdImage"));
  m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithBoolProperty<mitk::Image>("segmentation"));
  //CreateConnections();

  return m_PixelGreyValueManipulatorComponentGUI;

}

/************** SET SELECTOR VISIBILITY ***************/
void QmitkPixelGreyValueManipulatorComponent::SetSelectorVisibility(bool visibility)
{
  if(m_PixelGreyValueManipulatorComponentGUI)
  {
    m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()->setShown(visibility);
  }
  m_ShowSelector = visibility;
}


/***************        ACTIVATED       ***************/
void QmitkPixelGreyValueManipulatorComponent::Activated()
{
  m_Active = true;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Activated();
  } 



  //BEGIN ONLY FOR SEEDPOINTS******************************************************************************************************************************************
  if (m_SeedPointSetNode.IsNull())
  {
    //SeedPoints are to define the two Points for the ThresholdGradient
    //add Point with crtl + leftMouseButton
    m_Seeds = mitk::PointSet::New();

    m_SeedPointSetNode = mitk::DataTreeNode::New();
    m_SeedPointSetNode->SetData(m_Seeds);
    mitk::ColorProperty::Pointer color = new mitk::ColorProperty(0.2, 0.0, 0.8);
    mitk::Point3D colorTwo; 
    mitk::FillVector3D(colorTwo, 0.2, 0.0, 0.8);
    m_PixelGreyValueManipulatorComponentGUI->GetQmitkPointListWidget()->SwitchInteraction(&m_SeedPointSetInteractor, &m_SeedPointSetNode, 2, colorTwo,"SeedPoints ");  //-1 for unlimited points
    m_SeedPointSetNode->SetProperty("color",color);
    m_SeedPointSetNode->SetIntProperty("layer", 101);
    m_SeedPointSetNode->SetProperty("name", new mitk::StringProperty("SeedPoints"));

    m_SeedPointSetInteractor = new mitk::PointSetInteractor("seedpointsetinteractor", m_SeedPointSetNode, 2);

    m_DataTreeIterator.GetPointer()->Add(m_SeedPointSetNode);

    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_SeedPointSetInteractor);
  }
  else 
  {
    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_SeedPointSetInteractor);
  }
  //END ONLY FOR SEEDPOINTS******************************************************************************************************************************************

}

/***************       DEACTIVATED      ***************/
void QmitkPixelGreyValueManipulatorComponent::Deactivated()
{
  m_Active = false;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Deactivated();
  } 

  //BEGIN ONLY FOR SEEDPOINTS******************************************************************************************************************************************
  //deactivate m_SeedPointSetNode when leaving SurfaceCreator
  if (m_SeedPointSetNode.IsNotNull())
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_SeedPointSetInteractor);
  }
  //END ONLY FOR SEEDPOINTS******************************************************************************************************************************************
}

/***************      SET THRESHOLD     ***************/
void QmitkPixelGreyValueManipulatorComponent::SetThreshold(const QString& threshold)
{
  m_PixelGreyValueManipulatorComponentGUI->SetThreshold(atoi(threshold));
}

///*********** SHOW PIXEL MANIPULATOR CONTENT ***********/
void QmitkPixelGreyValueManipulatorComponent::ShowPixelGreyValueManipulatorContent(bool)
{
  m_PixelGreyValueManipulatorComponentGUI->GetPixelManipulatorContentGroupBox()->setShown(m_PixelGreyValueManipulatorComponentGUI->GetPixelGreyValueManipulatorGroupBox()->isChecked());
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkPixelGreyValueManipulatorComponent::ShowImageContent(bool)
{
  m_PixelGreyValueManipulatorComponentGUI->GetImageContent()->setShown(m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()->isChecked());
}

void QmitkPixelGreyValueManipulatorComponent::HideOrShowValue2(int /*index*/)
{ 
  if(m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()->currentItem() == 2)
  {
    m_PixelGreyValueManipulatorComponentGUI->GetValue2GroupBox()->setShown(true);
  }
  else if(m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()->currentItem() != 2)
  {
    m_PixelGreyValueManipulatorComponentGUI->GetValue2GroupBox()->setShown(false);
  }
}

/****PIPELINE CONTROLLER TO CREATE MANIPULATED IMAGE***/
void QmitkPixelGreyValueManipulatorComponent::PipelineControllerToCreateManipulatedImage()
{
  int manipulationMode;
  int manipulationArea;
  GetManipulationModeAndAreaFromGUI(manipulationMode, manipulationArea);

  m_ManipulationMode = manipulationMode;
  m_ManipulationArea = manipulationArea;

  bool boolSegmentation = false;
  if(manipulationArea == 2)
  {
    boolSegmentation = true;
  }

  switch(manipulationMode)
  {
  case 0: 
    //nothing selected
    break;
  case 1: 
    LinearShift();
    break;
  case 2: 
    GradientShift();
    break;
  case 3:
    ChangeGreyValue();
    break;
  case 4:
    LightenOrShade();
    break;
  default:
    break;
  }
}


/*************** GET MANIPULATION MODE  ***************/
void QmitkPixelGreyValueManipulatorComponent::GetManipulationModeAndAreaFromGUI(int & manipulationMode, int & manipulationArea)
{
  manipulationMode = m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()->currentItem(); // 0 = Nothing selected, 1 = linear shift, 2 = gradient shift, 3 = change grey value, 4 = lighten / shade
  manipulationArea = m_PixelGreyValueManipulatorComponentGUI->GetManipulationAreaComboBox()->currentItem(); // 0 = Nothing selected, 1 = on Image, 2 = on Segmentation

  //default is Mode = linear, Area = Segmentation
  if(m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()->currentItem() == 0)
  {
    manipulationMode = 1;
  }

  if(m_PixelGreyValueManipulatorComponentGUI->GetManipulationAreaComboBox()->currentItem() == 0)
  {
    manipulationArea = 1;
  }
}

/*************** GET MANIPULATION AREA  ***************/
void QmitkPixelGreyValueManipulatorComponent::GetManipulationValueFromGUI(int & value1, int & value2, int & baseValue)
{
  value1 = atoi(m_PixelGreyValueManipulatorComponentGUI->GetValue1LineEdit()->text());
  value2 = atoi(m_PixelGreyValueManipulatorComponentGUI->GetValue2LineEdit()->text());
  baseValue = m_PixelGreyValueManipulatorComponentGUI->GetBaseThreshold();

}


/***************      LINEAR SHIFT      ***************/
void QmitkPixelGreyValueManipulatorComponent::LinearShift()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  mitk::Image*  image = m_MitkImage;
  AccessFixedDimensionByItk_1(image, CreateLinearShiftedImage, 3, m_Segmentation.GetPointer()); 
  QApplication::restoreOverrideCursor();
}

/***************     GRADIENT SHIFT     ***************/
void QmitkPixelGreyValueManipulatorComponent::GradientShift()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  mitk::Image*  image = m_MitkImage;
  AccessFixedDimensionByItk_1(image, CreateGradientShiftedImage, 3, m_Segmentation.GetPointer()); 
  QApplication::restoreOverrideCursor();
}

/***************   CHANGE GREY VALUE    ***************/
void QmitkPixelGreyValueManipulatorComponent::ChangeGreyValue()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  mitk::Image*  image = m_MitkImage;
  AccessFixedDimensionByItk_1(image, CreateChangedGreyValueImage, 3, m_Segmentation.GetPointer()); 
  QApplication::restoreOverrideCursor();
}

/***************   LIGHTEN OR SHADE     ***************/
void QmitkPixelGreyValueManipulatorComponent::LightenOrShade()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  mitk::Image*  image = m_MitkImage;
  AccessFixedDimensionByItk_1(image, CreateLightenOrShadeImage, 3, m_Segmentation.GetPointer()); 
  QApplication::restoreOverrideCursor();
}


/***************     LINEAR TEMPLATE    ***************/
template < typename TPixel, unsigned int VImageDimension >    
void QmitkPixelGreyValueManipulatorComponent::CreateLinearShiftedImage( itk::Image< TPixel, VImageDimension >* itkImage, const mitk::Image* segmentation)
{
  typedef itk::Image< TPixel, VImageDimension > ItkImageType;
  itk::ImageRegionConstIterator<ItkImageType> it(itkImage, itkImage->GetLargestPossibleRegion() );

  typedef itk::Image< unsigned char, VImageDimension > ItkSegmentationImageType;
  typename ItkSegmentationImageType::Pointer itkSegmentation;

  if(segmentation != NULL)
  {
    mitk::CastToItkImage(segmentation, itkSegmentation);
  }

  typename ItkImageType::Pointer itkShiftedImage = ItkImageType::New();
  itkShiftedImage->SetRegions(itkImage->GetLargestPossibleRegion());
  itkShiftedImage->Allocate();
  itk::ImageRegionIterator<ItkImageType> itShifted(itkShiftedImage, itkShiftedImage->GetLargestPossibleRegion() );

  const typename ItkImageType::RegionType & imageRegion = itkImage->GetLargestPossibleRegion();

  int imageDim = 1;

  for(int dimension = 0; dimension < VImageDimension; ++dimension)
  {
    imageDim *= imageRegion.GetSize(dimension); //Anzahl der Pixel des Bildes die in unten stehender while-Schleife durchlaufen werden müssen
  }

  int value1;
  int value2;
  int baseValue;

  GetManipulationValueFromGUI(value1, value2, baseValue);

  int baseThreshold = baseValue;
  int shiftedThresholdOne = value1;

  if(m_ManipulationArea == 2) //if manipulation shall be only on segmented parts
  {
    if(m_Segmentation.IsNotNull())
    {
      itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
      while(!(it.IsAtEnd()))
      {
        if(itSeg.Get()!=0)
          itShifted.Set(it.Get()+ (baseThreshold -shiftedThresholdOne));
        else
          itShifted.Set(it.Get());
        ++it;
        ++itShifted;
        ++itSeg;

        --imageDim;//Kontrollausgabe
        if(imageDim % 1000 == 0)
        {
          std::cout<<imageDim<<std::endl;
        }
      }//end of while
    }//if(m_Segmentation != NULL)
  }//end of manipulation on segmented area

  else //manipulation shall be on entire image
  {
    while(!(it.IsAtEnd()))
    {
      itShifted.Set(it.Get()+ (baseThreshold -shiftedThresholdOne));
      ++it;
      ++itShifted;

      --imageDim;//Kontrollausgabe
      if(imageDim % 1000 == 0)
      {
        std::cout<<imageDim<<std::endl;
      }
    }//end of while
  }//end of manipulation on entire image

  AddManipulatedImageIntoTree<ItkImageType>(itkShiftedImage);

}// end of CreateLinearShiftedImage

/***************    GRADIENT TEMPLATE   ***************/
template < typename TPixel, unsigned int VImageDimension >    
void QmitkPixelGreyValueManipulatorComponent::CreateGradientShiftedImage( itk::Image< TPixel, VImageDimension >* itkImage, const mitk::Image* segmentation)
{
  typedef itk::Image< TPixel, VImageDimension > ItkImageType;
  itk::ImageRegionConstIterator<ItkImageType> it(itkImage, itkImage->GetLargestPossibleRegion() );

  typedef itk::Image< unsigned char, VImageDimension > ItkSegmentationImageType;
  typename ItkSegmentationImageType::Pointer itkSegmentation;

  if(segmentation != NULL)
  {
    mitk::CastToItkImage(segmentation, itkSegmentation);
  }

  typename ItkImageType::Pointer itkShiftedImage = ItkImageType::New();
  itkShiftedImage->SetRegions(itkImage->GetLargestPossibleRegion());
  itkShiftedImage->Allocate();
  itk::ImageRegionIterator<ItkImageType> itShifted(itkShiftedImage, itkShiftedImage->GetLargestPossibleRegion() );

  const typename ItkImageType::RegionType & imageRegion = itkImage->GetLargestPossibleRegion();

  int imageDim = 1;

  for(int dimension = 0; dimension < VImageDimension; ++dimension)
  {
    imageDim *= imageRegion.GetSize(dimension); //Anzahl der Pixel des Bildes die in unten stehender while-Schleife durchlaufen werden müssen
  }

  int value1;
  int value2;
  int baseValue;

  GetManipulationValueFromGUI(value1, value2, baseValue);

  int baseThreshold = baseValue;
  int shiftedThresholdOne = value1;
  int shiftedThresholdTwo = value2;

  if(m_SeedPointSetNode.IsNotNull())
  {
    mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(m_SeedPointSetNode->GetData());
    int numberOfPoints = pointSet->GetSize();

    mitk::PointSet::PointType pointOne = pointSet->GetPoint(0);
    mitk::PointSet::PointType pointTwo = pointSet->GetPoint(1);

    if(numberOfPoints == 2)
    {

      if(m_ManipulationArea == 2) //if manipulation shall be only on segmented parts
      {     
        if(m_Segmentation.IsNotNull())
        {
          itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
          while(!(it.IsAtEnd()))
          {
            if(itSeg.Get()!=0)//da wo segmentiert ist
            {
              InternalGradientShiftCalculation(shiftedThresholdOne, shiftedThresholdTwo, baseThreshold, itShifted, it, pointOne, pointTwo );
            }
            else
            {
              itShifted.Set(it.Get());
            }
            ++itSeg;
            ++it;
            ++itShifted;

            //control output: how many Pixels are still to change
            --imageDim;
            if(imageDim % 1000 == 0)
            {
              std::cout<<imageDim<<std::endl;
            }
          }//end of while(!(it.IsAtEnd())
        }//end of if(m_Segmentation.IsNotNull()
      }// end of if manipulation shall be only on segmented parts

      else //if manipulation area is entier image
      {
        while(!(it.IsAtEnd()))
        {
          InternalGradientShiftCalculation(shiftedThresholdOne, shiftedThresholdTwo, baseThreshold, itShifted, it, pointOne, pointTwo );
          ++it;
          ++itShifted;

          //control output: how many Pixels are still to change
          --imageDim;
          if(imageDim % 1000 == 0)
          {
            std::cout<<imageDim<<std::endl;
          }
        }//end of while(!(it.IsAtEnd())
      }//end of else -> manipulation area is entier image
    }// end of if (numberOfPoints == 2)
  } //end of if(m_PointSetNode != NULL)

  AddManipulatedImageIntoTree<ItkImageType>(itkShiftedImage);

}// end of CreateGradientShiftedImage


/**************INTERNAL GRADIENT SHIFT CALC************/
template < typename ItkImageType >  
void QmitkPixelGreyValueManipulatorComponent::InternalGradientShiftCalculation(int & shiftedThresholdOne, int & shiftedThresholdTwo, int & normalThreshold, itk::ImageRegionIterator<ItkImageType> & itShifted, itk::ImageRegionConstIterator<ItkImageType> & it, mitk::PointSet::PointType & pointOne, mitk::PointSet::PointType & pointTwo)
{
  typename ItkImageType::IndexType thisPointIndex = it.GetIndex();
  //to change index to worldcoordinates
  mitk::Image* image = static_cast<mitk::Image*>(m_MitkImage);
  mitk::Point3D thisPoint3DWorldIndex;
  mitk::Point3D thisPoint3DWorldChangedIndex;

  thisPoint3DWorldIndex[0]=thisPointIndex[0];
  thisPoint3DWorldIndex[1]=thisPointIndex[1];
  thisPoint3DWorldIndex[2]=thisPointIndex[2];

  image->GetGeometry(0)->IndexToWorld(thisPoint3DWorldIndex, thisPoint3DWorldChangedIndex);

  double x1 = pointOne[0];
  double y1 = pointOne[1];
  double z1 = pointOne[2];

  double x3 = thisPoint3DWorldChangedIndex[0];
  double y3 = thisPoint3DWorldChangedIndex[1];
  double z3 = thisPoint3DWorldChangedIndex[2]; 

  mitk::Vector3D pMinusP1;
  pMinusP1[0]= x3 - x1;
  pMinusP1[1]= y3 - y1;
  pMinusP1[2]= z3 - z1;

  mitk::Vector3D v;
  v = pointTwo-pointOne;
  mitk::ScalarType vq;
  vq = 1.0/v.GetSquaredNorm();

  mitk::ScalarType f;
  f=v * (pMinusP1)*vq;

  double gradientShiftThresh; 
  gradientShiftThresh = shiftedThresholdOne * (1-f) + shiftedThresholdTwo * f;

  double min;
  double max;
  if (shiftedThresholdOne < shiftedThresholdTwo)
  {
    min = shiftedThresholdOne;
    max = shiftedThresholdTwo;
  }
  else 
  {
    max = shiftedThresholdOne;
    min = shiftedThresholdTwo;
  }

  if(gradientShiftThresh < min)
  {
    gradientShiftThresh = min;
  }
  if(gradientShiftThresh > max)
  {
    gradientShiftThresh = max;
  }

  double shiftValue = normalThreshold - gradientShiftThresh;
  itShifted.Set((typename ItkImageType::PixelType)(it.Get() + shiftValue));
}//end of InternalGradientShiftCalculation


/**************CHANGED GREY VALUE TEMPLATE*************/
template < typename TPixel, unsigned int VImageDimension >    
void QmitkPixelGreyValueManipulatorComponent::CreateChangedGreyValueImage( itk::Image< TPixel, VImageDimension >* itkImage, const mitk::Image* segmentation)
{
  typedef itk::Image< TPixel, VImageDimension > ItkImageType;
  itk::ImageRegionConstIterator<ItkImageType> it(itkImage, itkImage->GetLargestPossibleRegion() );

  typedef itk::Image< unsigned char, VImageDimension > ItkSegmentationImageType;
  typename ItkSegmentationImageType::Pointer itkSegmentation;

  if(segmentation != NULL)
  {
    mitk::CastToItkImage(segmentation, itkSegmentation);
  }

  typename ItkImageType::Pointer itkShiftedImage = ItkImageType::New();
  itkShiftedImage->SetRegions(itkImage->GetLargestPossibleRegion());
  itkShiftedImage->Allocate();
  itk::ImageRegionIterator<ItkImageType> itShifted(itkShiftedImage, itkShiftedImage->GetLargestPossibleRegion() );

  const typename ItkImageType::RegionType & imageRegion = itkImage->GetLargestPossibleRegion();

  int imageDim = 1;

  for(int dimension = 0; dimension < VImageDimension; ++dimension)
  {
    imageDim *= imageRegion.GetSize(dimension); //Anzahl der Pixel des Bildes die in unten stehender while-Schleife durchlaufen werden müssen
  }

  int value1;
  int value2;
  int baseValue;

  GetManipulationValueFromGUI(value1, value2, baseValue);

  int pixelChangeValue = value1;

  if(m_ManipulationArea == 2) //if manipulation shall be only on segmented parts
  {
    if(m_Segmentation.IsNotNull())
    {
      itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
      while(!(it.IsAtEnd()))
      {
        if(itSeg.Get()!=0)
          itShifted.Set(pixelChangeValue);
        else
          itShifted.Set(it.Get());
        ++it;
        ++itShifted;
        ++itSeg;

        //control output: how many Pixels are still to change
        --imageDim;
        if(imageDim % 1000 == 0)
        {
          std::cout<<imageDim<<std::endl;
        }
      }//end of while (!(it.IsAtEnd()))
    }//if(m_Segmentation != NULL)
  }//end of manipulation on segmented area


  else //manipulation shall be on entire image
  {
    while(!(it.IsAtEnd()))
    {
      itShifted.Set(pixelChangeValue); //set the new pixelvalue

      ++it;
      ++itShifted;

      //control output: how many Pixels are still to change
      --imageDim;
      if(imageDim % 1000 == 0)
      {
        std::cout<<imageDim<<std::endl;
      }
    }//end of while (!(it.IsAtEnd()))
  }//end of manipulation on entire image

  AddManipulatedImageIntoTree<ItkImageType>(itkShiftedImage);

}// end of CHANGED GREY VALUE IMAGE


/************** LIGHTEN OR SHADE TEMPLATE *************/
template < typename TPixel, unsigned int VImageDimension >    
void QmitkPixelGreyValueManipulatorComponent::CreateLightenOrShadeImage( itk::Image< TPixel, VImageDimension >* itkImage, const mitk::Image* segmentation)
{
  typedef itk::Image< TPixel, VImageDimension > ItkImageType;
  itk::ImageRegionConstIterator<ItkImageType> it(itkImage, itkImage->GetLargestPossibleRegion() );

  typedef itk::Image< unsigned char, VImageDimension > ItkSegmentationImageType;
  typename ItkSegmentationImageType::Pointer itkSegmentation;

  if(segmentation != NULL)
  {
    mitk::CastToItkImage(segmentation, itkSegmentation);
  }

  typename ItkImageType::Pointer itkShiftedImage = ItkImageType::New();
  itkShiftedImage->SetRegions(itkImage->GetLargestPossibleRegion());
  itkShiftedImage->Allocate();
  itk::ImageRegionIterator<ItkImageType> itShifted(itkShiftedImage, itkShiftedImage->GetLargestPossibleRegion() );

  const typename ItkImageType::RegionType & imageRegion = itkImage->GetLargestPossibleRegion();

  int imageDim = 1;

  for(int dimension = 0; dimension < VImageDimension; ++dimension)
  {
    imageDim *= imageRegion.GetSize(dimension); //Anzahl der Pixel des Bildes die in unten stehender while-Schleife durchlaufen werden müssen
  }

  int value1;
  int value2;
  int baseValue;

  GetManipulationValueFromGUI(value1, value2, baseValue);

  int pixelChangeValue = value1;

  if(m_ManipulationArea == 2) //if manipulation shall be only on segmented parts
  {
    if(m_Segmentation.IsNotNull())
    {
      itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
      while(!(it.IsAtEnd()))// for each pixel int the image
      {
        if(itSeg.Get()!=0)// if there is a segmentation 
          itShifted.Set(it.Get() + pixelChangeValue);//add the new pixel value on the old one
        else // else leave the value the old one
          itShifted.Set(it.Get());
        ++it;
        ++itShifted;
        ++itSeg;

        //control output: how many Pixels are still to change
        --imageDim;
        if(imageDim % 1000 == 0)
        {
          std::cout<<imageDim<<std::endl;
        }
      }//end of while (!(it.IsAtEnd()))
    }//if(m_Segmentation != NULL)
  }//end of manipulation on segmented area


  else //manipulation shall be on entire image
  {
    while(!(it.IsAtEnd()))
    {
      itShifted.Set(it.Get() + pixelChangeValue);//add the new pixel value on the old one

      ++it;
      ++itShifted;

      //control output: how many Pixels are still to change
      --imageDim;
      if(imageDim % 1000 == 0)
      {
        std::cout<<imageDim<<std::endl;
      }
    }//end of while (!(it.IsAtEnd()))
  }//end of manipulation on entire image

  AddManipulatedImageIntoTree<ItkImageType>(itkShiftedImage);

}// end of CreateLightenOrShadeImage

template < typename ItkImageType >  
void QmitkPixelGreyValueManipulatorComponent::AddManipulatedImageIntoTree(typename ItkImageType::Pointer & itkShiftedImage)
{
  if(m_SegmentedShiftResultNode.IsNotNull())
  {
    m_SegmentedShiftResultNode->SetData(NULL);
  }

  m_PixelChangedImage = mitk::Image::New(); 
  mitk::CastToMitkImage(itkShiftedImage, m_PixelChangedImage);
  m_PixelChangedImage->SetGeometry(static_cast<mitk::Geometry3D*>(m_MitkImage->GetGeometry()->Clone().GetPointer()));
  m_ItNewBuildSeg = GetDataTreeIterator();

  mitk::DataTreeIteratorClone  selectedIterator; 

  if(m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter()->GetSelectedItem() != NULL)
  {
    selectedIterator = m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter()->GetIteratorToSelectedItem(); 
    m_SegmentationNode = m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter()->GetSelectedItem()->GetNode();
  }
  else
  {
    selectedIterator = m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->GetFilter()->GetIteratorToSelectedItem(); 
  }

  //check if current node is a image
  if ( m_SegmentationNode.IsNotNull() ) 
  {
    std::string nodeName;

    if( m_SegmentationNode->GetPropertyList().IsNotNull() )
    {
      if( m_SegmentationNode->GetPropertyList()->GetProperty("name").IsNotNull() )
      {
        nodeName = m_SegmentationNode->GetPropertyList()->GetProperty("name")->GetValueAsString();
      }//end of if( m_SegmentationNode->GetPropertyList()->GetProperty("name") != NULL)
    }//end of if( m_SegmentationNode->GetPropertyList() != NULL)
  }//end of if(m_SegmentationNode.IsNotNull())
  ++m_PixelChangedImageCounter;
  std::ostringstream buffer;
  buffer << m_PixelChangedImageCounter;
  std::string sPName = "PixelChanged Image " + buffer.str();
  m_PixelChangedImageNode = mitk::DataTreeNode::New();//m_GradientShiftedImageNode = mitk::DataTreeNode::New();
  m_PixelChangedImageNode->SetData(m_PixelChangedImage);
  m_PixelChangedImageNode->SetProperty("name", new mitk::StringProperty(sPName ) );
  m_PixelChangedImageNode->SetIntProperty("layer", 2);
  selectedIterator->Add(m_PixelChangedImageNode);
}
