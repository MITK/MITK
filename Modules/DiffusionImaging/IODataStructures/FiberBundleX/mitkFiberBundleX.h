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
#include <mitkBaseData.h>
#include "MitkDiffusionImagingExports.h"
#include <mitkImage.h>


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

    // fiber colorcodings
    static const char* COLORCODING_ORIENTATION_BASED;
    static const char* COLORCODING_FA_BASED;
    static const char* COLORCODING_CUSTOM;
    static const char* FIBER_ID_ARRAY;

    virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion( itk::DataObject *data );

    mitkClassMacro( FiberBundleX, BaseData )
    itkNewMacro( Self )
    mitkNewMacro1Param(Self, vtkSmartPointer<vtkPolyData>) // custom constructor

    // colorcoding related methods
    void SetColorCoding(const char*);
    void SetFAMap(mitk::Image::Pointer);
    void DoColorCodingOrientationBased();
    void DoColorCodingFaBased();
    void DoUseFaFiberOpacity();
    void ResetFiberOpacity();

    // fiber smoothing/resampling
    void ResampleFibers(float pointDistance = 1);
    void DoFiberSmoothing(int pointsPerCm);

    // add/subtract fibers
    mitk::FiberBundleX::Pointer AddBundle(mitk::FiberBundleX* fib);
    mitk::FiberBundleX::Pointer SubtractBundle(mitk::FiberBundleX* fib);

    // fiber subset extraction
    mitk::FiberBundleX::Pointer ExtractFiberSubset(mitk::PlanarFigure::Pointer pf);
    std::vector<long> ExtractFiberIdSubset(mitk::PlanarFigure::Pointer pf);
    vtkSmartPointer<vtkPolyData> GeneratePolyDataByIds( std::vector<long> );

    // get/set data
    void SetFiberPolyData(vtkSmartPointer<vtkPolyData>, bool updateGeometry = true);
    vtkSmartPointer<vtkPolyData> GetFiberPolyData();
    QStringList GetAvailableColorCodings();
    char* GetCurrentColorCoding();
    itkGetMacro(NumFibers, int);

    // copy fiber bundle
    mitk::FiberBundleX::Pointer GetDeepCopy();

    void GenerateFiberIds();

  protected:

    FiberBundleX( vtkPolyData* fiberPolyData = NULL );
    virtual ~FiberBundleX();

    itk::Point<float, 3> GetItkPoint(double point[3]);

    // calculate geometry from fiber extent
    void UpdateFiberGeometry();

    // calculate colorcoding values according to m_CurrentColorCoding
    void UpdateColorCoding();

  private:

    // actual fiber container
    vtkSmartPointer<vtkPolyData>  m_FiberPolyData;

    // contains fiber ids
    vtkSmartPointer<vtkDataSet>   m_FiberIdDataSet;

    char* m_CurrentColorCoding;
    int   m_NumFibers;
  };

} // namespace mitk

#endif /*  _MITK_FiberBundleX_H */
