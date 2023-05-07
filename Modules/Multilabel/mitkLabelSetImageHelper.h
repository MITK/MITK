/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabelSetImageHelper_h
#define mitkLabelSetImageHelper_h

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
     * @remark The data is not set. Set it manually to have a properly setup node.
     * @param segmentationName A name for the new segmentation node.
     * @return The new segmentation node as a data node pointer.
     */
    MITKMULTILABEL_EXPORT mitk::DataNode::Pointer CreateEmptySegmentationNode(const std::string& segmentationName = std::string());

    /**
     * @brief This function creates and returns a new data node with a new empty segmentation
     * data structure.
     * The segmentation node is named according to the given reference data node, otherwise a name
     * is passed explicitly.
     * Some properties are set to ensure a proper setup segmentation and node
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
     * @param namePrefix      The prefix of the label name that is prepended by a sequential number
     *
     * @return                The new label.
     */
    MITKMULTILABEL_EXPORT mitk::Label::Pointer CreateNewLabel(const LabelSetImage* labelSetImage, const std::string& namePrefix = "Label");

  } // namespace LabelSetImageHelper
} // namespace mitk

#endif
