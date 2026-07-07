/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkComputeContourSetNormalsFilter_h
#define mitkComputeContourSetNormalsFilter_h

#include <mitkProgressBar.h>
#include <mitkSurface.h>
#include <mitkSurfaceToSurfaceFilter.h>
#include <MitkSurfaceInterpolationExports.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkLine.h>
#include <vtkMath.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <mitkImage.h>

namespace mitk
{
  /** \brief Filter that computes surface normals for contour edge points.
   *
   * Takes a set of contour surfaces (represented as vtkPolygon-based mitk::Surface objects)
   * and computes normal vectors for each contour edge point. The computed normals
   * can be accessed on the output surfaces via:
   * \code
   * filter->GetOutput(i)->GetVtkPolyData()->GetCellData()->GetNormals();
   * \endcode
   *
   * If a segmentation binary image is provided via SetSegmentationBinaryImage(),
   * the filter ensures that the computed normals point away from (not into)
   * the segmented region.
   *
   * \sa ReduceContourSetFilter, CreateDistanceImageFromSurfaceFilter, SurfaceInterpolationController
   */
  class MITKSURFACEINTERPOLATION_EXPORT ComputeContourSetNormalsFilter : public SurfaceToSurfaceFilter
  {
  public:
    mitkClassMacro(ComputeContourSetNormalsFilter, SurfaceToSurfaceFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief Return the computed normals as a mitk::Surface.
       *
       * The returned surface contains the normal vectors as cell data.
       *
       * \return Surface containing the computed normal vectors.
       */
      mitk::Surface::Pointer GetNormalsAsSurface();

    /** \brief Reset the filter by removing all inputs and outputs. */
    void Reset();

    /** \brief Set the maximum voxel spacing of the original image.
     *
     * Used to determine the sampling distance for normal computation.
     *
     * \param[in] maxSpacing The maximum spacing value.
     */
    void SetMaxSpacing(double maxSpacing);

    /** \brief Enable or disable the MITK progress bar during computation.
     * \param[in] status True to use the progress bar, false to disable it.
     */
    void SetUseProgressBar(bool status);

    /** \brief Set the step size by which the progress bar advances per contour.
     * \param[in] stepSize The progress increment per processed contour.
     */
    void SetProgressStepSize(unsigned int stepSize);

    /** \brief Set the segmentation binary image for normal direction validation.
     *
     * When set, the filter checks that computed normals point outward (away from
     * the segmented region) and flips them if necessary.
     *
     * \param[in] segmentationImage Pointer to the binary segmentation image.
     */
    void SetSegmentationBinaryImage(mitk::Image *segmentationImage) { m_SegmentationBinaryImage = segmentationImage; }
  protected:
    ComputeContourSetNormalsFilter();
    ~ComputeContourSetNormalsFilter() override;
    void GenerateData() override;
    void GenerateOutputInformation() override;

  private:
    // The segmentation out of which the contours were extracted. Can be used to determine the direction of the normals
    mitk::Image::Pointer m_SegmentationBinaryImage;
    double m_MaxSpacing;

    unsigned int m_NegativeNormalCounter;
    unsigned int m_PositiveNormalCounter;

    bool m_UseProgressBar;
    unsigned int m_ProgressStepSize;

  }; // class

} // namespace
#endif
