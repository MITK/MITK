/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkPlanarFigureActivator.h"
#include "mitkPlanarFigureObjectFactory.h"

#include "QmitkNodeDescriptorManager.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateAND.h"


void
mitk::PlanarFigureActivator::Start(berry::IBundleContext::Pointer /*context*/)
{
  RegisterPlanarFigureObjectFactory();

  QmitkNodeDescriptorManager* descriptorManager = QmitkNodeDescriptorManager::GetInstance();
  
  // Adding "PlanarLine"
  mitk::NodePredicateDataType::Pointer isPlanarLine = mitk::NodePredicateDataType::New("PlanarLine");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarLine"), QString(":/QmitkExt/PlanarLine_48.png"), isPlanarLine, descriptorManager));

  // Adding "PlanarCircle"
  mitk::NodePredicateDataType::Pointer isPlanarCircle = mitk::NodePredicateDataType::New("PlanarCircle");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarCircle"), QString(":/QmitkExt/PlanarCircle_48.png"), isPlanarCircle, descriptorManager));
  
  // Adding "PlanarAngle"
  mitk::NodePredicateDataType::Pointer isPlanarAngle = mitk::NodePredicateDataType::New("PlanarAngle");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarAngle"), QString(":/QmitkExt/PlanarAngle_48.png"), isPlanarAngle, descriptorManager));
  
  // Adding "PlanarFourPointAngle"
  mitk::NodePredicateDataType::Pointer isPlanarFourPointAngle = mitk::NodePredicateDataType::New("PlanarFourPointAngle");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarFourPointAngle"), QString(":/QmitkExt/PlanarFourPointAngle_48.png"), isPlanarFourPointAngle, descriptorManager));
  
  // Adding "PlanarRectangle"
  mitk::NodePredicateDataType::Pointer isPlanarRectangle = mitk::NodePredicateDataType::New("PlanarRectangle");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarRectangle"), QString(":/QmitkExt/PlanarRectangle_48.png"), isPlanarRectangle, descriptorManager));
  
  // Adding "PlanarPolygon"
  mitk::NodePredicateDataType::Pointer isPlanarPolygon = mitk::NodePredicateDataType::New("PlanarPolygon");
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarPolygon"), QString(":/QmitkExt/PlanarPolygon_48.png"), isPlanarPolygon, descriptorManager));
  
  // Adding "PlanarPath"
  mitk::NodePredicateProperty::Pointer isNotClosedPolygon = mitk::NodePredicateProperty::New("ClosedPlanarPolygon", mitk::BoolProperty::New(false));
  mitk::NodePredicateAND::Pointer isPlanarPath = mitk::NodePredicateAND::New(isNotClosedPolygon, isPlanarPolygon);
  descriptorManager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("PlanarPath"), QString(":/QmitkExt/PlanarPath_48.png"), isPlanarPath, descriptorManager));

}
