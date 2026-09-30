/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkVolumeRenderingScalarRange.h>

#include <mitkExceptionMacro.h>

#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkVolumeProperty.h>

#include <algorithm>

namespace
{
  /** \brief Report the given range as the array's own, in place of its data's.
   *
   * Seeds the per-component ranges VTK caches on first use, which it then
   * trusts without ever comparing them against the data again.
   */
  void ReportRange(vtkDataArray *array, double min, double max)
  {
    const double range[2] = {min, max};

    // Last, and never to be followed by anything that modifies the array:
    // vtkAbstractArray::Modified() drops both keys, and the array would go back
    // to reporting the range of its data without saying so.
    for (auto *key : {vtkDataArray::PER_COMPONENT(), vtkDataArray::PER_FINITE_COMPONENT()})
    {
      vtkNew<vtkInformationVector> perComponent;
      perComponent->SetNumberOfInformationObjects(1);
      perComponent->GetInformationObject(0)->Set(vtkDataArray::COMPONENT_RANGE(), range, 2);
      array->GetInformation()->Set(key, perComponent);
    }
  }
}

vtkSmartPointer<vtkImageData> mitk::ViewWithScalarRange(vtkImageData *image, double min, double max)
{
  if (nullptr == image)
    mitkThrow() << "Cannot build a scalar range view of a null image.";

  auto *scalars = image->GetPointData()->GetScalars();

  if (nullptr == scalars)
    mitkThrow() << "Cannot build a scalar range view of an image without scalars.";

  // One information object is seeded below, and VTK reads the one belonging to
  // the component it asks about. A further component would find none there.
  if (1 != scalars->GetNumberOfComponents())
  {
    mitkThrow() << "Cannot build a scalar range view of scalars with "
                << scalars->GetNumberOfComponents() << " components; one is supported.";
  }

  if (!(min < max))
    mitkThrow() << "Cannot build a scalar range view over the empty range [" << min << ", " << max << "].";

  auto view = vtkSmartPointer<vtkImageData>::New();
  view->ShallowCopy(image);

  // The copy leaves the view holding the source's own array, and the range is
  // kept on the array rather than on the image. A second array over the same
  // voxels is what carries the reported range without copying them, and
  // without the source being changed under everything else that reads it.
  auto reported = vtkSmartPointer<vtkDataArray>::Take(scalars->NewInstance());
  reported->SetNumberOfComponents(scalars->GetNumberOfComponents());
  reported->SetVoidArray(scalars->GetVoidPointer(0), scalars->GetNumberOfValues(), 1);
  reported->SetName(scalars->GetName());
  ReportRange(reported, min, max);

  view->GetPointData()->SetScalars(reported);

  return view;
}

vtkImageData *mitk::ScalarRangeViewCache::GetView(vtkImageData *image, double min, double max)
{
  if (nullptr == image)
    mitkThrow() << "Cannot build a scalar range view of a null image.";

  // Covering the range is enough, and a view wider than it costs no accuracy:
  // the ray caster sizes its lookup table as the range over the closest pair of
  // nodes in the curve, so a wider range buys a proportionally wider table
  // rather than a coarser curve. The second bound is where that stops holding,
  // the table being clamped to the largest texture the driver will allocate -
  // which is the state this view exists to avoid in the first place.
  const bool covered = m_View != nullptr &&
                       m_Source == image &&
                       m_SourceTime == image->GetMTime() &&
                       m_Range[0] <= min && max <= m_Range[1] &&
                       m_Range[1] - m_Range[0] <= 4.0 * (max - min);

  if (!covered)
  {
    // Wider than asked for, because a curve is edited by dragging and every
    // replaced view costs the ray caster a reload of the whole volume: a
    // slider moving the curve across the data would otherwise reload on each
    // tick it emits. The slack is what those ticks are spent against.
    const double slack = 0.5 * (max - min);

    m_View = ViewWithScalarRange(image, min - slack, max + slack);
    m_Source = image;
    // An image allocated later where this one stood would compare equal to it.
    // Its modification time cannot, VTK counting those globally.
    m_SourceTime = image->GetMTime();
    m_Range[0] = min - slack;
    m_Range[1] = max + slack;
  }

  return m_View.GetPointer();
}

vtkImageData *mitk::ScalarRangeViewCache::GetView(vtkImageData *image, vtkVolumeProperty *property)
{
  if (nullptr == image)
    mitkThrow() << "Cannot build a scalar range view of a null image.";

  if (nullptr == property)
    mitkThrow() << "Cannot build a scalar range view without a volume property.";

  const auto *scalars = image->GetPointData()->GetScalars();

  if (nullptr == scalars || 1 != scalars->GetNumberOfComponents())
    return image;

  const double *color = property->GetRGBTransferFunction()->GetRange();
  const double *opacity = property->GetScalarOpacity()->GetRange();

  const double min = std::min(color[0], opacity[0]);
  const double max = std::max(color[1], opacity[1]);

  return min < max ? this->GetView(image, min, max) : image;
}

void mitk::ScalarRangeViewCache::Reset()
{
  m_View = nullptr;
  m_Source = nullptr;
  m_SourceTime = 0;
  m_Range[0] = 0.0;
  m_Range[1] = 0.0;
}
