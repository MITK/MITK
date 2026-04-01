/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGrowCutTool_h
#define mitkGrowCutTool_h

#include <mitkSegWithPreviewTool.h>
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
   * \brief Interactive GrowCut segmentation tool.
   *
   * Implements the GrowCut algorithm as an interactive segmentation tool with preview.
   * The user provides seed labels (foreground and background) and the algorithm grows
   * the segmentation based on intensity similarity with an optional distance penalty.
   *
   * \ingroup ToolManagerEtAl
   * \sa GrowCutSegmentationFilter, SegWithPreviewTool
   */
  class MITKSEGMENTATION_EXPORT GrowCutTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(GrowCutTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Sets the distance penalty factor for GrowCut. */
    itkSetMacro(DistancePenalty, double);

    /** \brief Returns the distance penalty factor. */
    itkGetConstMacro(DistancePenalty, double);

    /** \brief Type of the seed image used by GrowCut. */
    typedef itk::Image<DefaultSegmentationDataType, 3> SeedImageType;

    /** \brief Index type for the seed image. */
    typedef typename SeedImageType::IndexType IndexType;

    /** \brief Returns the display name of this tool ("GrowCut"). */
    const char *GetName() const override;

    /** \brief Returns the icon resource for this tool's button. */
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void Deactivated() override;

    /**
     * \brief Checks whether the current seed image is valid for running GrowCut.
     * \return true if the seed image contains valid seed labels, false otherwise.
     */
    bool SeedImageIsValid();

  protected:
    GrowCutTool(); // purposely hidden
    ~GrowCutTool() override;

    void DoUpdatePreview(const Image *inputAtTimeStep,
                         const Image *oldSegAtTimeStep,
                         MultiLabelSegmentation *previewImage,
                         TimeStepType timeStep) override;

    double m_DistancePenalty = 0.0;
  };

} // namespace mitk

#endif
