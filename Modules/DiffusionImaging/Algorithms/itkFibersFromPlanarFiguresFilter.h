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
#ifndef itkFibersFromPlanarFiguresFilter_h
#define itkFibersFromPlanarFiguresFilter_h

// MITK
#include <mitkPlanarEllipse.h>
#include <mitkFiberBundleX.h>

// ITK
#include <itkProcessObject.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

using namespace std;

namespace itk{

/**
* \brief Generates artificial fibers distributed in and interpolated between the input planar figures.   */

class FibersFromPlanarFiguresFilter : public ProcessObject
{
public:

    enum FiberDistribution{
        DISTRIBUTE_UNIFORM,
        DISTRIBUTE_GAUSSIAN
    };

    typedef FibersFromPlanarFiguresFilter Self;
    typedef ProcessObject                                       Superclass;
    typedef SmartPointer< Self >                                Pointer;
    typedef SmartPointer< const Self >                          ConstPointer;
    typedef vector< vector< mitk::PlanarEllipse::Pointer > >    FiducialListType;
    typedef vector< vector< unsigned int > >                    FlipListType;
    typedef mitk::FiberBundleX::Pointer                         FiberType;
    typedef vector< mitk::FiberBundleX::Pointer >               FiberContainerType;

    itkNewMacro(Self)
    itkTypeMacro( FibersFromPlanarFiguresFilter, ProcessObject )

    void GenerateData();

    virtual void Update(){
        this->GenerateData();
    }

    void SetFlipList(FlipListType fliplist){ m_FlipList = fliplist; }
    void SetFiducials(FiducialListType fiducials){ m_Fiducials = fiducials; }
    FiberContainerType GetFiberBundles(){ return m_FiberBundles; }
    itkSetMacro(Density, int)
    itkSetMacro(FiberSampling, int)
    itkSetMacro(Tension, double)
    itkSetMacro(Continuity, double)
    itkSetMacro(Bias, double)
    itkSetMacro(FiberDistribution, FiberDistribution)
    itkSetMacro(Variance, double)

protected:

    FibersFromPlanarFiguresFilter();
    virtual ~FibersFromPlanarFiguresFilter();
    void GeneratePoints();

    FiberDistribution   m_FiberDistribution;    ///< flag to switch between uniform and gaussian distribution of the fiber waypoints inside of the fiducials
    FlipListType        m_FlipList;             ///< contains flags indicating a flip of the 2D fiber x-coordinates (needed to resolve some unwanted fiber twisting)
    FiducialListType    m_Fiducials;            ///< container of the planar ellipses used as fiducials for the fiber generation process
    FiberContainerType  m_FiberBundles;         ///< container for the output fiber bundles
    int                 m_Density;              ///< number of fibers per bundle
    int                 m_FiberSampling;        ///< sampling points of the fibers per cm
    double              m_Tension;              ///< tension parameter of the Kochanek-Bartels splines
    double              m_Continuity;           ///< continuity parameter of the Kochanek-Bartels splines
    double              m_Bias;                 ///< bias parameter of the Kochanek-Bartels splines
    double              m_Variance;             ///< variance of the gaussian waypoint distribution
    vector< mitk::Vector2D > m_2DPoints;        ///< container for the 2D fiber waypoints
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFibersFromPlanarFiguresFilter.cpp"
#endif

#endif
