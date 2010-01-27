/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2009-05-12 20:04:59 +0200 (Tue, 12 May 2009) $
Version:   $Revision: 17180 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkExtRegisterClasses.h"

#include "QmitkRenderingManagerFactory.h"

#include "QmitkBinaryThresholdToolGUIFactory.h"
#include "QmitkCalculateGrayValueStatisticsToolGUIFactory.h"
#include "QmitkDrawPaintbrushToolGUIFactory.h"
#include "QmitkErasePaintbrushToolGUIFactory.h"

#include "QmitkCallbackFromGUIThread.h"
#include "QmitkNodeDescriptorManager.h"

#include "mitkProperties.h"

#include <QtCore>

#include <iostream>

void QmitkExtRegisterClasses()
{
  static bool alreadyDone = false;
  if (!alreadyDone)
  {
    MITK_INFO << "QmitkExtRegisterClasses()";
    
    itk::ObjectFactoryBase::RegisterFactory( QmitkBinaryThresholdToolGUIFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( QmitkCalculateGrayValueStatisticsToolGUIFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( QmitkDrawPaintbrushToolGUIFactory::New() );
    itk::ObjectFactoryBase::RegisterFactory( QmitkErasePaintbrushToolGUIFactory::New() );

    static QmitkCallbackFromGUIThread globalQmitkCallbackFromGUIThread;  
    
    QmitkNodeDescriptorManager* descriptorManager = QmitkNodeDescriptorManager::GetInstance();
    
    // Adding "PlanarLine"
    mitk::NodePredicateDataType::Pointer isPlanarLine = mitk::NodePredicateDataType::New("PlanarLine");
    descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarLine"), QString(":/Qmitk/PlanarLine_48.png"), isPlanarLine, descriptorManager));

    // Adding "PlanarCircle"
    mitk::NodePredicateDataType::Pointer isPlanarCircle = mitk::NodePredicateDataType::New("PlanarCircle");
    descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarCircle"), QString(":/Qmitk/PlanarCircle_48.png"), isPlanarCircle, descriptorManager));
    
    // Adding "PlanarAngle"
    mitk::NodePredicateDataType::Pointer isPlanarAngle = mitk::NodePredicateDataType::New("PlanarAngle");
    descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarAngle"), QString(":/Qmitk/PlanarAngle_48.png"), isPlanarAngle, descriptorManager));
    
    // Adding "PlanarFourPointAngle"
    mitk::NodePredicateDataType::Pointer isPlanarFourPointAngle = mitk::NodePredicateDataType::New("PlanarFourPointAngle");
    descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarFourPointAngle"), QString(":/Qmitk/PlanarFourPointAngle_48.png"), isPlanarFourPointAngle, descriptorManager));
    
    // Adding "PlanarRectangle"
    mitk::NodePredicateDataType::Pointer isPlanarRectangle = mitk::NodePredicateDataType::New("PlanarRectangle");
    descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarRectangle"), QString(":/Qmitk/PlanarRectangle_48.png"), isPlanarRectangle, descriptorManager));
    
    // Adding "PlanarPolygon"
    mitk::NodePredicateDataType::Pointer isPlanarPolygon = mitk::NodePredicateDataType::New("PlanarPolygon");
    descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarPolygon"), QString(":/Qmitk/PlanarPolygon_48.png"), isPlanarPolygon, descriptorManager));
    
    // Adding "PlanarPath"
    mitk::NodePredicateProperty::Pointer isNotClosedPolygon
    = mitk::NodePredicateProperty::New("ClosedPlanarPolygon", mitk::BoolProperty::New(false));
    mitk::NodePredicateAND::Pointer isPlanarPath = mitk::NodePredicateAND::New(isNotClosedPolygon, isPlanarPolygon);
    descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarPath"), QString(":/Qmitk/PlanarPath_48.png"), isPlanarPath, descriptorManager));
    

    alreadyDone = true;
  }
}

