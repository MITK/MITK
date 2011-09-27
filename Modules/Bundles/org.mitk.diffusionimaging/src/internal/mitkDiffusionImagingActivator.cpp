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

#include "mitkDiffusionImagingActivator.h"
#include "mitkDiffusionImagingObjectFactory.h"

#include "QmitkNodeDescriptorManager.h"
#include "mitkNodePredicateDataType.h"

#include <QtPlugin>

void
mitk::DiffusionImagingActivator::start(ctkPluginContext* context)
{
  Q_UNUSED(context)
  
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

  mitk::NodePredicateDataType::Pointer isFiberBundle = mitk::NodePredicateDataType::New("FiberBundle");
  manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("FiberBundle"), QString(":/QmitkDiffusionImaging/FiberBundle.png"), isFiberBundle, manager));
  
  mitk::NodePredicateDataType::Pointer isFiberBundleX = mitk::NodePredicateDataType::New("FiberBundleX");
  manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("FiberBundleX"), QString(":/QmitkDiffusionImaging/FiberBundleX.png"), isFiberBundleX, manager));

}

void
mitk::DiffusionImagingActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

Q_EXPORT_PLUGIN2(org_mitk_diffusionimaging, mitk::DiffusionImagingActivator)
