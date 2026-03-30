/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExtractDirectedPlaneImageFilter_h
#define mitkExtractDirectedPlaneImageFilter_h

#include <mitkImageToImageFilter.h>
#include <MitkImageExtractionExports.h>

#include <mitkVtkResliceInterpolationProperty.h>
#include <vtkImageReslice.h>

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

    /**
     * \brief Set the world geometry describing the plane to be extracted.
     * \param[in] _arg Pointer to the PlaneGeometry defining the slice position and orientation.
     * \pre The geometry must be a PlaneGeometry or an AbstractTransformGeometry.
     */
    itkSetMacro(WorldGeometry, PlaneGeometry *);

    /**
     * \brief Get the internal vtkImageReslice instance.
     *
     * The reslicer can be used to configure interpolation mode. See the
     * vtkImageReslice documentation for available options. Misuse is at
     * the caller's risk.
     *
     * \return Pointer to the vtkImageReslice object used internally.
     */
    itkGetMacro(Reslicer, vtkImageReslice *);

    /**
     * \brief Set/Get the target time step for extraction from a 4D image.
     *
     * For 3D images this parameter is ignored. For 4D images, the specified
     * time step is extracted before reslicing. Default is 0.
     */
    itkSetMacro(TargetTimestep, unsigned int);
    itkGetMacro(TargetTimestep, unsigned int);

    /**
     * \brief Set/Get whether the resampling extent is determined by geometry.
     *
     * When true, the output spacing is derived from the world geometry's
     * extent. When false, the spacing is derived from the input image
     * geometry. Default is true.
     */
    itkSetMacro(InPlaneResampleExtentByGeometry, bool);
    itkGetMacro(InPlaneResampleExtentByGeometry, bool);

    /**
     * \brief Set the reslice interpolation property.
     * \param[in] _arg Pointer to a VtkResliceInterpolationProperty defining the interpolation mode.
     */
    setMacro(ResliceInterpolationProperty, VtkResliceInterpolationProperty *);

    /** \brief Get the reslice interpolation property. */
    itkGetMacro(ResliceInterpolationProperty, VtkResliceInterpolationProperty *);

    /**
     * \brief Set/Get whether the filter operates in mapper mode.
     */
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

#endif
