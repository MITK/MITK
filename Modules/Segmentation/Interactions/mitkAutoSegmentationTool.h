/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAutoSegmentationTool_h_Included
#define mitkAutoSegmentationTool_h_Included

#include "mitkCommon.h"
#include "mitkTool.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  class Image;

  /**
    \brief Superclass for tool that create a new segmentation without user interaction in render windows

    This class is undocumented. Ask the creator ($Author$) to supply useful comments.
  */
  class MITKSEGMENTATION_EXPORT AutoSegmentationTool : public Tool
  {
  public:
    mitkClassMacro(AutoSegmentationTool, Tool);

    void Activated() override;
    void Deactivated() override;

    /** This function controls wether a confirmed segmentation should replace the old
    * segmentation/working node (true) or if it should be stored as new and additional
    * node (false).
    */
    void SetOverwriteExistingSegmentation(bool overwrite);

    /**
     * @brief Gets the name of the currently selected segmentation node
     * @return the name of the segmentation node or an empty string if
     *         none is selected
     */
    std::string GetCurrentSegmentationName();

    /**
     * @brief Depending on the selected mode either returns the currently selected segmentation node
     *        or (if overwrite mode is false) creates a new one from the selected reference data.
     * @remark Please keep in mind that new created nodes are not automatically added to the data storage.
     * Derived tools can call EnsureTargetSegmentationNodeInDataStorage to ensure it as soon as it is clear
     * that the target segmentation node will be/is confirmed.
     * @return a mitk::DataNode which contains a segmentation image
     */
    virtual DataNode *GetTargetSegmentationNode() const;

  protected:
    AutoSegmentationTool(); // purposely hidden
    AutoSegmentationTool(const char* interactorType, const us::Module* interactorModule = nullptr); // purposely hidden
    ~AutoSegmentationTool() override;

    const char *GetGroup() const override;

    /** Helper that extracts the image for the passed timestep, if the image has multiple time steps.*/
    static Image::ConstPointer GetImageByTimeStep(const Image* image, unsigned int timestep);
    /** Helper that extracts the image for the passed time point, if the image has multiple time steps.*/
    static Image::ConstPointer GetImageByTimePoint(const Image* image, TimePointType timePoint);

    void EnsureTargetSegmentationNodeInDataStorage() const;

    bool m_OverwriteExistingSegmentation;

  private:
    /**Contains the node returned by GetTargetSementationNode if m_OverwriteExistingSegmentation == false. Then
    * GetTargetSegmentation generates a new target segmentation node.*/
    mutable DataNode::Pointer m_NoneOverwriteTargetSegmentationNode;
  };

} // namespace

#endif
