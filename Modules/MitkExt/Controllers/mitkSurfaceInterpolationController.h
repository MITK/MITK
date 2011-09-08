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

#include "mitkCreateDistanceImageFromSurfaceFilter.h"
#include "mitkReduceContourSetFilter.h"
#include "mitkComputeContourSetNormalsFilter.h"

#include "vtkPolygon.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkAppendPolyData.h"

#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkWeakPointer.h"

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
     * If a contour at the position represented by the RestorePlaneOperation already exists
       this contour is returned. Otherwise NULL is returned.
     */
    //Surface::Pointer GetSurface (RestorePlanePositionOperation &op);

    /*
     * Interpolates the 3D surface from the given extracted contours
     */
    Surface::Pointer Interpolate ();

    //itkSetMacro(MinSpacing, double);
    //itkSetMacro(MaxSpacing, double);
    //itkSetMacro(DistImageVolume, unsigned int);
    //itkSetMacro(WorkingImage, Image*);

    void SetMinSpacing(double minSpacing);
    void SetMaxSpacing(double maxSpacing);
    void SetDistanceImageVolume(unsigned int distImageVolume);
    void SetWorkingImage(Image* workingImage);

   /* bool DataSetHasChanged();*/

    Surface* GetContoursAsSurface();

    void SetDataStorage(DataStorage &ds);

    unsigned int CreateNewContourList();

    void SetCurrentListID (int ID);



 protected:

   SurfaceInterpolationController();

   ~SurfaceInterpolationController();

 private:

   struct ContourPositionPair {
     Surface::Pointer contour;
     RestorePlanePositionOperation* position;
   };

    typedef std::vector<ContourPositionPair> ContourPositionPairList;
   //typedef std::map< RestorePlanePositionOperation*, Surface::Pointer > ContourPositionPairList;

    //ContourPositionPairList m_ContourList;
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

   /* bool m_Modified;*/

    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

    std::vector<ContourPositionPairList> m_ListOfContourLists;
    unsigned int m_CurrentContourListID;

 };
}
#endif