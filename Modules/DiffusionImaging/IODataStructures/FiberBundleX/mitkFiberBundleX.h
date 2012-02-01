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

    // set/get vtkPolyData
    void SetFiberPolyData(vtkSmartPointer<vtkPolyData>, bool updateGeometry = true);
    vtkSmartPointer<vtkPolyData> GetFiberPolyData();

    mitk::FiberBundleX::Pointer operator+(mitk::FiberBundleX* fib);
    mitk::FiberBundleX::Pointer operator-(mitk::FiberBundleX* fib);

    char* GetCurrentColorCoding();
    void SetColorCoding(const char*);
    void SetFAMap(mitk::Image::Pointer);

    QStringList GetAvailableColorCodings();
    void DoColorCodingOrientationbased();
    void DoColorCodingFAbased();
    void DoUseFAasColorOpacity();
    void ResetFiberColorOpacity();
    void DoGenerateFiberIds();

    void ResampleFibers(float len);
    void ResampleFibers();
    void DoFiberSmoothing(int pointsPerCm);

    mitk::FiberBundleX::Pointer ExtractFiberSubset(mitk::PlanarFigure::Pointer pf);
    std::vector<long> ExtractFiberIdSubset(mitk::PlanarFigure::Pointer pf);
    vtkSmartPointer<vtkPolyData> GeneratePolyDataByIds( std::vector<long> );
    mitk::FiberBundleX::Pointer GetDeepCopy();

    itkGetMacro(NumFibers, int);

  protected:

    FiberBundleX( vtkPolyData* fiberPolyData = NULL );
    virtual ~FiberBundleX();

    itk::Point<float, 3> GetItkPoint(double point[3]);

    void UpdateFiberGeometry();
    void UpdateColorCoding();

  private:

    // actual fiber container
    vtkSmartPointer<vtkPolyData> m_FiberPolyData;

    // contains fiber ids
    vtkSmartPointer<vtkDataSet> m_FiberIdDataSet;

    char* m_currentColorCoding;
    int m_NumFibers;
  };

} // namespace mitk

#endif /*  _MITK_FiberBundleX_H */
