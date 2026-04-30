/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExtractSliceFilter_h
#define mitkExtractSliceFilter_h

#include <MitkCoreExports.h>
#include <mitkImageToImageFilter.h>

#include <vtkAbstractTransform.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

namespace mitk
{
  /**
  \brief ExtractSliceFilter extracts a 2D arbitrary oriented slice from a 3D volume.

  The filter can reslice in all orthogonal planes such as sagittal, coronal and axial,
  and is also able to reslice an arbitrary oriented oblique plane.
  Curved planes are specified via an AbstractTransformGeometry as the input worldgeometry.

  Additionally the filter extracts the specified component of a multi-component input image.
  This is done only if the caller requests an mitk::Image output ('m_VtkOutputRequested' set to false).
  The default component to be extracted is '0'.

  The convenient workflow is:
  1. Set an image as input.
  2. Set the worldPlaneGeometry. This defines a grid where the slice is being extracted
  3. And then start the pipeline.

  There are a few more properties that can be set to modify the behavior of the slicing.
  The properties are:
  - interpolation mode either Nearestneighbor, Linear or Cubic.
  - a transform this is a convenient way to adapt the reslice axis for the case
    that the image is transformed e.g. rotated.
  - time step the time step in a times volume.
  - the component to extract from a multi-component input image
  - vtkoutputrequested, to define whether an mitk::image should be initialized
  - resample by geometry whether the resampling grid corresponds to the specs of the
    worldgeometry or is directly derived from the input image

  By default the properties are set to:
  - interpolation mode Nearestneighbor.
  - a transform nullptr (No transform is set).
  - time step 0.
  - component 0.
  - resample by geometry false (Corresponds to input image).
  */
  class MITKCORE_EXPORT ExtractSliceFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(ExtractSliceFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    mitkNewMacro1Param(Self, vtkImageReslice *);

    /**
     * \brief Set the plane geometry defining the reslice axis.
     *
     * \param geometry The PlaneGeometry that defines where to reslice.
     */
    void SetWorldGeometry(const PlaneGeometry *geometry)
    {
      if (geometry != m_WorldGeometry)
      {
        this->m_WorldGeometry = geometry;
        this->Modified();
      }
    }

    /**
     * \brief Set the time step in the 4D volume.
     *
     * \param timestep The time step index to extract.
     */
    void SetTimeStep(unsigned int timestep) { m_TimeStep = timestep; }

    /**
     * \brief Get the current time step.
     *
     * \return The time step index currently set.
     */
    unsigned int GetTimeStep() { return m_TimeStep; }

    /**
     * \brief Set the component of a multi-component image to be extracted.
     *
     * \param component The zero-based component index.
     */
    void SetComponent(unsigned int component) { m_Component = component; }

    /**
     * \brief Set a transform for the reslice axes.
     *
     * This transform is needed if the image volume itself is transformed (affects the reslice axis).
     *
     * \param transform The BaseGeometry whose transform is used.
     */
    void SetResliceTransformByGeometry(const BaseGeometry *transform) { this->m_ResliceTransform = transform; }

    /**
     * \brief Set whether the resampling grid is derived from image or world geometry.
     *
     * \param inPlaneResampleExtentByGeometry If \c false, resampling grid corresponds to the image;
     *        if \c true, it corresponds to the world geometry.
     */
    void SetInPlaneResampleExtentByGeometry(bool inPlaneResampleExtentByGeometry)
    {
      this->m_InPlaneResampleExtentByGeometry = inPlaneResampleExtentByGeometry;
    }

    /**
     * \brief Set the output dimensionality of the slice.
     *
     * \param dimension The output dimension (typically 2 for a slice).
     */
    void SetOutputDimensionality(unsigned int dimension) { this->m_OutputDimension = dimension; }

    /**
     * \brief Set the spacing in z direction manually.
     *
     * Required if the output dimension is > 2.
     *
     * \param zSpacing The spacing value in z direction.
     */
    void SetOutputSpacingZDirection(double zSpacing) { this->m_ZSpacing = zSpacing; }

    /**
     * \brief Set the extent in pixels for the z direction manually.
     *
     * Required if the output dimension is > 2.
     *
     * \param zMin Minimum z extent.
     * \param zMax Maximum z extent.
     */
    void SetOutputExtentZDirection(int zMin, int zMax)
    {
      this->m_ZMin = zMin;
      this->m_ZMax = zMax;
    }

    /**
     * \brief Get the bounding box of the slice [xMin, xMax, yMin, yMax, zMin, zMax].
     *
     * The method uses the input of the filter to calculate the bounds.
     * It is recommended to use
     * GetClippedPlaneBounds(const BaseGeometry*, const PlaneGeometry*, double*)
     * if you are not sure about the input.
     *
     * \param bounds Output array of six doubles: [xMin, xMax, yMin, yMax, zMin, zMax].
     * \return \c true if bounds were successfully computed, \c false otherwise.
     */
    bool GetClippedPlaneBounds(double bounds[6]);

    /**
     * \brief Get the bounding box of the slice [xMin, xMax, yMin, yMax, zMin, zMax].
     *
     * \param boundingGeometry The bounding geometry of the volume.
     * \param planeGeometry The plane geometry defining the slice.
     * \param bounds Output array of six doubles: [xMin, xMax, yMin, yMax, zMin, zMax].
     * \return \c true if bounds were successfully computed, \c false otherwise.
     */
    bool GetClippedPlaneBounds(const BaseGeometry *boundingGeometry,
                               const PlaneGeometry *planeGeometry,
                               double *bounds);

    /**
     * \brief Get the spacing of the slice.
     *
     * \return Pointer to an array of two mitk::ScalarType values representing the x and y spacing.
     */
    mitk::ScalarType *GetOutputSpacing();

    /**
     * \brief Get the output as vtkImageData.
     *
     * \note SetVtkOutputRequest(true) has to be called at least once before
     * GetVtkOutput(). Otherwise the output is empty for the first update step.
     *
     * \return The resliced output as vtkImageData.
     */
    vtkImageData *GetVtkOutput()
    {
      m_VtkOutputRequested = true;
      return m_Reslicer->GetOutput();
    }

    /**
     * \brief Set VtkOutputRequest to suppress the conversion of the image.
     *
     * It is suggested to use this with GetVtkOutput().
     *
     * \note SetVtkOutputRequest(true) has to be called at least once before
     * GetVtkOutput(). Otherwise the output is empty for the first update step.
     *
     * \param isRequested If \c true, output will be provided as vtkImageData.
     */
    void SetVtkOutputRequest(bool isRequested) { m_VtkOutputRequested = isRequested; }

    /**
     * \brief Get the reslice axes matrix.
     *
     * \note The axes are recalculated when calling SetResliceTransformByGeometry.
     *
     * \return The 4x4 reslice axes matrix.
     */
    vtkMatrix4x4 *GetResliceAxes() { return this->m_Reslicer->GetResliceAxes(); }

    /**
     * \brief Set the background level for areas outside the volume.
     *
     * \param backgroundLevel The background intensity value.
     */
    void SetBackgroundLevel(double backgroundLevel) { m_BackgroundLevel = backgroundLevel; }
    enum ResliceInterpolation
    {
      RESLICE_NEAREST = 0,
      RESLICE_LINEAR = 1,
      RESLICE_CUBIC = 3
    };

    /**
     * \brief Set the interpolation mode for reslicing.
     *
     * \param interpolation The interpolation mode (RESLICE_NEAREST, RESLICE_LINEAR, or RESLICE_CUBIC).
     */
    void SetInterpolationMode(ExtractSliceFilter::ResliceInterpolation interpolation)
    {
      this->m_InterpolationMode = interpolation;
    }

  protected:
    /**
     * \brief Constructor.
     *
     * \param reslicer Optional custom vtkImageReslice instance. If nullptr, a default one is created.
     */
    ExtractSliceFilter(vtkImageReslice *reslicer = nullptr);

    /** \brief Destructor. */
    ~ExtractSliceFilter() override;

    /** \brief Perform the actual reslicing. */
    void GenerateData() override;

    /** \brief Compute output information (spacing, origin, extent) based on input and world geometry. */
    void GenerateOutputInformation() override;

    /** \brief Set the requested region on the input image. */
    void GenerateInputRequestedRegion() override;

    PlaneGeometry::ConstPointer m_WorldGeometry;
    vtkSmartPointer<vtkImageReslice> m_Reslicer;

    unsigned int m_TimeStep;

    unsigned int m_OutputDimension;

    double m_ZSpacing;

    int m_ZMin;

    int m_ZMax;

    ResliceInterpolation m_InterpolationMode;

    bool m_InPlaneResampleExtentByGeometry; // Resampling grid corresponds to:  false->image    true->worldgeometry

    mitk::ScalarType *m_OutPutSpacing;

    bool m_VtkOutputRequested;

    double m_BackgroundLevel;

    unsigned int m_Component;

  private:
    BaseGeometry::ConstPointer m_ResliceTransform;
    /* Axis vectors of the relevant geometry. Set in GenerateOutputInformation() and also used in GenerateData().*/
    Vector3D m_Right, m_Bottom;
    /* Bounds of the relevant plane. Set in GenerateOutputInformation() and also used in GenerateData().*/
    int m_XMin, m_XMax, m_YMin, m_YMax;

  };
}

#endif
