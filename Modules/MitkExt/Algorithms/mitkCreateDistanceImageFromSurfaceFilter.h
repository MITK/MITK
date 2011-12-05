/*=========================================================================
Program: Medical Imaging & Interaction Toolkit
Module: $RCSfile$
Language: C++
Date: $Date: $
Version: $Revision: $
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notices for more information.
=========================================================================*/

#ifndef mitkCreateDistanceImageFromSurfaceFilter_h_Included
#define mitkCreateDistanceImageFromSurfaceFilter_h_Included

#include "MitkExtExports.h"

#include "mitkImageSource.h"
#include "mitkSurface.h"

#include "vtkSmartPointer.h"
#include "vtkDoubleArray.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkPolyData.h"

#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/algo/vnl_qr.h"

#include "itkImage.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkNeighborhoodIterator.h"

#include <queue>

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
  class MitkExt_EXPORT CreateDistanceImageFromSurfaceFilter : public ImageSource
  {

  public:

    typedef vnl_vector_fixed<double,3> PointType;

    typedef std::vector< PointType > NormalList;
    typedef std::vector< PointType > CenterList;

    typedef vnl_matrix<double> SolutionMatrix;
    typedef vnl_vector<double> FunctionValues;
    typedef vnl_vector<double> InterpolationWeights;

    typedef std::vector<Surface::Pointer> SurfaceList;

    mitkClassMacro(CreateDistanceImageFromSurfaceFilter,ImageSource);
    itkNewMacro(Self);

    //Methods copied from mitkSurfaceToSurfaceFilter
    virtual void SetInput( const mitk::Surface* surface );

    virtual void SetInput( unsigned int idx, const mitk::Surface* surface );

    virtual const mitk::Surface* GetInput();

    virtual const mitk::Surface* GetInput( unsigned int idx );

    virtual void RemoveInputs(mitk::Surface* input);


    /*
    \brief Set the size of the output distance image. The size is specified by the image's volume
           (i.e. in this case how many pixels are enclosed by the image)
           If non is set, the volume will be 500000 pixels.
    */
    itkSetMacro(DistanceImageVolume, unsigned int);

    void PrintEquationSystem();

    //Resets the filter, i.e. removes all inputs and outputs
    void Reset();

  protected:
    CreateDistanceImageFromSurfaceFilter();
    virtual ~CreateDistanceImageFromSurfaceFilter();
    virtual void GenerateData();
    virtual void GenerateOutputInformation();


  private:

    void CreateSolutionMatrixAndFunctionValues();
    double CalculateDistanceValue(PointType p);

    void CreateDistanceImage ();

    //Datastructures for the interpolation
    CenterList m_Centers;
    NormalList m_Normals;
    FunctionValues m_FunctionValues;
    InterpolationWeights m_Weights;
    SolutionMatrix m_SolutionMatrix;
    double m_DistanceImageSpacing;

    unsigned int m_DistanceImageVolume;

};

}//namespace


#endif
