/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkWatershedTool_h_Included
#define mitkWatershedTool_h_Included

#include "mitkAutoMLSegmentationWithPreviewTool.h"
#include "mitkCommon.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief Simple watershed segmentation tool.

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    Wraps ITK Watershed Filter into tool concept of MITK. For more information look into ITK documentation.

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT WatershedTool : public AutoMLSegmentationWithPreviewTool
  {
  public:
    mitkClassMacro(WatershedTool, AutoMLSegmentationWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char** GetXPM() const override;
    const char* GetName() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;

    itkSetMacro(Threshold, double);
    itkGetConstMacro(Threshold, double);

    itkSetMacro(Level, double);
    itkGetConstMacro(Level, double);

  protected:
    WatershedTool() = default;
    ~WatershedTool() = default;

    LabelSetImage::Pointer ComputeMLPreview(const Image* inputAtTimeStep, TimeStepType timeStep) override;

    /** \brief Threshold parameter of the ITK Watershed Image Filter. See ITK Documentation for more information. */
    double m_Threshold = 0.0;
    /** \brief Threshold parameter of the ITK Watershed Image Filter. See ITK Documentation for more information. */
    double m_Level = 0.0;

private:
    /** \brief Creates and runs an ITK filter pipeline consisting of the filters: GradientMagnitude-, Watershed- and
     * CastImageFilter.
      *
      * \param originalImage The input image, which is delivered by the AccessByItk macro.
      * \param segmentation A pointer to the output image, which will point to the pipeline output after execution.
      */
    template <typename TPixel, unsigned int VImageDimension>
    void ITKWatershed(const itk::Image<TPixel, VImageDimension>* originalImage, itk::SmartPointer<mitk::Image>& segmentation, bool inputChanged);

    itk::ProcessObject::Pointer m_MagFilter;
    itk::ProcessObject::Pointer m_WatershedFilter;
    mitk::Image::ConstPointer m_LastFilterInput;
  };

} // namespace

#endif
