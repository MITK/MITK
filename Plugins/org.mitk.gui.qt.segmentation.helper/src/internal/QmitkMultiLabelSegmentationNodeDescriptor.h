/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiLabelSegmentationNodeDescriptor_h
#define QmitkMultiLabelSegmentationNodeDescriptor_h

#include <QmitkNodeDescriptor.h>

/**
 * \brief Node descriptor for multi-label segmentations.
 *
 * Behaves like the built-in "MultiLabelSegmentation" descriptor of
 * QmitkNodeDescriptorManager, except that thumbnails are rendered from the
 * first group of the segmentation using its label colors instead of falling
 * back to the icon.
 *
 * \sa QmitkNodeDescriptor
 * \sa QmitkNodeDescriptorManager
 */
class QmitkMultiLabelSegmentationNodeDescriptor final : public QmitkNodeDescriptor
{
  Q_OBJECT

public:
  explicit QmitkMultiLabelSegmentationNodeDescriptor(QObject *parent = nullptr);

  /**
   * \brief Returns a thumbnail of the first group of the segmentation.
   *
   * Falls back to the base class implementation if the node does not hold a
   * multi-label segmentation with at least one group.
   */
  QPixmap GenerateThumbnail(const mitk::DataNode *node, int size) const override;
};

#endif
