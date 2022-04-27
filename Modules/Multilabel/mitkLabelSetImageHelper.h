/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKLABELSETIMAGEHELPER_H
#define MITKLABELSETIMAGEHELPER_H

#include <MitkMultilabelExports.h>

#include <mitkDataNode.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
   *
   */
  namespace LabelSetImageHelper
  {
    /**
     * @brief This function creates and returns a new empty segmentation data node.
     * @remark The data is not set, and must be done "by hand" to have a proper setup node.
     * @param segmentationName          An name for the new segmentation node.
     * @return                          The new segmentation node as a data node pointer.
     */
    MITKMULTILABEL_EXPORT mitk::DataNode::Pointer CreateEmptySegmentationNode(const std::string& segmentationName = std::string());

    /**
     * @brief This function creates and returns a new data node with a new empty segmentation
     * data structure.
     * The segmentation node is named according to the given reference data node, if not a name
     * is passed explicitly.
     * Some properties are set to automatically to ensure a proper setup segmentation and node
     * (e.g. link the segmentation node with its parent node).
     *
     * @param referenceNode             The reference node from which the name of the new segmentation node
     *                                  is derived.
     * @param initialSegmentationImage  The segmentation image that is used to initialize the label set image.
     * @param segmentationName          An optional name for the new segmentation node.
     *
     * @return                          The new segmentation node as a data node pointer.
     */
    MITKMULTILABEL_EXPORT mitk::DataNode::Pointer CreateNewSegmentationNode(const DataNode* referenceNode,
      const Image* initialSegmentationImage = nullptr, const std::string& segmentationName = std::string());

    /**
     * @brief This function creates and returns a new label. The label is automatically assigned an
     *        unused generic label name, depending on existing label names in all label sets of the
     *        given label set image.
     *        The color of the label is selected from the MULTILABEL lookup table, following the same
     *        rules of the naming to likely chose a unique color.
     *
     * @param labelSetImage   The label set image that the new label is added to
     *
     * @return                The new label.
     */
    MITKMULTILABEL_EXPORT mitk::Label::Pointer CreateNewLabel(const LabelSetImage* labelSetImage);

  } // namespace LabelSetImageHelper
} // namespace mitk

#endif // MITKLABELSETIMAGEHELPER_H
