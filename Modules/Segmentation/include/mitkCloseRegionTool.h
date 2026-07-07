/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCloseRegionTool_h
#define mitkCloseRegionTool_h

#include <mitkFillRegionBaseTool.h>
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief Closes/fills the inside of a contour with the foreground pixel value.

    \sa FillRegionBaseTool

    Finds the outer contour of a shape in 2D (possibly including holes) and sets all
    the pixels inside to the foreground pixel value (filling holes in a segmentation).
    If clicked on the background, the outer contour might contain the whole image and thus
    fill the whole image with the foreground pixel value.

    Uses connected threshold region growing followed by a binary fill-hole filter
    to close internal gaps within the clicked label region.

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT CloseRegionTool : public FillRegionBaseTool
  {
  public:
    mitkClassMacro(CloseRegionTool, FillRegionBaseTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Return the cursor icon resource for this tool. */
    us::ModuleResource GetCursorIconResource() const override;

    /** \brief Return the toolbar icon resource for this tool. */
    us::ModuleResource GetIconResource() const override;

    /** \brief Return the human-readable name of this tool ("Close"). */
    const char *GetName() const override;

  protected:
    CloseRegionTool() = default; // purposely hidden
    ~CloseRegionTool() = default;

    /** \brief Generate a fill image by region-growing from the seed point and closing holes.
      \param workingSlice The current 2D working slice.
      \param seedPoint The seed position in world coordinates.
      \param seedLabelValue Output parameter receiving the label value at the seed point.
      \return A binary fill image, or nullptr if the seed is on unlabeled background.
    */
    Image::Pointer GenerateFillImage(const Image* workingSlice, Point3D seedPoint, mitk::Label::PixelType& seedLabelValue) const override;

    /** \brief Configure fill parameters (label value and merge style) before applying the fill. */
    void PrepareFilling(const Image* workingSlice, Point3D seedPoint) override;

  };

} // namespace

#endif
