/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkMonaiLabel2DTool_h
#define mitkMonaiLabel2DTool_h

#include <mitkMonaiLabelTool.h>
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief MONAI Label segmentation tool operating on 2D slices.

    \ingroup ToolManagerEtAl
    \sa mitk::MonaiLabelTool
    \sa QmitkInteractiveSegmentation

    This tool sends the current 2D slice to a MONAI Label server for
    AI-based segmentation and writes the result back into the preview
    segmentation volume at the corresponding slice position.
  */
  class MITKSEGMENTATION_EXPORT MonaiLabel2DTool : public MonaiLabelTool
  {
  public:
    mitkClassMacro(MonaiLabel2DTool, MonaiLabelTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Return the human-readable name of this tool ("MONAI Label 2D"). */
    const char *GetName() const override;

    /** \brief Return the toolbar icon resource for this tool. */
    us::ModuleResource GetIconResource() const override;

    /** \brief Activate the tool and configure label transfer scope to all labels. */
    void Activated() override;

    /** \brief Write the input image slice to disk, extending the 2D slice to a 3D volume with depth 1.
      \param inputAtTimeStep The 2D image slice at the current time step.
      \param inputImagePath The file path where the image is saved.
    */
    void WriteImage(const Image *inputAtTimeStep, const std::string &inputImagePath) const override;

    /** \brief Write segmentation results back into the preview image at the working plane.
      \param previewImage The preview segmentation to write into.
      \param segResults The segmentation results returned by the MONAI Label server.
      \param timeStep The time step to write into.
    */
    void WriteBackResults(MultiLabelSegmentation *previewImage, MultiLabelSegmentation *segResults, TimeStepType timeStep) const override;

    /** \brief Clean up preview content and clear seeds after confirmation. */
    void ConfirmCleanUp() override;

  protected:
    MonaiLabel2DTool() = default;
    ~MonaiLabel2DTool() = default;
  };
}
#endif
