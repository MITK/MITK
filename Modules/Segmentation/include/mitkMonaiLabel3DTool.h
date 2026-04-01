/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkMonaiLabel3DTool_h
#define mitkMonaiLabel3DTool_h

#include <mitkMonaiLabelTool.h>
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief MONAI Label segmentation tool operating on 3D volumes.

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \sa mitk::MonaiLabelTool

    This tool sends the full 3D volume to a MONAI Label server for
    AI-based segmentation and writes the result back into the preview
    segmentation volume.

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT MonaiLabel3DTool : public MonaiLabelTool
  {
  public:
    mitkClassMacro(MonaiLabel3DTool, MonaiLabelTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Return the human-readable name of this tool ("MONAI Label 3D"). */
    const char *GetName() const override;

    /** \brief Return the toolbar icon resource for this tool. */
    us::ModuleResource GetIconResource() const override;

    /** \brief Activate the tool and configure label transfer scope to all labels. */
    void Activated() override;

    /** \brief Write the input 3D image to disk.
      \param inputAtTimeStep The 3D image at the current time step.
      \param inputImagePath The file path where the image is saved.
    */
    void WriteImage(const Image *inputAtTimeStep, const std::string &inputImagePath) const override;

    /** \brief Write segmentation results back into the preview image for the given time step.
      \param previewImage The preview segmentation to write into.
      \param segResults The segmentation results returned by the MONAI Label server.
      \param timeStep The time step to write into.
    */
    void WriteBackResults(MultiLabelSegmentation *previewImage, MultiLabelSegmentation *segResults, TimeStepType timeStep) const override;

  protected:
    MonaiLabel3DTool() = default;
    ~MonaiLabel3DTool() = default;
  };
} // namespace mitk
#endif
