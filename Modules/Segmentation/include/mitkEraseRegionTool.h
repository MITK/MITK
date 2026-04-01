/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEraseRegionTool_h
#define mitkEraseRegionTool_h

#include <mitkFillRegionBaseTool.h>
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief Erase a connected region by filling it with the background (unlabeled) pixel value.

    \sa FillRegionBaseTool

    \ingroup Interactions

    Finds the outer contour of a shape in 2D (possibly including single patches) and sets all
    the pixels inside to the background pixel value (erasing a segmentation).
    Clicking on an unlabeled (background) pixel has no effect.

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT EraseRegionTool : public FillRegionBaseTool
  {
  public:
    mitkClassMacro(EraseRegionTool, FillRegionBaseTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Return the cursor icon resource for this tool. */
    us::ModuleResource GetCursorIconResource() const override;

    /** \brief Return the toolbar icon resource for this tool. */
    us::ModuleResource GetIconResource() const override;

    /** \brief Return the human-readable name of this tool ("Erase"). */
    const char *GetName() const override;

  protected:
    EraseRegionTool() = default; // purposely hidden
    ~EraseRegionTool() = default;

    /** \brief Generate a fill image by region-growing from the seed point.
      \param workingSlice The current 2D working slice.
      \param seedPoint The seed position in world coordinates.
      \param seedLabelValue Output parameter receiving the label value at the seed point.
      \return A binary fill image, or nullptr if the seed is on unlabeled background.
    */
    Image::Pointer GenerateFillImage(const Image* workingSlice, Point3D seedPoint, mitk::Label::PixelType& seedLabelValue) const override;

    /** \brief Configure fill parameters to erase (set fill label to UNLABELED_VALUE). */
    void PrepareFilling(const Image* workingSlice, Point3D seedPoint) override;
  };

} // namespace

#endif
