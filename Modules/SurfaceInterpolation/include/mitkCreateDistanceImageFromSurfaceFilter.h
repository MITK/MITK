/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCreateDistanceImageFromSurfaceFilter_h
#define mitkCreateDistanceImageFromSurfaceFilter_h

#include <MitkSurfaceInterpolationExports.h>

#include <mitkImageSource.h>
#include <mitkProgressBar.h>
#include <mitkSurface.h>

#include <vnl/vnl_vector_fixed.h>

#include <itkImageBase.h>

#include <itkeigen/Eigen/Dense>

namespace mitk
{
  /** \brief Filter that creates a distance image from contour surface points using Radial Basis Function interpolation.
   *
   * Given a set of contour edge points with associated normals (as mitk::Surface inputs),
   * this filter computes an implicit distance function via Radial Basis Function (RBF)
   * interpolation and samples it into a 3D distance image.
   *
   * The distance function has the following properties:
   * - Points inside the surface yield negative values.
   * - Points outside the surface yield positive values.
   * - Points on the surface yield zero.
   *
   * The zero level set of the resulting distance image represents the interpolated
   * surface, which can be extracted using e.g. marching cubes.
   *
   * The distance image always has isotropic spacing. Its total number of pixels can
   * be adjusted via SetDistanceImageVolume() (default: 500000).
   *
   * \pre Each input surface must contain contour edge points with normals stored
   *      in the cell data.
   *
   * \sa ComputeContourSetNormalsFilter, ReduceContourSetFilter, SurfaceInterpolationController
   */
  class MITKSURFACEINTERPOLATION_EXPORT CreateDistanceImageFromSurfaceFilter : public ImageSource
  {
  public:
    typedef vnl_vector_fixed<double, 3> PointType;

    typedef itk::Image<double, 3> DistanceImageType;
    typedef DistanceImageType::IndexType IndexType;

    typedef std::vector<PointType> NormalList;
    typedef std::vector<PointType> CenterList;

    typedef std::vector<Surface::Pointer> SurfaceList;

    mitkClassMacro(CreateDistanceImageFromSurfaceFilter, ImageSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkGetMacro(DistanceImageSpacing, double);

        using Superclass::SetInput;

    // Methods copied from mitkSurfaceToSurfaceFilter
    virtual void SetInput(const mitk::Surface *surface);

    virtual void SetInput(unsigned int idx, const mitk::Surface *surface);

    virtual const mitk::Surface *GetInput();

    virtual const mitk::Surface *GetInput(unsigned int idx);

    virtual void RemoveInputs(mitk::Surface *input);

    /**
    \brief Set the size of the output distance image. The size is specified by the image's volume
           (i.e. in this case how many pixels are enclosed by the image)
           If non is set, the volume will be 500000 pixels.
    */
    itkSetMacro(DistanceImageVolume, unsigned int);

    void PrintEquationSystem();

    // Resets the filter, i.e. removes all inputs and outputs
    void Reset();

    /**
      \brief Set whether the mitkProgressBar should be used

      \a Parameter true for using the progress bar, false otherwise
    */
    void SetUseProgressBar(bool);

    /**
      \brief Set the stepsize which the progress bar should proceed

      \a Parameter The stepsize for progressing
    */
    void SetProgressStepSize(unsigned int stepSize);

    void SetReferenceImage(itk::ImageBase<3>::Pointer referenceImage);

  protected:
    CreateDistanceImageFromSurfaceFilter();
    ~CreateDistanceImageFromSurfaceFilter() override;
    void GenerateData() override;
    void GenerateOutputInformation() override;

  private:
    void CreateSolutionMatrixAndFunctionValues();
    double CalculateDistanceValue(PointType p);

    void FillDistanceImage();

    /**
    * \brief This method fills the given variables with the minimum and
    * maximum coordinates that contain all input-points in index- and
    * world-coordinates.
    *
    * This method iterates over all input-points and transforms them from
    * world-coordinates to index-coordinates using the transform of the
    * reference-Image.
    * Next, the minimal and maximal index-coordinates are determined that
    * span an area that contains all given input-points.
    * These index-coordinates are then transformed back to world-coordinates.
    *
    * These minimal and maximal points are then set to the given variables.
    */
    void DetermineBounds(DistanceImageType::PointType &minPointInWorldCoordinates,
                         DistanceImageType::PointType &maxPointInWorldCoordinates,
                         DistanceImageType::IndexType &minPointInIndexCoordinates,
                         DistanceImageType::IndexType &maxPointInIndexCoordinates);

    void PreprocessContourPoints();
    void CreateEmptyDistanceImage();

    // Datastructures for the interpolation
    CenterList m_Centers;
    NormalList m_Normals;

    Eigen::MatrixXd m_SolutionMatrix;
    Eigen::VectorXd m_FunctionValues;
    Eigen::VectorXd m_Weights;

    DistanceImageType::Pointer m_DistanceImageITK;
    itk::ImageBase<3>::Pointer m_ReferenceImage;

    double m_DistanceImageSpacing;
    double m_DistanceImageDefaultBufferValue;
    unsigned int m_DistanceImageVolume;

    bool m_UseProgressBar;
    unsigned int m_ProgressStepSize;
  };

} // namespace

#endif
