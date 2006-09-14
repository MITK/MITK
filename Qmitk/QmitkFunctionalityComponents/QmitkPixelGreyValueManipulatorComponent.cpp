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

#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>


/***************       CONSTRUCTOR      ***************/
QmitkPixelGreyValueManipulatorComponent::QmitkPixelGreyValueManipulatorComponent(QObject *parent, const char *parentName, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, bool updateSelector, bool showSelector):
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

}

/***************       CONNECTIONS      ***************/
void QmitkPixelGreyValueManipulatorComponent::CreateConnections()
{
  if ( m_PixelGreyValueManipulatorComponentGUI )
  {
    connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));        
    connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetPixelGreyValueManipulatorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowPixelGreyValueManipulatorContent(bool)));
    connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool)));

    //Button "create new manipulated image" pressed
    connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetCreateNewManipulatedImageButton()), SIGNAL(pressed()), (QObject*) this, SLOT(PipelineControlerToCreateManipulatedImage()));
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
  if(m_PixelGreyValueManipulatorComponentGUI != NULL)
  {

    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      m_AddedChildList[i]->ImageSelected(m_SelectedImage);
    }
  }
  TreeChanged();
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkPixelGreyValueManipulatorComponent::CreateContainerWidget(QWidget* parent)
{
  m_PixelGreyValueManipulatorComponentGUI = new QmitkPixelGreyValueManipulatorComponentGUI(parent);
  m_GUI = m_PixelGreyValueManipulatorComponentGUI;
  m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
 
  if(!m_ShowSelector)
  {
    m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()->setShown(false);
  }
  //m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithBoolProperty<mitk::Image>("IsSegmentationImage"));
  
  CreateConnections();

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
}

/***************       DEACTIVATED      ***************/
void QmitkPixelGreyValueManipulatorComponent::Deactivated()
{
  m_Active = false;
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
  if(m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()->currentItem() != 2)
  {
    m_PixelGreyValueManipulatorComponentGUI->GetValue2GroupBox()->setShown(false);
  }
  else if(m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()->currentItem() == 2)
  {
    m_PixelGreyValueManipulatorComponentGUI->GetValue2GroupBox()->setShown(true);
  }
}

/****PIPELINE CONTROLLER TO CREATE MANIPULATED IMAGE***/
 void QmitkPixelGreyValueManipulatorComponent::PipelineControllerToCreateManipulatedImage()
 {
  int manipulationMode;
  int manipulationArea;
  GetManipulationModeAndAreaFromGUI(manipulationMode, manipulationArea);
  int value1;
  int value2;
  GetManipulationValueFromGUI(value1, value2);
  switch(manipulationMode)
  {
  case 0: 
    //nothing selected
    break;
  case 1: 
    // Linear Shift 
    //if(manipulationArea == 2)//if manipulationArea == Segmentation
    //{
    //  //LinearShift(const mitk::Image* image, const mitk::Image* segmentation);
    //}
    //else(manipulationArea == 1 )// else (i.e. even if nothing is selected, the default is manipulationArea = Image
    //{
    //  //LinearShift(const mitk::Image* image, const mitk::Image* segmentation);
    //}
    break;
  case 2: 
    // GradientShift
    //    if(manipulationArea == 2)//if manipulationArea == Segmentation
    //{

    //}
    //else(manipulationArea == 1 )// else (i.e. even if nothing is selected, the default is manipulationArea = Image
    //{

    //}
    break;
  case 3:
    // ChangeGreyValue
    //    if(manipulationArea == 2)//if manipulationArea == Segmentation
    //{

    //}
    //else(manipulationArea == 1 )// else (i.e. even if nothing is selected, the default is manipulationArea = Image
    //{

    //}
    break;
  case 4:
    //Lighten / Shade
    //    if(manipulationArea == 2)//if manipulationArea == Segmentation
    //{

    //}
    //else(manipulationArea == 1 )// else (i.e. even if nothing is selected, the default is manipulationArea = Image
    //{

    //}
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
    manipulationArea = 2;
  }
}

/*************** GET MANIPULATION AREA  ***************/
void QmitkPixelGreyValueManipulatorComponent::GetManipulationValueFromGUI(int & value1, int & value2)
{
  value1 = atoi(m_PixelGreyValueManipulatorComponentGUI->GetValue1LineEdit()->text());
  value2 = atoi(m_PixelGreyValueManipulatorComponentGUI->GetValue2LineEdit()->text());

}

void QmitkPixelGreyValueManipulatorComponent::LinearShift(const mitk::Image* image, const mitk::Image* segmentation)
{

}

////******************************************** Shift Local Threshold******************************
////
////connect if button "SHIFT LOCAL THRESHOLD" was pressed, calls ChangePixelValue
//void QmitkSurfaceCreator::shiftLocalThreshold()
//{
//  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
//
//  m_ThresholdMethod = m_Controls->getShiftThresholdComboBox()->currentItem();
//
//  // if for valueChange a segmentation is neccessary (0, 1, 3)
//  if ((m_ThresholdMethod == 0) || (m_ThresholdMethod == 1) || (m_ThresholdMethod == 3))
//  {
//    if (m_Segmentation.IsNotNull())
//    {
//      mitk::Image*  image = static_cast<mitk::Image*> (m_IteratorOnImageToBeSkinExtracted->Get()->GetData());
//      AccessFixedDimensionByItk_2(image, ChangePixelValueByUrte, 3, m_Segmentation.GetPointer(), m_Controls); 
//    }//end of if (m_Segmentation != NULL)
//    else 
//    {
//      mitk::StatusBar::GetInstance()->DisplayText( "You need a Segmentation first! (load in ERIS)");
//    }
//  }//end of if ((m_ThresholdMethod == 0) || (m_ThresholdMethod == 1) || (m_ThresholdMethod == 3))
//  else //if no segmentation is neccessary
//  {
//    mitk::Image*  image = static_cast<mitk::Image*> (m_IteratorOnImageToBeSkinExtracted->Get()->GetData());
//    AccessFixedDimensionByItk_2(image, ChangePixelValueByUrte, 3, m_Segmentation.GetPointer(), m_Controls); 
//  }
//  std::cout<<"Template ready\t"<<std::endl;
//
//  QApplication::restoreOverrideCursor();
//}//end of shiftLocalThreshold()
//
//
////******************************************CHANGE PIXEL VALUE******************+
//// to change pixel values (linear shift, gradient shift, and per amount, on segmentation or image
//template < typename TPixel, unsigned int VImageDimension >    
//void QmitkSurfaceCreator::ChangePixelValueByUrte( itk::Image< TPixel, VImageDimension >* itkImage, const mitk::Image* segmentation, QmitkSurfaceCreatorControls * m_Controls)
//{
//  m_ThresholdMethod = m_Controls->getShiftThresholdComboBox()->currentItem();
//
//  typedef itk::Image< TPixel, VImageDimension > ItkImageType;
//  itk::ImageRegionConstIterator<ItkImageType> it(itkImage, itkImage->GetLargestPossibleRegion() );
//
//  typedef itk::Image< unsigned char, VImageDimension > ItkSegmentationImageType;
//  typename ItkSegmentationImageType::Pointer itkSegmentation;
//
//  if(segmentation != NULL)
//  {
//    mitk::CastToItkImage(segmentation, itkSegmentation);
//  }
//
//  typename ItkImageType::Pointer itkShiftedImage = ItkImageType::New();
//  itkShiftedImage->SetRegions(itkImage->GetLargestPossibleRegion());
//  itkShiftedImage->Allocate();
//  itk::ImageRegionIterator<ItkImageType> itShifted(itkShiftedImage, itkShiftedImage->GetLargestPossibleRegion() );
//
//  const typename ItkImageType::RegionType & imageRegion = itkImage->GetLargestPossibleRegion();
//
//  int xImageDim = imageRegion.GetSize(0);
//  int yImageDim = imageRegion.GetSize(1);
//  int zImageDim = imageRegion.GetSize(2);
//
//  int imageDim = xImageDim * yImageDim * zImageDim; //Anzahl der Pixel des Bildes die in unten stehender while-Schleife durchlaufen werden müssen
//
//  int normalThreshold(0);
//  int shiftedThresholdOne(0);
//  int shiftedThresholdTwo(0);
//  int pixelChangeValue(0);
//
//
//  // m_ThresholdMethod is set in the method comboBoxSelection. Its value depends on the choosen selection:
//  // switch which method is choosen
//  // 0 = shift linear on segmentation, value = normalThreshold - shiftThreshold
//  // 1 = shift with a gradient on a segmentation
//  // 2 = shift with a gradient on the hole image
//  // 3 = change value of pixel on a segmentation
//  // 4 = change value of pixel on the hole image
//  // 5 = lighten or shade pixel value on segmentation
//
//  switch(m_ThresholdMethod)
//  {
//
//
//  case 0: /*********************SHIFT LINEAR ON SEGMENTATION***************************************/
//    {
//      normalThreshold = atoi(m_Controls->getThresholdLineEdit()->text());
//      shiftedThresholdOne = atoi(m_Controls->getShiftThresholdSpinBoxOneLinear()->text());
//
//      if(m_Segmentation.IsNotNull())
//      {
//        itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
//        while(!(it.IsAtEnd()))
//        {
//          if(itSeg.Get()!=0)
//            itShifted.Set(it.Get()+ (normalThreshold -shiftedThresholdOne));
//          else
//            itShifted.Set(it.Get());
//          ++it;
//          ++itShifted;
//          ++itSeg;
//
//          --imageDim;//Kontrollausgabe
//          if(imageDim % 1000 == 0)
//          {
//            std::cout<<imageDim<<std::endl;
//          }
//
//        }//end of while
//      }//if(m_Segmentation != NULL)
//      else 
//      {
//        mitk::StatusBar::GetInstance()->DisplayText( "You need a Segmentation first! (load in ERIS)");
//      }
//    }//end of case 0
//    break;
//
//
//  case 1: /*********************SHIFT GRADIENT ON SEGMENTATION***************************************/
//    {
//      normalThreshold = atoi(m_Controls->getThresholdLineEdit()->text());
//      shiftedThresholdOne = atoi(m_Controls->getShiftThresholdSpinBoxOneGradientOnSegmentation()->text());
//      shiftedThresholdTwo = atoi(m_Controls->getShiftThresholdSpinBoxTwoGradientOnSegmentation()->text());
//
//      if(m_Segmentation.IsNotNull())
//      {
//        itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
//
//        if(m_PointSetNode.IsNotNull())
//        {
//          mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(m_SeedPointSetNode->GetData());
//          int numberOfPoints = pointSet->GetSize();
//
//          mitk::PointSet::PointType pointOne = pointSet->GetPoint(0);
//          mitk::PointSet::PointType pointTwo = pointSet->GetPoint(1);
//
//          if (numberOfPoints == 2)
//          {
//            while(!(it.IsAtEnd()))
//            {
//              if(itSeg.Get()!=0)
//              {
//                typename ItkImageType::IndexType thisPointIndex = it.GetIndex();
//
//                //to change index to worldcoordinates
//                mitk::Image*  image = static_cast<mitk::Image*> (m_IteratorOnImageToBeSkinExtracted->Get()->GetData());
//                mitk::Point3D thisPoint3DWorldIndex;
//                mitk::Point3D thisPoint3DWorldChangedIndex;
//                thisPoint3DWorldIndex[0]=thisPointIndex[0];
//                thisPoint3DWorldIndex[1]=thisPointIndex[1];
//                thisPoint3DWorldIndex[2]=thisPointIndex[2];
//
//                image->GetGeometry(0)->IndexToWorld(thisPoint3DWorldIndex, thisPoint3DWorldChangedIndex);
//
//                double x1 = pointOne[0];
//                double y1 = pointOne[1];
//                double z1 = pointOne[2];
//
//                double x3 = thisPoint3DWorldChangedIndex[0];
//                double y3 = thisPoint3DWorldChangedIndex[1];
//                double z3 = thisPoint3DWorldChangedIndex[2]; 
//
//                mitk::Vector3D pMinusP1;
//                pMinusP1[0]= x3 - x1;
//                pMinusP1[1]= y3 - y1;
//                pMinusP1[2]= z3 - z1;
//
//                mitk::Vector3D v;
//                v = pointTwo-pointOne;
//                mitk::ScalarType vq;
//                vq = 1.0/v.GetSquaredNorm();
//
//                mitk::ScalarType f;
//                f=v * (pMinusP1)*vq;
//
//                double gradientShiftThresh; 
//                gradientShiftThresh = shiftedThresholdOne * (1-f) + shiftedThresholdTwo * f;
//
//                double min;
//                double max;
//                if (shiftedThresholdOne < shiftedThresholdTwo)
//                {
//                  min = shiftedThresholdOne;
//                  max = shiftedThresholdTwo;
//                }
//                else 
//                {
//                  max = shiftedThresholdOne;
//                  min = shiftedThresholdTwo;
//                }
//
//                if(gradientShiftThresh < min)
//                {
//                  gradientShiftThresh = min;
//                }
//                if(gradientShiftThresh > max)
//                {
//                  gradientShiftThresh = max;
//                }
//
//                double shiftValue = normalThreshold - gradientShiftThresh;
//
//                itShifted.Set((typename ItkImageType::PixelType)(it.Get() + shiftValue));
//              }//end of if(itPixelChange.Get()!=0=
//
//              else
//                itShifted.Set(it.Get());
//              ++it;
//              ++itShifted;
//              ++itSeg;
//
//              //control output: how many Pixels are still to change
//              --imageDim;
//              if(imageDim % 1000 == 0)
//              {
//                std::cout<<imageDim<<std::endl;
//              }
//
//            }//end of while(!(it.IsAtEnd())
//          } // end of if (numberOfPoints == 2)
//          else if(numberOfPoints != 2)
//          {
//            mitk::StatusBar::GetInstance()->DisplayText( "You need 2 Seedpoints! (Ctrl + leftMouse)");
//          }//end of else if(numberOfPoints != 2)
//        }//end of if(m_PointSetNode != NULL)
//      }//end of if(m_Segmentation !=NULL)
//      else 
//      {
//        mitk::StatusBar::GetInstance()->DisplayText( "You need a Segmentation first! (load in ERIS)");
//      }
//    }//end of case 1
//    break;
//
//
//  case 2: /*********************SHIFT GRADIENT ON IMAGE***************************************/
//    {
//      normalThreshold = atoi(m_Controls->getThresholdLineEdit()->text());
//      shiftedThresholdOne = atoi(m_Controls->getShiftThresholdSpinBoxOneGradientOnImage()->text());
//      shiftedThresholdTwo = atoi(m_Controls->getShiftThresholdSpinBoxTwoGradientOnImage()->text());
//
//      if(m_PointSetNode.IsNotNull())
//      {
//        mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(m_SeedPointSetNode->GetData());
//        int numberOfPoints = pointSet->GetSize();
//        std::cout<<numberOfPoints<<std::endl;
//        std::cout<<"Punkte"<<std::endl;
//
//        mitk::PointSet::PointType pointOne = pointSet->GetPoint(0);
//        mitk::PointSet::PointType pointTwo = pointSet->GetPoint(1);
//
//        if (numberOfPoints == 2)
//        {
//          while(!(it.IsAtEnd()))
//          {
//            typename ItkImageType::IndexType thisPointIndex = it.GetIndex();
//
//            //to change index to worldcoordinates
//            mitk::Image*  image = static_cast<mitk::Image*> (m_IteratorOnImageToBeSkinExtracted->Get()->GetData());
//            mitk::Point3D thisPoint3DWorldIndex;
//            mitk::Point3D thisPoint3DWorldChangedIndex;
//            thisPoint3DWorldIndex[0]=thisPointIndex[0];
//            thisPoint3DWorldIndex[1]=thisPointIndex[1];
//            thisPoint3DWorldIndex[2]=thisPointIndex[2];
//
//            image->GetGeometry(0)->IndexToWorld(thisPoint3DWorldIndex, thisPoint3DWorldChangedIndex);
//
//            double x1 = pointOne[0];
//            double y1 = pointOne[1];
//            double z1 = pointOne[2];
//
//            double x3 = thisPoint3DWorldChangedIndex[0];
//            double y3 = thisPoint3DWorldChangedIndex[1];
//            double z3 = thisPoint3DWorldChangedIndex[2];        
//
//            mitk::Vector3D pMinusP1;
//            pMinusP1[0]= x3 - x1;
//            pMinusP1[1]= y3 - y1;
//            pMinusP1[2]= z3 - z1;
//
//            mitk::Vector3D v;
//            v = pointTwo-pointOne;
//            mitk::ScalarType vq;
//            vq = 1.0/v.GetSquaredNorm();
//
//            mitk::ScalarType f;
//            f=v * (pMinusP1)*vq;
//
//
//            double gradientShiftThresh; 
//            gradientShiftThresh = shiftedThresholdOne * (1-f) + shiftedThresholdTwo * f;
//
//            double min;
//            double max;
//            if (shiftedThresholdOne < shiftedThresholdTwo)
//            {
//              min = shiftedThresholdOne;
//              max = shiftedThresholdTwo;
//            }
//            else 
//            {
//              max = shiftedThresholdOne;
//              min = shiftedThresholdTwo;
//            }
//
//            if(gradientShiftThresh < min)
//            {
//              gradientShiftThresh = min;
//            }
//            if(gradientShiftThresh > max)
//            {
//              gradientShiftThresh = max;
//            }
//
//            double shiftValue = normalThreshold - gradientShiftThresh;
//
//            itShifted.Set((typename ItkImageType::PixelType)(it.Get() + shiftValue));
//
//            ++it;
//            ++itShifted;
//
//            //control output: how many Pixels are still to change
//            --imageDim;
//            if(imageDim % 1000 == 0)
//            {
//              std::cout<<imageDim<<std::endl;
//            }
//
//          }//end of while(!(it.IsAtEnd())
//        }//end of if(numberOfPoints!=2)
//        else if(numberOfPoints != 2)
//        {
//          mitk::StatusBar::GetInstance()->DisplayText( "You need 2 Seedpoints! (Ctrl + leftMouse)");
//        }//end of else if(numberOfPoints != 2)
//      }//end of if(m_PointSetNode != NULL)
//    }
//    break;
//  case 3: /*************************CHANGE PIXEL VALUE ON SEGMENTATION***************************/
//    {
//      pixelChangeValue = atoi(m_Controls->getChangePixelValueSpinBox()->text());
//      if(m_Segmentation.IsNotNull())
//      {
//        itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
//        while(!(it.IsAtEnd()))
//        {
//          if(itSeg.Get()!=0)
//            itShifted.Set(pixelChangeValue);
//          else
//            itShifted.Set(it.Get());
//          ++it;
//          ++itShifted;
//          ++itSeg;
//
//          //control output: how many Pixels are still to change
//          --imageDim;
//          if(imageDim % 1000 == 0)
//          {
//            std::cout<<imageDim<<std::endl;
//          }
//        }//end of while (!(it.IsAtEnd()))
//      }//end of if(m_Segmentation !=NULL)
//      else 
//      {
//        mitk::StatusBar::GetInstance()->DisplayText( "You need a Segmentation first! (load in ERIS)");
//      }
//    }//end of case 3
//    break;
//
//
//  case 4: /***************************CHANGE PIXEL VALUE ON IMAGE***********************************/
//    {
//      pixelChangeValue = atoi(m_Controls->getChangePixelValueSpinBox()->text());
//
//      while(!(it.IsAtEnd()))
//      {
//        itShifted.Set(pixelChangeValue); //set the new pixelvalue
//
//        ++it;
//        ++itShifted;
//
//        //control output: how many Pixels are still to change
//        --imageDim;
//        if(imageDim % 1000 == 0)
//        {
//          std::cout<<imageDim<<std::endl;
//        }
//      }//end of while (!(it.IsAtEnd()))
//    }
//    break;
//
//
//      case 5: /***********************LIGHTEN OR SHADE PIXEL VALUE ON SEGMENTATION*************************/
//    {
//      //define the pixelChangeValue: the value that will be added to the old pixelvalue
//      pixelChangeValue = atoi(m_Controls->getLightenOrShadePixelValueSpinBox()->text());
//      
//      if(m_Segmentation.IsNotNull())
//      {
//        itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
//        while(!(it.IsAtEnd()))// for each pixel int the image
//        {
//          if(itSeg.Get()!=0)// if there is a segmentation 
//            itShifted.Set(it.Get() + pixelChangeValue);//add the new pixel value on the old one
//          else // else leave the value the old one
//            itShifted.Set(it.Get());
//          ++it;
//          ++itShifted;
//          ++itSeg;
//
//          //control output: how many Pixels are still to change
//          --imageDim;
//          if(imageDim % 1000 == 0)
//          {
//            std::cout<<imageDim<<std::endl;
//          }
//        }//end of while (!(it.IsAtEnd()))
//      }//end of if(m_Segmentation !=NULL)
//      else 
//      {
//        mitk::StatusBar::GetInstance()->DisplayText( "You need a Segmentation first! (load in ERIS)");
//      }
//    }//end of case 3
//    break;
//
//
//  default:
//    {
//      mitk::StatusBar::GetInstance()->DisplayText("Something went wrong, please select again!");
//    }
//  }//end of switch (m_ThresholdMethod)
//
//  if(m_SegmentedShiftResultNode.IsNotNull())
//  {
//    m_SegmentedShiftResultNode->SetData(NULL);
//  }
//
//  m_PixelChangedImage = mitk::Image::New(); 
//  mitk::CastToMitkImage(itkShiftedImage, m_PixelChangedImage);
//  m_PixelChangedImage->SetGeometry(static_cast<mitk::Geometry3D*>(m_MitkImage->GetGeometry()->Clone().GetPointer()));
//  m_ItNewBuildSeg = GetDataTreeIterator();
//
//  const mitk::DataTreeIteratorClone * selectedIterator = m_Controls->getTreeNodeImageSelector()->GetSelectedIterator();
//  m_SegmentationNode = m_Controls->getTreeNodeImageSelector()->GetSelectedNode();
//
//  //check if current node is a image
//  if ( m_SegmentationNode.IsNotNull() ) 
//  {
//    std::string nodeName;
//
//    if( m_SegmentationNode->GetPropertyList().IsNotNull() )
//    {
//      if( m_SegmentationNode->GetPropertyList()->GetProperty("name").IsNotNull() )
//      {
//        nodeName = m_SegmentationNode->GetPropertyList()->GetProperty("name")->GetValueAsString();
//      }//end of if( m_SegmentationNode->GetPropertyList()->GetProperty("name") != NULL)
//    }//end of if( m_SegmentationNode->GetPropertyList() != NULL)
//
//    ++m_PixelChangedImageCounter;
//    std::ostringstream buffer;
//    buffer << m_PixelChangedImageCounter;
//    std::string sPName = "PixelChanged Image " + buffer.str();
//    m_PixelChangedImageNode = mitk::DataTreeNode::New();//m_GradientShiftedImageNode = mitk::DataTreeNode::New();
//    m_PixelChangedImageNode->SetData(m_PixelChangedImage);
//    //mitk::DataTreeNodeFactory::SetDefaultImageProperties(m_PixelChangedImageNode);
//    m_PixelChangedImageNode->SetProperty("name", new mitk::StringProperty(sPName ) );
//    m_PixelChangedImageNode->SetIntProperty("layer", 2);
//    (*selectedIterator)->Add(m_PixelChangedImageNode);
//
//  }//end of if(m_SegmentationNode.IsNotNull())
//}//end of ChangePixelValue
