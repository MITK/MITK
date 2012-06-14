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
#ifndef itkGibbsTrackingFilter_h
#define itkGibbsTrackingFilter_h

#include "itkProcessObject.h"
#include "itkVectorContainer.h"
#include "itkImage.h"

#include <fstream>
#include <QFile>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

namespace itk{

template< class ItkQBallImageType >
class GibbsTrackingFilter : public ProcessObject
{
public:
    typedef GibbsTrackingFilter Self;
    typedef ProcessObject Superclass;
    typedef SmartPointer< Self > Pointer;
    typedef SmartPointer< const Self > ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro( GibbsTrackingFilter, ProcessObject )

    typedef typename ItkQBallImageType::Pointer ItkQBallImageTypePointer;
    typedef Image< float, 3 >                   ItkFloatImageType;
    typedef vtkSmartPointer< vtkPolyData >      FiberPolyDataType;

    itkSetMacro( TempStart, float )
    itkGetMacro( TempStart, float )

    itkSetMacro( TempEnd, float )
    itkGetMacro( TempEnd, float )

    itkSetMacro( NumIt, unsigned long )
    itkGetMacro( NumIt, unsigned long )

    itkSetMacro( ParticleWeight, float )
    itkGetMacro( ParticleWeight, float )

    /** width of particle sigma (std-dev of gaussian around center) **/
    itkSetMacro( ParticleWidth, float )
    itkGetMacro( ParticleWidth, float )

    /** length of particle from midpoint to ends **/
    itkSetMacro( ParticleLength, float )
    itkGetMacro( ParticleLength, float )

    itkSetMacro( ChempotConnection, float )
    itkGetMacro( ChempotConnection, float )

    itkSetMacro( ChempotParticle, float )
    itkGetMacro( ChempotParticle, float )

    itkSetMacro( InexBalance, float )
    itkGetMacro( InexBalance, float )

    itkSetMacro( Chempot2, float )
    itkGetMacro( Chempot2, float )

    itkSetMacro( FiberLength, int )
    itkGetMacro( FiberLength, int )

    itkSetMacro( AbortTracking, bool )
    itkGetMacro( AbortTracking, bool )

    itkSetMacro( CurrentStep, unsigned long )
    itkGetMacro( CurrentStep, unsigned long )

    itkSetMacro( CurvatureHardThreshold, float)
    itkGetMacro( CurvatureHardThreshold, float)

    itkGetMacro(NumParticles, unsigned long)
    itkGetMacro(NumConnections, unsigned long)
    itkGetMacro(NumAcceptedFibers, int)
    itkGetMacro(ProposalAcceptance, float)
    itkGetMacro(Steps, unsigned int)

    // input data
    itkSetMacro(QBallImage, typename ItkQBallImageType::Pointer)
    itkSetMacro(MaskImage, ItkFloatImageType::Pointer)

    void GenerateData();

    virtual void Update(){
        this->GenerateData();
    }

    FiberPolyDataType GetFiberBundle();

protected:

    GibbsTrackingFilter();
    virtual ~GibbsTrackingFilter();
    bool EstimateParticleWeight();

    // Input Images
    typename ItkQBallImageType::Pointer m_QBallImage;
    typename ItkFloatImageType::Pointer m_MaskImage;

    // Tracking parameters
    float   m_TempStart;            // Start temperature
    float   m_TempEnd;              // End temperature
    unsigned long m_NumIt;          // Total number of iterations
    unsigned long m_CurrentStep;    // current tracking step
    float   m_ParticleWeight;       // w (unitless)
    float   m_ParticleWidth;        //sigma  (mm)
    float   m_ParticleLength;       // ell (mm)
    float   m_ChempotConnection;    // gross L (chemisches potential)
    float   m_ChempotParticle;      // unbenutzt (immer null, wenn groesser dann insgesamt weniger teilchen)
    float   m_InexBalance;          // gewichtung zwischen den lambdas; -5 ... 5 -> nur intern ... nur extern,default 0
    float   m_Chempot2;             // typischerweise 0
    int     m_FiberLength;
    bool    m_AbortTracking;
    int     m_NumAcceptedFibers;
    volatile bool   m_BuildFibers;
    unsigned int    m_Steps;
    float   m_Memory;
    float   m_ProposalAcceptance;
    float   m_CurvatureHardThreshold;
    float   m_Meanval_sq;
    bool    m_DuplicateImage;

    FiberPolyDataType m_FiberPolyData;
    unsigned long m_NumParticles;
    unsigned long m_NumConnections;
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGibbsTrackingFilter.cpp"
#endif

#endif
