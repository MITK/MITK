
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
#include <mitkBaseData.h>
#include "MitkDiffusionImagingExports.h"


//includes storing fiberdata
#include <vtkSmartPointer.h> //may be replaced by class precompile argument
#include <vtkPolyData.h> // may be replaced by class
#include <vtkPoints.h> // my be replaced by class
#include <vtkDataSet.h>

#include <QStringList>

#include <mitkPlanarFigure.h>

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

//    friend class FiberBundleXWriter;
//    friend class FiberBundleXReader;

    virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion( itk::DataObject *data );

    mitkClassMacro( FiberBundleX, BaseData )
    itkNewMacro( Self )
    mitkNewMacro1Param(Self, vtkSmartPointer<vtkPolyData>) // custom constructor

    // set/get vtkPolyData
    void SetFiberPolyData(vtkSmartPointer<vtkPolyData>, bool updateGeometry = true);
    vtkSmartPointer<vtkPolyData> GetFiberPolyData();

    char* GetCurrentColorCoding();
    void SetColorCoding(const char*);
    bool isFiberBundleXModified();
    void setFBXModificationDone();

    QStringList GetAvailableColorCodings();
    void DoColorCodingOrientationbased();
    void DoGenerateFiberIds();
    void ResampleFibers(float len);
    std::vector<int> DoExtractFiberIds(mitk::PlanarFigure::Pointer );

    itkGetMacro(NumFibers, int);

  protected:
    FiberBundleX( vtkSmartPointer<vtkPolyData> fiberPolyData = NULL );
    virtual ~FiberBundleX();
    void UpdateFiberGeometry();

  private:

    // actual fiber container
    vtkSmartPointer<vtkPolyData> m_FiberPolyData;

    // contains all additional IDs of Fibers which are needed for efficient fiber manipulation such as extracting etc.
    vtkSmartPointer<vtkDataSet> m_FiberIdDataSet;

    char* m_currentColorCoding;

    //this flag concerns only visual representation.
    bool m_isModified;

    bool m_NumFibers;
  };
} // namespace mitk

#endif /*  _MITK_FiberBundleX_H */
