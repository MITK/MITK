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
  /** Helper class that allows to manage the node properties for label highlighting.
   * Using the class ensures that the highlighting will always be removed if needed:
   * (a) the destructor of the guard is called, or (b) the node changes (highlight
   * of former node will be removed). The guard also manages the triggering the
   * RenderWindowManager, if need, to refresh the render windows.
   */
  class MITKMULTILABEL_EXPORT LabelHighlightGuard
  {
  public:
    void SetSegmentationNode(DataNode* node);
    DataNode::Pointer GetSegmentationNode() const;

    void SetHighlightedLabels(MultiLabelSegmentation::LabelValueVectorType labels);
    void SetHighlightInvisibleLabels(bool highlightInvisible);

    ~LabelHighlightGuard();

    constexpr static const char* PROPERTY_NAME_LABELS_HIGHLIGHTED()
    {
      return "org.mitk.multilabel.labels.highlighted";
    };

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
