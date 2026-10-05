/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVolumeRenderingScalarRange_h
#define mitkVolumeRenderingScalarRange_h

#include <MitkVolumeVisualizationExports.h>

#include <vtkSmartPointer.h>
#include <vtkType.h>

class vtkImageData;
class vtkVolumeProperty;

namespace mitk
{
  /**
   * \brief A view of an image that reports the given scalar range as its own.
   *
   * The GPU ray caster takes the scalar range from the image's own array and
   * uses it twice: to fill the color and opacity lookup tables, and to turn
   * each sampled voxel into a coordinate in them. Both halves therefore span
   * the whole image, however little of it a transfer function was authored
   * over. One extreme voxel is enough to leave the curve addressed by a few of
   * the table's entries, and to ask for a table past the largest texture the
   * driver will allocate, which it then silently clamps.
   *
   * Handing the ray caster a view reporting the transfer function's own range
   * lines both halves up on that instead. VTK's ScalarOpacityRangeType reaches
   * only the first half and so cannot do this.
   *
   * Which scalar maps to which color and opacity is unchanged, as long as the
   * range covers the outermost nodes of the curves drawn with it: a lookup
   * outside the range clamps to the end of the table, which is the value a
   * transfer function extrapolates there anyway. What changes is how finely
   * the curve is resolved, and that is the point.
   *
   * The view shares the source's voxels rather than copying them, so it must
   * not outlive them. The source is left untouched, and everything else asking
   * it for its scalar range still gets the real one.
   *
   * \param[in] image The image to view; its scalars must have one component.
   * \param[in] min   The lower bound the view reports.
   * \param[in] max   The upper bound the view reports; must exceed \p min.
   * \return A view of \p image over the given range.
   * \throw mitk::Exception The image holds no single-component scalars, or the
   *        range is empty or inverted. A caller with no curve to take a range
   *        from - a binary node, or one carrying no transfer function - is
   *        expected to render the image itself rather than ask for a view of
   *        nothing.
   */
  MITKVOLUMEVISUALIZATION_EXPORT vtkSmartPointer<vtkImageData> ViewWithScalarRange(
    vtkImageData *image, double min, double max);

  /**
   * \brief Holds the ViewWithScalarRange one volume is being drawn through.
   *
   * A view is a distinct input, and the ray caster reloads the entire volume
   * texture whenever the input it is handed is not the one it loaded. Keeping
   * the view across renders is therefore what makes the approach affordable at
   * all, and a cache with one entry is enough: a renderer draws one volume.
   *
   * Views are therefore built wider than asked for, and kept while the curves
   * still fall inside them, which is what stops a slider editing a curve from
   * reloading the volume on every tick it emits. Both stay correct because the
   * range decides how finely the curve is resolved and not what it maps to.
   */
  class MITKVOLUMEVISUALIZATION_EXPORT ScalarRangeViewCache
  {
  public:
    /**
     * \brief The view to render \p image through for the given range.
     *
     * \param[in] image The image to view; must be the one the curves apply to.
     * \param[in] min   The lower bound to cover.
     * \param[in] max   The upper bound to cover; must exceed \p min.
     * \return A view covering at least the range, owned by this cache and
     *         valid until the next call.
     * \throw mitk::Exception As ViewWithScalarRange.
     */
    vtkImageData *GetView(vtkImageData *image, double min, double max);

    /**
     * \brief The image to render \p image through for the curves \p property
     *        carries.
     *
     * The union of the color and opacity ranges, because the ray caster
     * addresses their two lookup tables with a single coordinate and so fills
     * both over one range.
     *
     * \param[in] image The image to render.
     * \param[in] property The property it is about to be rendered with.
     * \return A view covering both curves, or \p image itself where they name
     *         no range to cover - a binary node's color function holds one
     *         node, a node carrying no transfer function none at all - and
     *         where the image holds more than one component per voxel, which
     *         gives the curves no single range to line up with. The ray caster
     *         then spreads the curves over the image's own range, as before.
     */
    vtkImageData *GetView(vtkImageData *image, vtkVolumeProperty *property);

    /**
     * \brief Drop the view held.
     *
     * The view shares the source's voxels, so whoever releases those has to
     * say so: until then the cache holds a view of memory nobody owns.
     */
    void Reset();

  private:
    vtkSmartPointer<vtkImageData> m_View;
    const vtkImageData *m_Source = nullptr;
    vtkMTimeType m_SourceTime = 0;
    double m_Range[2] = {0.0, 0.0};
  };
}

#endif
