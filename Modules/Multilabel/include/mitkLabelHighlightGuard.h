/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabelHighlightGuard_h
#define mitkLabelHighlightGuard_h

#include <MitkMultilabelExports.h>

#include <mitkDataNode.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  /**
   * \brief RAII guard for managing label highlighting on data nodes.
   *
   * This helper class manages the node properties used for label highlighting
   * in multi-label segmentations. It ensures that highlighting is always
   * properly removed when:
   * - The destructor of the guard is called, or
   * - The segmentation node changes (highlighting on the former node is removed).
   *
   * The guard also triggers the RenderingManager to refresh render windows
   * when highlighting state changes.
   *
   * \sa MultiLabelSegmentation, LabelSetImageVtkMapper2D
   */
  class MITKMULTILABEL_EXPORT LabelHighlightGuard
  {
  public:
    /**
     * \brief Sets the segmentation node on which labels should be highlighted.
     *
     * If a different node was previously set, the highlighting on the old node is
     * removed first. Setting nullptr removes all highlighting.
     *
     * \param[in] node Pointer to the data node, or nullptr to clear highlighting.
     */
    void SetSegmentationNode(DataNode* node);

    /**
     * \brief Returns the currently associated segmentation data node.
     * \return Smart pointer to the segmentation node, or nullptr if none is set.
     */
    DataNode::Pointer GetSegmentationNode() const;

    /**
     * \brief Sets the label values that should be highlighted.
     *
     * Only takes effect if a segmentation node is set. Triggers a render update.
     *
     * \param[in] labels Vector of label values to highlight.
     */
    void SetHighlightedLabels(MultiLabelSegmentation::LabelValueVectorType labels);

    /**
     * \brief Returns the currently highlighted label values.
     * \return Vector of highlighted label values.
     */
    MultiLabelSegmentation::LabelValueVectorType GetHighlightedLabels() const;

    /**
     * \brief Sets whether invisible labels should also be highlighted.
     * \param[in] highlightInvisible If true, labels that are invisible will also be highlighted.
     */
    void SetHighlightInvisibleLabels(bool highlightInvisible);

    /**
     * \brief Destructor. Removes all highlighting from the associated node.
     */
    ~LabelHighlightGuard();

    /**
     * \brief Returns the property name used to store highlighted label values on a data node.
     * \return The property name string "org.mitk.multilabel.labels.highlighted".
     */
    constexpr static const char* PROPERTY_NAME_LABELS_HIGHLIGHTED()
    {
      return "org.mitk.multilabel.labels.highlighted";
    };

    /**
     * \brief Returns the property name used to control highlighting of invisible labels.
     * \return The property name string "org.mitk.multilabel.highlight_invisible".
     */
    constexpr static const char* PROPERTY_NAME_HIGHLIGHT_INVISIBLE()
    {
      return "org.mitk.multilabel.highlight_invisible";
    };

  protected:
    static void UpdateNode(DataNode* node, MultiLabelSegmentation::LabelValueVectorType labels, bool highlightInvisible);
    MultiLabelSegmentation::LabelValueVectorType m_Labels = {};
    bool m_HighlightInvisible = false;
    WeakPointer<DataNode> m_Node;
  };
}

#endif
