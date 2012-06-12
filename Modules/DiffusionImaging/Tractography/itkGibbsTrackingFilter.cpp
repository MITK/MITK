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

#include <itkProgressReporter.h>

#include <mitkQBallImage.h>

#include "GibbsTracking/MersenneTwister.h"
#include "GibbsTracking/mitkFiberBuilder.h"
#include "GibbsTracking/mitkMetropolisHastingsSampler.h"
#include "GibbsTracking/mitkEnergyComputer.h"

#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

#include <QMutexLocker>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_vector_fixed.h>

#include <fstream>
#include <QCoreApplication>
#include <itkRescaleIntensityImageFilter.h>
#include <itkOrientationDistributionFunction.h>
#include <itkImageDuplicator.h>
#include <mitkStandardFileLocations.h>
#include <itkDiffusionQballGeneralizedFaImageFilter.h>
#include <vnl/vnl_copy.h>

namespace itk{

template< class ItkQBallImageType >
GibbsTrackingFilter< ItkQBallImageType >::GibbsTrackingFilter():
    m_TempStart(0.1),
    m_TempEnd(0.001),
    m_NumIt(500000),
    m_ParticleWeight(0),
    m_ParticleWidth(0),
    m_ParticleLength(0),
    m_ChempotConnection(10),
    m_ChempotParticle(0),
    m_InexBalance(0),
    m_Chempot2(0.2),
    m_FiberLength(10),
    m_AbortTracking(false),
    m_NumConnections(0),
    m_NumParticles(0),
    m_NumAcceptedFibers(0),
    m_CurrentStep(0),
    m_BuildFibers(false),
    m_Steps(10),
    m_Memory(0),
    m_ProposalAcceptance(0),
    m_CurvatureHardThreshold(0.7),
    m_Meanval_sq(0.0)
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

    m_NumAcceptedFibers = 0;

    // image sizes and spacing
    int imgSize[4] = {  QBALL_ODFSIZE,
                        m_QBallImage->GetLargestPossibleRegion().GetSize().GetElement(0),
                        m_QBallImage->GetLargestPossibleRegion().GetSize().GetElement(1),
                        m_QBallImage->GetLargestPossibleRegion().GetSize().GetElement(2)};
    double spacing[3] = {m_QBallImage->GetSpacing().GetElement(0),m_QBallImage->GetSpacing().GetElement(1),m_QBallImage->GetSpacing().GetElement(2)};

    // make sure image has enough slices
    if (imgSize[1]<3 || imgSize[2]<3 || imgSize[3]<3)
    {
        MITK_INFO << "itkGibbsTrackingFilter: image size < 3 not supported";
        m_AbortTracking = true;
    }

    // calculate rotation matrix
    vnl_matrix<double> temp = m_QBallImage->GetDirection().GetVnlMatrix();
    vnl_matrix<float>  directionMatrix; directionMatrix.set_size(3,3);
    vnl_copy(temp, directionMatrix);
    vnl_vector_fixed<float, 3> d0 = directionMatrix.get_column(0); d0.normalize();
    vnl_vector_fixed<float, 3> d1 = directionMatrix.get_column(1); d1.normalize();
    vnl_vector_fixed<float, 3> d2 = directionMatrix.get_column(2); d2.normalize();
    directionMatrix.set_column(0, d0);
    directionMatrix.set_column(1, d1);
    directionMatrix.set_column(2, d2);
    vnl_matrix_fixed<float, 3, 3> I = directionMatrix*directionMatrix.transpose();
    if(!I.is_identity(mitk::eps)){
        MITK_INFO << "itkGibbsTrackingFilter: image direction is not a rotation matrix. Tracking not possible!";
        m_AbortTracking = true;
    }

    // generate local working copy of QBall image
    typedef itk::ImageDuplicator< ItkQBallImageType > DuplicateFilterType;
    typename DuplicateFilterType::Pointer duplicator = DuplicateFilterType::New();
    duplicator->SetInputImage( m_QBallImage );
    duplicator->Update();
    typename ItkQBallImageType::Pointer qballImage = duplicator->GetOutput();

    // perform mean subtraction on odfs
    typedef ImageRegionIterator< ItkQBallImageType > InputIteratorType;
    InputIteratorType it(qballImage, qballImage->GetLargestPossibleRegion() );
    it.GoToBegin();
    while (!it.IsAtEnd())
    {
        itk::OrientationDistributionFunction<float, QBALL_ODFSIZE> odf(it.Get().GetDataPointer());
        float mean = odf.GetMeanValue();
        odf -= mean;
        it.Set(odf.GetDataPointer());
        ++it;
    }

    // mask image
    int maskImageSize[3];
    float *mask;
    if(m_MaskImage.IsNotNull())
    {
        mask = (float*) m_MaskImage->GetBufferPointer();
        maskImageSize[0] = m_MaskImage->GetLargestPossibleRegion().GetSize().GetElement(0);
        maskImageSize[1] = m_MaskImage->GetLargestPossibleRegion().GetSize().GetElement(1);
        maskImageSize[2] = m_MaskImage->GetLargestPossibleRegion().GetSize().GetElement(2);
    }
    else
    {
        mask = 0;
        maskImageSize[0] = imgSize[1];
        maskImageSize[1] = imgSize[2];
        maskImageSize[2] = imgSize[3];
    }
    int mask_oversamp_mult = maskImageSize[0]/imgSize[1];

    // load lookuptable
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
    SphereInterpolator *sinterp = new SphereInterpolator(coords, ind, QBALL_ODFSIZE, 301, 0.5);

    // get paramters
    float minSpacing;
    if(spacing[0]<spacing[1] && spacing[0]<spacing[2])
        minSpacing = spacing[0];
    else if (spacing[1] < spacing[2])
        minSpacing = spacing[1];
    else
        minSpacing = spacing[2];

    if(m_ParticleLength == 0)
        m_ParticleLength = 1.5*minSpacing;
    if(m_ParticleWidth == 0)
        m_ParticleWidth = 0.5*minSpacing;
    if(m_ParticleWeight == 0)
        if (!EstimateParticleWeight())
        {
            MITK_INFO << "itkGibbsTrackingFilter: could not estimate particle weight!";
            m_ParticleWeight = 0.0001;
        }
    m_CurrentStep = 0;
    m_Memory = 0;

    float alpha = log(m_TempEnd/m_TempStart);
    m_Steps = m_NumIt/10000;
    if (m_Steps<10)
        m_Steps = 10;
    if (m_Steps>m_NumIt)
    {
        MITK_INFO << "itkGibbsTrackingFilter: not enough iterations!";
        m_AbortTracking = true;
    }

    if (m_CurvatureHardThreshold < mitk::eps)
        m_CurvatureHardThreshold = 0;
    unsigned long singleIts = (unsigned long)((1.0*m_NumIt) / (1.0*m_Steps));

    MTRand randGen(1);
    srand(1);

    MITK_INFO << "itkGibbsTrackingFilter: setting up MH-sampler";


    ParticleGrid* particleGrid = new ParticleGrid(m_MaskImage, 2*m_ParticleLength);
    MetropolisHastingsSampler* sampler = new MetropolisHastingsSampler(particleGrid, &randGen);

    EnergyComputer encomp(&randGen, qballImage, imgSize,spacing,sinterp,particleGrid,mask,mask_oversamp_mult, directionMatrix);
    encomp.setParameters(m_ParticleWeight,m_ParticleWidth,m_ChempotConnection*m_ParticleLength*m_ParticleLength,m_ParticleLength,m_CurvatureHardThreshold,m_InexBalance,m_Chempot2, m_Meanval_sq);

    sampler->SetEnergyComputer(&encomp);
    sampler->SetParameters(m_TempStart,singleIts,m_ParticleLength,m_CurvatureHardThreshold,m_ChempotParticle);

    // main loop
    for( int step = 0; step < m_Steps; step++ )
    {
        if (m_AbortTracking)
            break;

        m_CurrentStep = step+1;
        float temperature = m_TempStart * exp(alpha*(((1.0)*step)/((1.0)*m_Steps)));

        sampler->SetTemperature(temperature);
        sampler->Iterate(&m_ProposalAcceptance, &m_NumConnections, &m_NumParticles, &m_AbortTracking);

        MITK_INFO << "itkGibbsTrackingFilter: proposal acceptance: " << 100*m_ProposalAcceptance << "%";
        MITK_INFO << "itkGibbsTrackingFilter: particles: " << m_NumParticles;
        MITK_INFO << "itkGibbsTrackingFilter: connections: " << m_NumConnections;
        MITK_INFO << "itkGibbsTrackingFilter: progress: " << 100*(float)step/m_Steps << "%";

        if (m_BuildFibers)
        {
            FiberBuilder fiberBuilder(particleGrid, m_MaskImage);
            m_FiberPolyData = fiberBuilder.iterate(m_FiberLength);
            m_NumAcceptedFibers = m_FiberPolyData->GetNumberOfLines();
            m_BuildFibers = false;
        }
    }


    FiberBuilder fiberBuilder(particleGrid, m_MaskImage);
    m_FiberPolyData = fiberBuilder.iterate(m_FiberLength);
    m_NumAcceptedFibers = m_FiberPolyData->GetNumberOfLines();

    delete sinterp;
    delete coords;
    delete ind;
    delete sampler;
    delete particleGrid;
    m_AbortTracking = true;
    m_BuildFibers = false;

    MITK_INFO << "itkGibbsTrackingFilter: done generate data";
}
}




