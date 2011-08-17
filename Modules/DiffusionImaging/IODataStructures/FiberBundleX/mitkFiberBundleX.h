
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


#ifndef _MITK_FiberBundleX_H
#define _MITK_FiberBundleX_H

//includes for MITK datastructure
#include "mitkBaseData.h"
#include "MitkDiffusionImagingExports.h"


//includes storing fiberdata
#include <vtkSmartPointer.h> //may be replaced by class precompile argument
#include <vtkPolyData.h> // may be replaced by class
#include <vtkPoints.h> // my be replaced by class


namespace mitk {

  /**
   * \brief Base Class for Fiber Bundles;   */
  class  MitkDiffusionImaging_EXPORT FiberBundleX : public BaseData
  {
  public:

    // names of certain arrays (e.g colorcodings, etc.) 
    static const char* COLORCODING_ORIENTATION_BASED;
    static const char* COLORCODING_FA_BASED;
    
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

    //    import fiber result from tractography algorithms
    void SetFibers(vtkSmartPointer<vtkPolyData>);
    
    //    return original computed fibers ... actually there is no smartpointer needed because original fibers are passed from the tractography filter
    vtkPolyData* GetOriginalFibers();
    
    //    return processed fibers
    vtkPolyData* GetFibers();
    
    //    return vertex polydata
    vtkPolyData* GetVertices();
    
    void DoColorCodingOrientationbased();


  protected:
    FiberBundleX();
    virtual ~FiberBundleX();

  private:
    //      The following polydata variables are used for fiber- and pointbased representation of the tractography results. As VTK suggests, one vtkPolyData is used to manage vertices and the other for polylines.
    //      FiberPolyData stores all brain fibers using polylines (in world coordinates)
    //    this variable hosts the smoothed fiber data, this data we generate, therefore a smartpointer structure is recommended
    vtkSmartPointer<vtkPolyData> m_FiberPolyData;  
    
    //    this variable hosts the original fiber data, no smartpointer needed because who or whatever passes this data to FiberBundleX should use vtkSmartPointer structure
  
    vtkPolyData* m_OriginalFiberPolyData;
    
    //    VertexPolyData stores all original points as vertices computed by tracking algorithms
    vtkSmartPointer<vtkPolyData> m_VertexPolyData;
    
    
  };

} // namespace mitk

#endif /*  _MITK_FiberBundleX_H */
