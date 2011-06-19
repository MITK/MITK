
/*=========================================================================
 
 Program:   Medical Imaging & Interaction Toolkit
 Module:    $RCSfile$
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 11989 $
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


#ifndef _MITK_FiberBundle_H
#define _MITK_FiberBundle_H

#include "mitkBaseData.h"
#include "MitkDiffusionImagingExports.h"
#include "mitkPlanarFigure.h"

/* This Class represents a bunch of FiberTracts as a Bundle. 
 A Bundle is represented by a GroupSpatialObject */

#include "itkDTITubeSpatialObject.h"
#include "itkDTITubeSpatialObjectPoint.h"
#include "itkGroupSpatialObject.h"
#include "itkPolyLineParametricPath.h"
#include "itkSlowPolyLineParametricPath.h"
#include "itkVectorContainer.h"
#include "itkPointSet.h"
#include "itkVector.h"

namespace mitk {
  
  /**
   * \brief Base Class for Fiber Bundles;   */
  class  MitkDiffusionImaging_EXPORT FiberBundle : public BaseData
  {
    
    /* friend classes wanna access typedefs 
      ContainerPointType, ContainerTractType, ContainerType */
    friend class FiberBundleWriter;
    friend class FiberBundleReader;
//    friend class itkTractsToDWIImageFilter;
  
    /** Types for the standardized TractContainer **/
    typedef itk::Point<float,3>                                                   ContainerPointType; //no need to init, is no smartpointer
    typedef itk::VectorContainer<unsigned int, ContainerPointType>                ContainerTractType;
    typedef itk::VectorContainer< unsigned int, ContainerTractType::Pointer >     ContainerType; //init via smartpointer    
 
    
    
    /** Types for the ITK Stochastic TractContainer **/
    typedef itk::SlowPolyLineParametricPath< 3 >  itkStochTractType;
    typedef itk::VectorContainer< unsigned int, itkStochTractType::Pointer > itkStochTractContainerType;


    // virtual methods that need to be implemented
    virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion( itk::DataObject *data );
    
    
  public:
    
    static const int TRACTPOINT_BACKFACE;
    static const int TRACTPOINT_ON_PLANE;
    static const int TRACTPOINT_FRNTFACE;

    
    /* DTITubeSpatialObject Definitions */
    typedef itk::GroupSpatialObject<3>        FiberGroupType;
    typedef FiberGroupType::ChildrenListType  ChildrenListType;
    
    typedef itk::DTITubeSpatialObject<3>       DTITubeType;
    typedef itk::DTITubeSpatialObjectPoint<3>  DTITubePointType;
    
    mitkClassMacro( FiberBundle, BaseData );
    itkNewMacro( Self );

      
   /* Handle Output Type of ITK Stochastic Tractography Fiber Tracking */
    void addTractContainer( ContainerType::Pointer );
    void additkStochTractContainer(itkStochTractContainerType::Pointer);
    void initFiberGroup();
    void addSingleDTITract(mitk::FiberBundle::DTITubeType::Pointer);
    DTITubeType::Pointer copySingleDTITract(DTITubeType::Pointer);

    /* Methods for PlanarFigure ROIs */
    //mitk::FiberBundle::Pointer extractFibersPF(mitk::PlanarFigure::Pointer);
    std::vector<int> extractFibersByPF(mitk::PlanarFigure::Pointer, std::vector<int>* set=0);
    mitk::FiberBundle::Pointer extractFibersById(std::vector<int> );
    std::vector<int> getAllIDsInFiberBundle();

    
    mitk::Point3D calculateCrossingPoint(mitk::Point3D , mitk::Point3D , mitk::PlanarFigure::Pointer ); //depricated
    bool checkForGap(int, int); //depricated
    /*********************************/
    
    
    void debug_members();
    
    void SetBounds(float* b);
    float* GetBounds();

    
    
//****  REALTIME MONITOR CONTAINER METHOD ****//
    //flag overwrite existing bundle
    void addContainer4speedDisplay( ContainerType::Pointer );

    itkGetMacro(GroupFiberBundle, FiberGroupType::Pointer);
    itkGetMacro(TractContainer, ContainerType::Pointer);

    //** explicit handling of FiberBundleDataStructure contents *//
    void PushPoint(int fiberIndex, ContainerPointType point);
    void PushTract(ContainerTractType::Pointer tract);
    ContainerPointType GetPoint(int tractIndex, int pointIndex);
    ContainerTractType::Pointer GetTract(int tractIndex);
    int GetNumTracts();
    int GetNumPoints(int tractIndex);
    FiberGroupType::Pointer getGroupFiberBundle();

    mitk::FiberBundle::Pointer JoinBundle(mitk::FiberBundle::Pointer bundle);
    int FindTractByEndpoints(mitk::FiberBundle::DTITubeType::Pointer searchTract);
    mitk::FiberBundle::Pointer SubstractBundle(mitk::FiberBundle::Pointer bundle);
    void InsertBundle(mitk::FiberBundle::Pointer bundle);

  protected:
    FiberBundle();
    virtual ~FiberBundle();
      
  private:
    FiberGroupType::Pointer m_GroupFiberBundle;
    ContainerType::Pointer m_TractContainer;
    itkStochTractContainerType::Pointer m_debugITKContainer;
    
    void fiberPostprocessing_FA( DTITubePointType* ); 
    void fiberPostprocessing_setPoint(DTITubePointType* , ContainerPointType );
    void fiberPostprocessing_setTensorMatrix(DTITubePointType*);
    
    int debug_PrototypeCounter;
    float m_boundsFB[3];
    
    /* Methods for PlanarFigure ROIs */
    bool isPointInSelection(mitk::Point3D, mitk::PlanarFigure::Pointer);
    
    
  };
  
} // namespace mitk

#endif /*  _MITK_FiberBundle_H */
