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
#include "itkGibbsTrackingFilter.h"

// MITK
#include <itkOrientationDistributionFunction.h>
#include <itkDiffusionQballGeneralizedFaImageFilter.h>
#include <mitkStandardFileLocations.h>
#include <MersenneTwister.h>
#include <mitkFiberBuilder.h>
#include <mitkMetropolisHastingsSampler.h>
#include <mitkEnergyComputer.h>
#include <itkTensorImageToQBallImageFilter.h>

// ITK
#include <itkImageDuplicator.h>
#include <itkResampleImageFilter.h>

// MISC
#include <fstream>
#include <QFile>

namespace itk{

template< class ItkQBallImageType >
GibbsTrackingFilter< ItkQBallImageType >::GibbsTrackingFilter():
    m_StartTemperature(0.1),
    m_EndTemperature(0.001),
    m_Iterations(500000),
    m_ParticleWeight(0),
    m_ParticleWidth(0),
    m_ParticleLength(0),
    m_ConnectionPotential(10),
    m_InexBalance(0),
    m_ParticlePotential(0.2),
    m_MinFiberLength(10),
    m_AbortTracking(false),
    m_NumConnections(0),
    m_NumParticles(0),
    m_NumAcceptedFibers(0),
    m_CurrentStep(0),
    m_BuildFibers(false),
    m_Steps(10),
    m_ProposalAcceptance(0),
    m_CurvatureThreshold(0.7),
    m_DuplicateImage(true)
{

}

template< class ItkQBallImageType >
GibbsTrackingFilter< ItkQBallImageType >::~GibbsTrackingFilter()
{

}

// fill output fiber bundle datastructure
template< class ItkQBallImageType >
typename GibbsTrackingFilter< ItkQBallImageType >::FiberPolyDataType GibbsTrackingFilter< ItkQBallImageType >::GetFiberBundle()
{
    if (!m_AbortTracking)
    {
        m_BuildFibers = true;
        while (m_BuildFibers){}
    }

    return m_FiberPolyData;
}

template< class ItkQBallImageType >
bool
GibbsTrackingFilter< ItkQBallImageType >
::EstimateParticleWeight()
{
    MITK_INFO << "itkGibbsTrackingFilter: estimating particle weight";
    typedef itk::DiffusionQballGeneralizedFaImageFilter<float,float,QBALL_ODFSIZE> GfaFilterType;
    GfaFilterType::Pointer gfaFilter = GfaFilterType::New();
    gfaFilter->SetInput(m_QBallImage);
    gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
    gfaFilter->Update();
    ItkFloatImageType::Pointer gfaImage = gfaFilter->GetOutput();

    float samplingStart = 1.0;
    float samplingStop = 0.66;

    // GFA iterator
    typedef ImageRegionIterator< ItkFloatImageType > GfaIteratorType;
    GfaIteratorType gfaIt(gfaImage, gfaImage->GetLargestPossibleRegion() );

    // Mask iterator
    typedef ImageRegionConstIterator< ItkFloatImageType > MaskIteratorType;
    MaskIteratorType mit(m_MaskImage, m_MaskImage->GetLargestPossibleRegion() );

    // Input iterator
    typedef ImageRegionConstIterator< ItkQBallImageType > InputIteratorType;
    InputIteratorType it(m_QBallImage, m_QBallImage->GetLargestPossibleRegion() );

    float upper = 0;
    int count = 0;
    for(float thr=samplingStart; thr>samplingStop; thr-=0.01)
    {
        it.GoToBegin();
        mit.GoToBegin();
        gfaIt.GoToBegin();

        while( !gfaIt.IsAtEnd() )
        {
            if(gfaIt.Get()>thr && mit.Get()>0)
            {
                itk::OrientationDistributionFunction<float, QBALL_ODFSIZE> odf(it.Get().GetDataPointer());
                upper += odf.GetMaxValue()-odf.GetMeanValue();
                ++count;
            }
            ++it;
            ++mit;
            ++gfaIt;
        }
    }

    if (count>0)
        upper /= count;
    else
        return false;

    m_ParticleWeight = upper/6;
    return true;
}

// perform global tracking
template< class ItkQBallImageType >
void GibbsTrackingFilter< ItkQBallImageType >::GenerateData()
{
    // check if input is qball or tensor image and generate qball if necessary
    if (m_QBallImage.IsNull() && m_TensorImage.IsNotNull())
    {
        TensorImageToQBallImageFilter<float,float>::Pointer filter = TensorImageToQBallImageFilter<float,float>::New();
        filter->SetInput( m_TensorImage );
        filter->Update();
        m_QBallImage = filter->GetOutput();
    }

    // generate local working copy of QBall image (if not disabled)
    if (m_DuplicateImage)
    {
        typedef itk::ImageDuplicator< ItkQBallImageType > DuplicateFilterType;
        typename DuplicateFilterType::Pointer duplicator = DuplicateFilterType::New();
        duplicator->SetInputImage( m_QBallImage );
        duplicator->Update();
        m_QBallImage = duplicator->GetOutput();
    }

    // perform mean subtraction on odfs
    typedef ImageRegionIterator< ItkQBallImageType > InputIteratorType;
    InputIteratorType it(m_QBallImage, m_QBallImage->GetLargestPossibleRegion() );
    it.GoToBegin();
    while (!it.IsAtEnd())
    {
        itk::OrientationDistributionFunction<float, QBALL_ODFSIZE> odf(it.Get().GetDataPointer());
        float mean = odf.GetMeanValue();
        odf -= mean;
        it.Set(odf.GetDataPointer());
        ++it;
    }

    // check if mask image is given if it needs resampling
    PrepareMaskImage();

    // prepare parameters
    float minSpacing;
    if(m_QBallImage->GetSpacing()[0]<m_QBallImage->GetSpacing()[1] && m_QBallImage->GetSpacing()[0]<m_QBallImage->GetSpacing()[2])
        minSpacing = m_QBallImage->GetSpacing()[0];
    else if (m_QBallImage->GetSpacing()[1] < m_QBallImage->GetSpacing()[2])
        minSpacing = m_QBallImage->GetSpacing()[1];
    else
        minSpacing = m_QBallImage->GetSpacing()[2];

    if(m_ParticleLength == 0)
        m_ParticleLength = 1.5*minSpacing;
    if(m_ParticleWidth == 0)
        m_ParticleWidth = 0.5*minSpacing;
    if(m_ParticleWeight == 0)
        if (!EstimateParticleWeight())
        {
            MITK_INFO << "itkGibbsTrackingFilter: could not estimate particle weight. using default value.";
            m_ParticleWeight = 0.0001;
        }
    float alpha = log(m_EndTemperature/m_StartTemperature);
    m_Steps = m_Iterations/10000;
    if (m_Steps<10)
        m_Steps = 10;
    if (m_Steps>m_Iterations)
    {
        MITK_INFO << "itkGibbsTrackingFilter: not enough iterations!";
        m_AbortTracking = true;
    }
    if (m_CurvatureThreshold < mitk::eps)
        m_CurvatureThreshold = 0;
    unsigned long singleIts = (unsigned long)((1.0*m_Iterations) / (1.0*m_Steps));

    // seed random generators
    MTRand randGen(1);
    srand(1);

    // load sphere interpolator to evaluate the ODFs
    SphereInterpolator* interpolator = LoadSphereInterpolator();

    // initialize the actual tracking components (ParticleGrid, Metropolis Hastings Sampler and Energy Computer)
    ParticleGrid* particleGrid = new ParticleGrid(m_MaskImage, m_ParticleLength);

    EnergyComputer encomp(m_QBallImage, m_MaskImage, particleGrid, interpolator, &randGen);
    encomp.SetParameters(m_ParticleWeight,m_ParticleWidth,m_ConnectionPotential*m_ParticleLength*m_ParticleLength,m_CurvatureThreshold,m_InexBalance,m_ParticlePotential);

    MetropolisHastingsSampler sampler(particleGrid, &encomp, &randGen, m_CurvatureThreshold);

    // main loop
    m_NumAcceptedFibers = 0;
    for( m_CurrentStep = 1; m_CurrentStep <= m_Steps; m_CurrentStep++ )
    {
        // update temperatur for simulated annealing process
        float temperature = m_StartTemperature * exp(alpha*(((1.0)*m_CurrentStep)/((1.0)*m_Steps)));
        sampler.SetTemperature(temperature);

        for (unsigned long i=0; i<singleIts; i++)
        {
            if (m_AbortTracking)
                break;

            sampler.MakeProposal();

            if (m_BuildFibers || (i==singleIts-1 && m_CurrentStep==m_Steps))
            {
                m_ProposalAcceptance = (float)sampler.GetNumAcceptedProposals()/m_Iterations;
                m_NumParticles = particleGrid->m_NumParticles;
                m_NumConnections = particleGrid->m_NumConnections;

                FiberBuilder fiberBuilder(particleGrid, m_MaskImage);
                m_FiberPolyData = fiberBuilder.iterate(m_MinFiberLength);
                m_NumAcceptedFibers = m_FiberPolyData->GetNumberOfLines();
                m_BuildFibers = false;
            }
        }

        m_ProposalAcceptance = (float)sampler.GetNumAcceptedProposals()/m_Iterations;
        m_NumParticles = particleGrid->m_NumParticles;
        m_NumConnections = particleGrid->m_NumConnections;
        MITK_INFO << "itkGibbsTrackingFilter: proposal acceptance: " << 100*m_ProposalAcceptance << "%";
        MITK_INFO << "itkGibbsTrackingFilter: particles: " << m_NumParticles;
        MITK_INFO << "itkGibbsTrackingFilter: connections: " << m_NumConnections;
        MITK_INFO << "itkGibbsTrackingFilter: progress: " << 100*(float)m_CurrentStep/m_Steps << "%";
    }

    delete interpolator;
    delete particleGrid;
    m_AbortTracking = true;
    m_BuildFibers = false;

    MITK_INFO << "itkGibbsTrackingFilter: done generate data";
}

template< class ItkQBallImageType >
void GibbsTrackingFilter< ItkQBallImageType >::PrepareMaskImage()
{
    if(m_MaskImage.IsNull())
    {
        MITK_INFO << "itkGibbsTrackingFilter: generating default mask image";
        m_MaskImage = ItkFloatImageType::New();
        m_MaskImage->SetSpacing( m_QBallImage->GetSpacing() );
        m_MaskImage->SetOrigin( m_QBallImage->GetOrigin() );
        m_MaskImage->SetDirection( m_QBallImage->GetDirection() );
        m_MaskImage->SetRegions( m_QBallImage->GetLargestPossibleRegion() );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1.0);
    }
    else if ( m_MaskImage->GetLargestPossibleRegion().GetSize()[0]!=m_QBallImage->GetLargestPossibleRegion().GetSize()[0] ||
         m_MaskImage->GetLargestPossibleRegion().GetSize()[1]!=m_QBallImage->GetLargestPossibleRegion().GetSize()[1] ||
         m_MaskImage->GetLargestPossibleRegion().GetSize()[2]!=m_QBallImage->GetLargestPossibleRegion().GetSize()[2] ||
         m_MaskImage->GetSpacing()[0]!=m_QBallImage->GetSpacing()[0] ||
         m_MaskImage->GetSpacing()[1]!=m_QBallImage->GetSpacing()[1] ||
         m_MaskImage->GetSpacing()[2]!=m_QBallImage->GetSpacing()[2] )
    {
        MITK_INFO << "itkGibbsTrackingFilter: resampling mask image";
        typedef itk::ResampleImageFilter< ItkFloatImageType, ItkFloatImageType, float > ResamplerType;
        ResamplerType::Pointer resampler = ResamplerType::New();
        resampler->SetOutputSpacing( m_QBallImage->GetSpacing() );
        resampler->SetOutputOrigin( m_QBallImage->GetOrigin() );
        resampler->SetOutputDirection( m_QBallImage->GetDirection() );
        resampler->SetSize( m_QBallImage->GetLargestPossibleRegion().GetSize() );

        resampler->SetInput( m_MaskImage );
        resampler->SetDefaultPixelValue(1.0);
        resampler->Update();
        m_MaskImage = resampler->GetOutput();
        MITK_INFO << "itkGibbsTrackingFilter: resampling finished";
    }
}

template< class ItkQBallImageType >
SphereInterpolator* GibbsTrackingFilter< ItkQBallImageType >::LoadSphereInterpolator()
{
    QString applicationDir = QCoreApplication::applicationDirPath();
    applicationDir.append("/");
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( applicationDir.toStdString().c_str(), false );
    applicationDir.append("../");
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( applicationDir.toStdString().c_str(), false );
    applicationDir.append("../../");
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( applicationDir.toStdString().c_str(), false );

    std::string lutPath = mitk::StandardFileLocations::GetInstance()->FindFile("FiberTrackingLUTBaryCoords.bin");
    ifstream BaryCoords;
    BaryCoords.open(lutPath.c_str(), ios::in | ios::binary);
    float* coords;
    if (BaryCoords.is_open())
    {
        float tmp;
        coords = new float [1630818];
        BaryCoords.seekg (0, ios::beg);
        for (int i=0; i<1630818; i++)
        {
            BaryCoords.read((char *)&tmp, sizeof(tmp));
            coords[i] = tmp;
        }
        BaryCoords.close();
    }
    else
    {
        MITK_INFO << "itkGibbsTrackingFilter: unable to open barycoords file";
        m_AbortTracking = true;
    }

    ifstream Indices;
    lutPath = mitk::StandardFileLocations::GetInstance()->FindFile("FiberTrackingLUTIndices.bin");
    Indices.open(lutPath.c_str(), ios::in | ios::binary);
    int* ind;
    if (Indices.is_open())
    {
        int tmp;
        ind = new int [1630818];
        Indices.seekg (0, ios::beg);
        for (int i=0; i<1630818; i++)
        {
            Indices.read((char *)&tmp, 4);
            ind[i] = tmp;
        }
        Indices.close();
    }
    else
    {
        MITK_INFO << "itkGibbsTrackingFilter: unable to open indices file";
        m_AbortTracking = true;
    }

    // initialize sphere interpolator with lookuptables
    return new SphereInterpolator(coords, ind, QBALL_ODFSIZE, 301, 0.5);
}

}




