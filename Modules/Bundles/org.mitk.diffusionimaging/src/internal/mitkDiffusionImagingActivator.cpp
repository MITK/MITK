/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date: 2009-05-18 17:57:41 +0200 (Mo, 18 Mai 2009) $
Version:   $Revision: 17020 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkDiffusionImagingActivator.h"
#include "mitkDiffusionImagingObjectFactory.h"

#include "QmitkNodeDescriptorManager.h"
#include "mitkNodePredicateDataType.h"
//#include "mitkDataNodeObject.h"
//
//#include "berryISelectionService.h"
//#include "berryIStructuredSelection.h"
//#include "berryPlatformUI.h"
//
//#include <QLabel>
//#include <QHBoxLayout>

void
mitk::DiffusionImagingActivator::Start(berry::IBundleContext::Pointer /*context*/)
{
  RegisterDiffusionImagingObjectFactory();

  QmitkNodeDescriptorManager* manager = 
    QmitkNodeDescriptorManager::GetInstance();

  mitk::NodePredicateDataType::Pointer isDiffusionImage = mitk::NodePredicateDataType::New("DiffusionImage");
  QmitkNodeDescriptor* desc = new QmitkNodeDescriptor(QObject::tr("DiffusionImage"), QString(":/QmitkDiffusionImaging/QBallData24.png"), isDiffusionImage, manager);
  manager->AddDescriptor(desc);

  mitk::NodePredicateDataType::Pointer isTensorImage = mitk::NodePredicateDataType::New("TensorImage");
  manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("TensorImage"), QString(":/QmitkDiffusionImaging/recontensor.png"), isTensorImage, manager));

  mitk::NodePredicateDataType::Pointer isQBallImage = mitk::NodePredicateDataType::New("QBallImage");
  manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("QBallImage"), QString(":/QmitkDiffusionImaging/reconodf.png"), isQBallImage, manager));

  //m_OpacitySlider = new QSlider;
  //m_OpacitySlider->setMinimum(0);
  //m_OpacitySlider->setMaximum(100);
  //m_OpacitySlider->setOrientation(Qt::Horizontal);
  //QObject::connect( m_OpacitySlider, SIGNAL( valueChanged(int) )
  //  , this, SLOT( OpactiyChanged(int) ) );

  //QLabel* _OpacityLabel = new QLabel("Channel: ");
  //QHBoxLayout* _OpacityWidgetLayout = new QHBoxLayout;
  //_OpacityWidgetLayout->addWidget(_OpacityLabel);
  //_OpacityWidgetLayout->addWidget(m_OpacitySlider);
  //QWidget* _OpacityWidget = new QWidget;
  //_OpacityWidget->setLayout(_OpacityWidgetLayout);

  //QWidgetAction* m_OpacityAction = new QWidgetAction(this);
  //m_OpacityAction->setDefaultWidget(_OpacityWidget);
  //QObject::connect( m_OpacityAction, SIGNAL( changed() )
  //  , this, SLOT( OpactiyActionChanged() ) );

  //  desc->AddAction(m_OpacityAction, false);

}

//void mitk::DiffusionImagingActivator::OpactiyChanged(int value)
//{
//  berry::ISelection::ConstPointer sel(
//    berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
//  berry::IStructuredSelection::ConstPointer ssel = sel.Cast<const berry::IStructuredSelection>();
//
//  if (ssel)
//  {
//    if (mitk::DataNodeObject::Pointer nodeObj = ssel->Begin()->Cast<mitk::DataNodeObject>())
//    {
//      mitk::DataNode::Pointer node = nodeObj->GetDataNode();
//      if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
//      {
//        node->SetIntProperty("DisplayChannel", value);
//        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
//      }
//    }
//  }
//}
//
//void mitk::DiffusionImagingActivator::OpactiyActionChanged()
//{
//  berry::ISelection::ConstPointer sel(
//    berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
//  berry::IStructuredSelection::ConstPointer ssel = sel.Cast<const berry::IStructuredSelection>();
// 
//  if (ssel)
//  {
//    if (mitk::DataNodeObject::Pointer nodeObj = ssel->Begin()->Cast<mitk::DataNodeObject>())
//    {
//      mitk::DataNode::Pointer node = nodeObj->GetDataNode();
//      if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
//      {
//        int displayChannel = 0.0;
//        if(node->GetIntProperty("DisplayChannel", displayChannel))
//        {
//          m_OpacitySlider->setValue(displayChannel);
//        }
//      }
//    }
//  }
//
//  MITK_INFO << "changed";
//}