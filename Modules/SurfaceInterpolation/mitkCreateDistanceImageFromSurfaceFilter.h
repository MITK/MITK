/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkCreateDistanceImageFromSurfaceFilter_h_Included
#define mitkCreateDistanceImageFromSurfaceFilter_h_Included

#include <MitkSurfaceInterpolationExports.h>

#include "mitkImageSource.h"
#include "mitkSurface.h"
#include "mitkProgressBar.h"

#include "vtkSmartPointer.h"
#include "vtkDoubleArray.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkPolyData.h"

#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/algo/vnl_qr.h"

#include "itkImageBase.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkNeighborhoodIterator.h"

#include <queue>

#include <Eigen/Dense>

namespace mitk {

  /**
  \brief This filter interpolates the 3D surface for a segmented area. The basis for the interpolation
         are the edge-points of contours that are drawn into an image.

         The interpolation itself is performed via Radial Basis Function Interpolation.

         ATTENTION:
         This filter needs beside the edge points of the delineated contours additionally the normals for each
         edge point.

         \sa mitkSurfaceInterpolationController

         Based on the contour edge points and their normal this filter calculates a distance function with the following
         properties:
         - Putting a point into the distance function that lies inside the considered surface gives a negativ scalar value
         - Putting a point into the distance function that lies outside the considered surface gives a positive scalar value
         - Putting a point into the distance function that lies exactly on the considered surface gives the value zero

         With this interpolated distance function a distance image will be created. The desired surface can then be extract e.g.
         with the marching cubes algorithm. (Within the  distance image the surface goes exactly where the pixelvalues are zero)

         Note that the obtained distance image has always an isotropig spacing. The size (in this case volume) of the image can be
         adjusted by calling SetDistanceImageVolume(unsigned int volume) which specifies the number ob pixels enclosed by the image.

  \ingroup Process

  $Author: fetzer$
  */
  class MitkSurfaceInterpolation_EXPORT CreateDistanceImageFromSurfaceFilter : public ImageSource
  {

  public:

    typedef vnl_vector_fixed<double,3> PointType;

    typedef itk::Image<double, 3> DistanceImageType;
    typedef DistanceImageType::IndexType IndexType;
    typedef itk::ImageRegionIteratorWithIndex<DistanceImageType> ImageIterator;
    typedef itk::NeighborhoodIterator<DistanceImageType> NeighborhoodImageIterator;

    typedef std::vector< PointType > NormalList;
    typedef std::vector< PointType > CenterList;

    typedef std::vector<Surface::Pointer> SurfaceList;


    mitkClassMacro(CreateDistanceImageFromSurfaceFilter,ImageSource);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    using Superclass::SetInput;

    //Methods copied from mitkSurfaceToSurfaceFilter
    virtual void SetInput( const mitk::Surface* surface );

    virtual void SetInput( unsigned int idx, const mitk::Surface* surface );

    virtual const mitk::Surface* GetInput();

    virtual const mitk::Surface* GetInput( unsigned int idx );

    virtual void RemoveInputs(mitk::Surface* input);


    /**
    \brief Set the size of the output distance image. The size is specified by the image's volume
           (i.e. in this case how many pixels are enclosed by the image)
           If non is set, the volume will be 500000 pixels.
    */
    itkSetMacro(DistanceImageVolume, unsigned int);

    void PrintEquationSystem();

    //Resets the filter, i.e. removes all inputs and outputs
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

    void SetReferenceImage( itk::ImageBase<3>::Pointer referenceImage );


  protected:
    CreateDistanceImageFromSurfaceFilter();
    virtual ~CreateDistanceImageFromSurfaceFilter();
    virtual void GenerateData();
    virtual void GenerateOutputInformation();


  private:

    void CreateSolutionMatrixAndFunctionValues();
    double CalculateDistanceValue(PointType p);

    void CreateDistanceImage ();

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
    void DetermineBounds( DistanceImageType::PointType &minPointInWorldCoordinates,
                          DistanceImageType::PointType &maxPointInWorldCoordinates,
                          DistanceImageType::IndexType &minPointInIndexCoordinates,
                          DistanceImageType::IndexType &maxPointInIndexCoordinates );


    void FillImageRegion(DistanceImageType::RegionType reqRegion, DistanceImageType::PixelType pixelValue, DistanceImageType::Pointer image);

    //Datastructures for the interpolation
    CenterList m_Centers;
    NormalList m_Normals;

    Eigen::MatrixXd m_SolutionMatrix;
    Eigen::VectorXd m_FunctionValues;
    Eigen::VectorXd m_Weights;

    double m_DistanceImageSpacing;

    itk::ImageBase<3>::Pointer m_ReferenceImage;

    unsigned int m_DistanceImageVolume;

    bool m_UseProgressBar;
    unsigned int m_ProgressStepSize;
};

}//namespace


#endif
