
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

/* =============== IMPORTANT TODO ===================
 * ==== USE vtkSmartPointer<> when necessary ONLY!!!!
 */

#ifndef _MITK_FiberBundleX_H
#define _MITK_FiberBundleX_H

//includes for MITK datastructure
#include "mitkBaseData.h"
#include "MitkDiffusionImagingExports.h"


//includes storing fiberdata
#include <vtkSmartPointer.h> //may be replaced by class precompile argument
#include <vtkPolyData.h> // may be replaced by class
#include <vtkPoints.h> // my be replaced by class
#include <vtkDataSet.h>

namespace mitk {

  /**
   * \brief Base Class for Fiber Bundles;   */
  class  MitkDiffusionImaging_EXPORT FiberBundleX : public BaseData
  {
  public:
  
    // names of certain arrays (e.g colorcodings, etc.) 
    static const char* COLORCODING_ORIENTATION_BASED;
    static const char* COLORCODING_FA_BASED;
    static const char* FIBER_ID_ARRAY;
    
    /* friend classes wanna access typedefs
    ContainerPointType, ContainerTractType, ContainerType */
    friend class FiberBundleXWriter;
    friend class FiberBundleXReader;

    
    // ======virtual methods must have======
    virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion( itk::DataObject *data );
    //=======================================
    
    mitkClassMacro( FiberBundleX, BaseData );
    itkNewMacro( Self );


    
    
    
    /*====FIBERBUNDLE I/O METHODS====*/
    void SetFibers(vtkPolyData*); //set result of tractography algorithm in vtkPolyData format using vtkPolyLines
    vtkPolyData* GetFibers();
    vtkSmartPointer<vtkPolyData> GetVertices();
    char* getCurrentColorCoding();
    bool isFiberBundleXModified();
    void setFBXModificationDone();
    
    /*===FIBERBUNDLE PROCESSING METHODS====*/
    void DoColorCodingOrientationbased();
    void DoGenerateFiberIds();
    
    /*===FIBERBUNDLE ASSESSMENT METHODS====*/
    // Compute Bounding Box of FiberStructure; needed for MITK Geometry
    double* DoComputeFiberStructureBoundingBox();



  protected:
    FiberBundleX();
    virtual ~FiberBundleX();

  private:
    
        
////  ====TODO====================================
//    bool doSelfHealingColorOrient( vtkPolyData* );
////  ============================================
    
    //      The following polydata variables are used for fiber- and pointbased representation of the tractography results. As VTK suggests, one vtkPolyData is used to manage vertices and the other for polylines.
    //      FiberPolyData stores all brain fibers using polylines (in world coordinates)
    //    this variable hosts the smoothed fiber data, this data we generate, therefore a smartpointer structure is recommended
//    vtkSmartPointer<vtkPolyData> m_FiberPolyData;  is depricated
//    
    //    this variable hosts the original fiber data, no smartpointer needed because who or whatever passes this data to FiberBundleX should use vtkSmartPointer structure
  
    vtkPolyData* m_FiberStructureData; //this is a common pointer because fiberDataStructure gets passed to this class. m_FiberStructureData is destroyed in the destructor then.
    
    //    VertexPolyData stores all original points as vertices computed by tracking algorithms
    vtkSmartPointer<vtkPolyData> m_VertexPolyData;
    
    // this variable contains all additional IDs of Fibers which are needed for efficient fiber manipulation such as extracting etc.
    vtkSmartPointer<vtkDataSet> m_FiberIdDataSet;
    char* m_currentColorCoding;

    //this flag conzerns only visual representation.
    bool m_isModified;
    
    
  };

} // namespace mitk

#endif /*  _MITK_FiberBundleX_H */
