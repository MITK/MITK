/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkMultiLabelSegmentationNodeDescriptor.h"

#include <QmitkNodeDescriptorManager.h>

void mitk::PluginActivator::start(ctkPluginContext *)
{
  auto *nodeDescriptorManager = QmitkNodeDescriptorManager::GetInstance();

  // Replace the built-in descriptor instead of registering a second one under
  // the same class name: QmitkNodeDescriptorManager resolves both by class name
  // and by node, and both lookups return the last match, so two descriptors for
  // the same data type would make it depend on registration order which one owns
  // the actions that QmitkDataNodeContextMenu attaches by class name.
  //
  // This plug-in activates eagerly, so the swap happens before any view exists
  // and nothing can have resolved or decorated the descriptor being removed.
  if (auto *builtInDescriptor = nodeDescriptorManager->GetDescriptor(QStringLiteral("MultiLabelSegmentation")))
    nodeDescriptorManager->RemoveDescriptor(builtInDescriptor);

  nodeDescriptorManager->AddDescriptor(new QmitkMultiLabelSegmentationNodeDescriptor(nodeDescriptorManager));
}

void mitk::PluginActivator::stop(ctkPluginContext *)
{
  // The descriptor stays registered. CTK never unloads a plug-in library, so it
  // remains callable, and unregistering it would leave multi-label segmentations
  // without any descriptor at all.
}
