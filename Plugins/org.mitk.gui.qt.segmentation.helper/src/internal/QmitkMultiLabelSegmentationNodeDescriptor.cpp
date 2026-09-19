/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiLabelSegmentationNodeDescriptor.h"

#include <mitkLabelSetImage.h>
#include <mitkNodePredicateDataType.h>

#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

QmitkMultiLabelSegmentationNodeDescriptor::QmitkMultiLabelSegmentationNodeDescriptor(QObject *parent)
  // Class name, icon and predicate deliberately mirror the built-in descriptor
  // of QmitkNodeDescriptorManager so that this one is a drop-in replacement.
  : QmitkNodeDescriptor(QStringLiteral("MultiLabelSegmentation"),
                        QStringLiteral(":/Qmitk/SegmentationIcon_48.png"),
                        mitk::NodePredicateDataType::New("MultiLabelSegmentation"),
                        parent)
{
}

QPixmap QmitkMultiLabelSegmentationNodeDescriptor::GenerateThumbnail(const mitk::DataNode *node, int size) const
{
  if (nullptr == node)
    return QPixmap();

  const auto *segmentation = dynamic_cast<const mitk::MultiLabelSegmentation *>(node->GetData());

  if (nullptr == segmentation || 0 == segmentation->GetNumberOfGroups())
    return QmitkNodeDescriptor::GenerateThumbnail(node, size);

  auto lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->DeepCopy(segmentation->GetLookupTable()->GetVtkLookupTable());

  // Render the background label black so that it blends into the thumbnail.
  lookupTable->SetTableValue(0, 0.0, 0.0, 0.0);

  const auto thumbnail = RenderThumbnail(segmentation->GetGroupImage(0), lookupTable, size);

  return thumbnail.isNull()
    ? this->GetIcon(node).pixmap(size, size)
    : thumbnail;
}
