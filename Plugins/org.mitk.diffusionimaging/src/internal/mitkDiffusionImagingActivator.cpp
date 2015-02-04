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

#include "mitkDiffusionImagingActivator.h"

#include "QmitkNodeDescriptorManager.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateIsDWI.h"
#include <mitkDiffusionPropertyHelper.h>

#include <QtPlugin>

void
mitk::DiffusionImagingActivator::start(ctkPluginContext* context)
{
  Q_UNUSED(context)

  QmitkNodeDescriptorManager* manager =
    QmitkNodeDescriptorManager::GetInstance();

  mitk::NodePredicateIsDWI::Pointer isDiffusionImage = mitk::NodePredicateIsDWI::New();
  QmitkNodeDescriptor* desc = new QmitkNodeDescriptor(QObject::tr("DiffusionImage"), QString(":/QmitkDiffusionImaging/QBallData24.png"), isDiffusionImage, manager);
  manager->AddDescriptor(desc);

  mitk::NodePredicateDataType::Pointer isTensorImage = mitk::NodePredicateDataType::New("TensorImage");
  manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("TensorImage"), QString(":/QmitkDiffusionImaging/recontensor.png"), isTensorImage, manager));

  mitk::NodePredicateDataType::Pointer isQBallImage = mitk::NodePredicateDataType::New("QBallImage");
  manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("QBallImage"), QString(":/QmitkDiffusionImaging/reconodf.png"), isQBallImage, manager));

  mitk::NodePredicateDataType::Pointer isFiberBundle = mitk::NodePredicateDataType::New("FiberBundle");
  manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("FiberBundle"), QString(":/QmitkDiffusionImaging/FiberBundle.png"), isFiberBundle, manager));

  mitk::NodePredicateDataType::Pointer isConnectomicsNetwork = mitk::NodePredicateDataType::New("ConnectomicsNetwork");
  manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("ConnectomicsNetwork"), QString(":/QmitkDiffusionImaging/ConnectomicsNetwork.png"), isConnectomicsNetwork, manager));

}

void
mitk::DiffusionImagingActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_diffusionimaging, mitk::DiffusionImagingActivator)
#endif
