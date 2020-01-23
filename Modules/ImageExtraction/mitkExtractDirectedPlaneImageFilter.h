/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExtractDirectedPlaneImageFilter_h_Included
#define mitkExtractDirectedPlaneImageFilter_h_Included

#include "mitkImageToImageFilter.h"
#include <MitkImageExtractionExports.h>

#include "mitkVtkResliceInterpolationProperty.h"
#include "vtkImageReslice.h"

#define setMacro(name, type)                                                                                           \
  virtual void Set##name(type _arg)                                                                                    \
  {                                                                                                                    \
    if (this->m_##name != _arg)                                                                                        \
    {                                                                                                                  \
      this->m_##name = _arg;                                                                                           \
    }                                                                                                                  \
  }

#define getMacro(name, type)                                                                                           \
  virtual type Get##name() { return m_##name; }
class vtkPoints;

namespace mitk
{
  /**
    \deprecated This class is deprecated. Use mitk::ExtractSliceFilter instead.
    \sa ExtractSliceFilter

    \brief Extracts a 2D slice of arbitrary geometry from a 3D or 4D image.

    \sa mitkImageMapper2D

    \ingroup ImageToImageFilter

    This class takes a 3D or 4D mitk::Image as input and tries to extract one slice from it.
    This slice can be arbitrary oriented in space. The 2D slice is resliced by a
    vtk::ResliceImage filter if not perpendicular to the input image.

    The world geometry of the plane to be extracted image must be given as an input
    to the filter in order to correctly calculate world coordinates of the extracted slice.
    Setting a timestep from which the plane should be extracted is optional.

    Output will not be set if there was a problem extracting the desired slice.

    Last contributor: $Author: T. Schwarz$
  */

  class MITKIMAGEEXTRACTION_EXPORT ExtractDirectedPlaneImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(ExtractDirectedPlaneImageFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkSetMacro(WorldGeometry, PlaneGeometry *);

    // The Reslicer is accessible to configure the desired interpolation;
    // (See vtk::ImageReslice class for documentation).
    // Misusage is at your own risk...
    itkGetMacro(Reslicer, vtkImageReslice *);

    // The target timestep in a 4D image from which the 2D plane is supposed
    // to be extracted.
    itkSetMacro(TargetTimestep, unsigned int);
    itkGetMacro(TargetTimestep, unsigned int);

    itkSetMacro(InPlaneResampleExtentByGeometry, bool);
    itkGetMacro(InPlaneResampleExtentByGeometry, bool);

    setMacro(ResliceInterpolationProperty, VtkResliceInterpolationProperty *);
    itkGetMacro(ResliceInterpolationProperty, VtkResliceInterpolationProperty *);

    setMacro(IsMapperMode, bool);
    getMacro(IsMapperMode, bool);

  protected:
    ExtractDirectedPlaneImageFilter(); // purposely hidden
    ~ExtractDirectedPlaneImageFilter() override;

    void GenerateData() override;
    void GenerateOutputInformation() override;

    bool CalculateClippedPlaneBounds(const BaseGeometry *boundingGeometry,
                                     const PlaneGeometry *planeGeometry,
                                     double *bounds);
    bool LineIntersectZero(vtkPoints *points, int p1, int p2, double *bounds);

    const PlaneGeometry *m_WorldGeometry;
    vtkImageReslice *m_Reslicer;

    unsigned int m_TargetTimestep;
    bool m_InPlaneResampleExtentByGeometry;
    int m_ThickSlicesMode;
    int m_ThickSlicesNum;
    bool m_IsMapperMode;

    VtkResliceInterpolationProperty *m_ResliceInterpolationProperty;
  };

} // namespace mitk

#endif // mitkExtractDirectedPlaneImageFilter_h_Included
