/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef mitkSurfaceInterpolationController_h_Included
#define mitkSurfaceInterpolationController_h_Included

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkSurface.h"
#include "mitkInteractionConst.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"

#include "mitkCreateDistanceImageFromSurfaceFilter.h"
#include "mitkReduceContourSetFilter.h"
#include "mitkComputeContourSetNormalsFilter.h"

#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkWeakPointer.h"

#include "vtkPolygon.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkAppendPolyData.h"

#include "vtkMarchingCubes.h"
#include "vtkImageData.h"
#include "mitkVtkRepresentationProperty.h"
#include "vtkProperty.h"


namespace mitk
{

 class MitkExt_EXPORT SurfaceInterpolationController : public itk::Object
 {

  public:

    mitkClassMacro(SurfaceInterpolationController, itk::Object);
    itkNewMacro(Self);

    static SurfaceInterpolationController* GetInstance();

    /*
     * Adds a new extracted contour to the list 
     */
    void AddNewContour(Surface::Pointer newContour, RestorePlanePositionOperation *op);

    /*
     * Interpolates the 3D surface from the given extracted contours
     */
    void Interpolate ();

    mitk::Surface::Pointer GetInterpolationResult();

    void SetMinSpacing(double minSpacing);
    void SetMaxSpacing(double maxSpacing);
    void SetDistanceImageVolume(unsigned int distImageVolume);
    void SetWorkingImage(Image* workingImage);

    Surface* GetContoursAsSurface();

    void SetDataStorage(DataStorage &ds);

    unsigned int CreateNewContourList();

    void SetCurrentListID (unsigned int ID);

    mitk::Image* GetImage();

 protected:

   SurfaceInterpolationController();

   ~SurfaceInterpolationController();

 private:

   struct ContourPositionPair {
     Surface::Pointer contour;
     RestorePlanePositionOperation* position;
   };

    typedef std::vector<ContourPositionPair> ContourPositionPairList;

    ContourPositionPairList::iterator m_Iterator;

    ReduceContourSetFilter::Pointer m_ReduceFilter;
    ComputeContourSetNormalsFilter::Pointer m_NormalsFilter;
    CreateDistanceImageFromSurfaceFilter::Pointer m_InterpolateSurfaceFilter;

    double m_MinSpacing;
    double m_MaxSpacing;

    const Image* m_WorkingImage;

    Surface::Pointer m_Contours;

    vtkSmartPointer<vtkPolyData> m_PolyData;

    unsigned int m_DistImageVolume;

    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

    std::vector<ContourPositionPairList> m_ListOfContourLists;

    unsigned int m_CurrentContourListID;

    mitk::Surface::Pointer m_InterpolationResult;
 };
}
#endif
