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

#ifndef mitkPlanarFigureSegmentationController_h_Included
#define mitkPlanarFigureSegmentationController_h_Included

#include "mitkCommon.h"
#include <MitkPlanarFigureSegmentationExports.h>
#include "mitkImage.h"

#include "mitkPlanarFigure.h"
#include <mitkSurface.h>
#include "mitkReduceContourSetFilter.h"
#include <mitkComputeContourSetNormalsFilter.h>
#include <mitkCreateDistanceImageFromSurfaceFilter.h>


namespace mitk
{

/**
* \brief This class creates a binary image from a set of PlanarFigures.
*
* The class offers a convenient way to create a binary image from a
* variable number of contours that are represented by PlanarFigures.
*
* The first step is to create a mitkSurface for each PlanarFigure.
*
* The actual segmentation is done by interpolating these surfaces
* by means of the mitkCreateDistanceImageFromSurfaceFilter.
*
* Using the vtkMarchingCubes a surface is created from the resulting
* distance-image. This surface is then turned into a binary image using the
* mitkSurfaceToImageFilter.
*/

class MITKPLANARFIGURESEGMENTATION_EXPORT PlanarFigureSegmentationController : public itk::Object
{
  public:

    mitkClassMacroItkParent(PlanarFigureSegmentationController, itk::Object);
    itkFactorylessNewMacro(PlanarFigureSegmentationController) /// specify the segmentation image that should be interpolated
    itkCloneMacro(Self)

    virtual ~PlanarFigureSegmentationController();

    typedef std::vector<PlanarFigure::Pointer> PlanarFigureListType;
    typedef std::vector<Surface::Pointer> SurfaceListType;

    /**
    * \brief Setter for the reference image.
    *
    * The reference image is the image on which the segmentation is
    * performed. It's needed to set the correct geometry information
    * on the segmentation result (pixel-spacing, slice-thickness, etc.).
    */
    void SetReferenceImage( Image::Pointer referenceImage );

    /**
    * \brief Adds a new PlanarFigure to be considered in the interpolation.
    *
    * This method can be used to add a new contour, represented by a
    * PlanarFigure, to be considered in the interpolation.
    *
    * It is not possible to add the same PlanarFigure twice.
    *
    * \warn The method does NOT check if there are two PlanarFigures on
    * the same slice. Doing this will lead to wrong segmentation results.
    */
    void AddPlanarFigure( PlanarFigure::Pointer planarFigure );

    void RemovePlanarFigure( mitk::PlanarFigure::Pointer planarFigure );

    /**
    * \brief Performs the interpolation and returns the result
    * as binary image.
    *
    * This method updates the CreateDistanceImageFromSurfaceFilter
    * and creates a binary image from the resulting distance-image.
    *
    * This is done by creating an intermediate mitk::Surface that
    * represents the interpolated 3D contour using the vtkMarchingCubes.
    *
    * The binary image is then extracted from the surface by means of
    * the mitkSurfaceToImageFilter.
    */
    Image::Pointer GetInterpolationResult();


    /**
    * \brief Method to create a surface from a PlanarFigure
    *
    * This method takes any kind of PlanarFigure and creates a
    * surface-representation.
    * The resulting surface contains of only ONE vtkPolygon that contains
    * all points of the PlanarFigure's polyline.
    */
    static Surface::Pointer CreateSurfaceFromPlanarFigure( PlanarFigure::Pointer figure );

    PlanarFigureListType GetAllPlanarFigures();

    void SetReferenceDirectionVector( mitk::Vector3D vector );



  protected:

    PlanarFigureSegmentationController();

    void InitializeFilters();

    template<typename TPixel, unsigned int VImageDimension>
    void GetImageBase(itk::Image<TPixel, VImageDimension>* input, itk::ImageBase<3>::Pointer& result);

    PlanarFigureListType m_PlanarFigureList;
    SurfaceListType m_SurfaceList;

    /**
    * \brief Filter to reduce the number of control points.
    */
    ReduceContourSetFilter::Pointer m_ReduceFilter;

    /**
    * \brief Filter to compute the normals for the created surface
    */
    ComputeContourSetNormalsFilter::Pointer m_NormalsFilter;

    /**
    * \brief Filter to create "distance-image" from contours.
    */
    CreateDistanceImageFromSurfaceFilter::Pointer m_DistanceImageCreator;

    Image::Pointer m_ReferenceImage;

    Image::Pointer m_SegmentationAsImage;

    mitk::Vector3D m_ReferenceDirectionVector;


};

} // namespace

#endif


