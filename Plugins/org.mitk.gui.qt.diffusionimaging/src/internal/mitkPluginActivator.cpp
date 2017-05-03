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
#include "mitkPluginActivator.h"

#include "src/internal/Perspectives/QmitkDiffusionImagingAppPerspective.h"
#include "src/internal/Perspectives/QmitkDiffusionDefaultPerspective.h"

#include "src/internal/QmitkDiffusionDicomImportView.h"
#include "src/internal/QmitkControlVisualizationPropertiesView.h"

#include "QmitkNodeDescriptorManager.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateIsDWI.h"
#include <mitkDiffusionPropertyHelper.h>

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
    BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingAppPerspective, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionDefaultPerspective, context)

    BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionDicomImport, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkControlVisualizationPropertiesView, context)

            //            Q_UNUSED(context)

    QmitkNodeDescriptorManager* manager = QmitkNodeDescriptorManager::GetInstance();

    mitk::NodePredicateIsDWI::Pointer isDiffusionImage = mitk::NodePredicateIsDWI::New();
    QmitkNodeDescriptor* desc = new QmitkNodeDescriptor(QObject::tr("DiffusionImage"), QString(":/QmitkDiffusionImaging/QBallData24.png"), isDiffusionImage, manager);
    manager->AddDescriptor(desc);

    mitk::NodePredicateDataType::Pointer isTensorImage = mitk::NodePredicateDataType::New("TensorImage");
    manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("TensorImage"), QString(":/QmitkDiffusionImaging/tensor.png"), isTensorImage, manager));

    mitk::NodePredicateDataType::Pointer isQBallImage = mitk::NodePredicateDataType::New("QBallImage");
    manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("QBallImage"), QString(":/QmitkDiffusionImaging/qball.png"), isQBallImage, manager));

    mitk::NodePredicateDataType::Pointer isFiberBundle = mitk::NodePredicateDataType::New("FiberBundle");
    manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("FiberBundle"), QString(":/QmitkDiffusionImaging/tractogram.png"), isFiberBundle, manager));

    mitk::NodePredicateDataType::Pointer isConnectomicsNetwork = mitk::NodePredicateDataType::New("ConnectomicsNetwork");
    manager->AddDescriptor(new QmitkNodeDescriptor(QObject::tr("ConnectomicsNetwork"), QString(":/QmitkDiffusionImaging/ConnectomicsNetwork.png"), isConnectomicsNetwork, manager));
}

void PluginActivator::stop(ctkPluginContext* context)
{
    Q_UNUSED(context)
}

}
