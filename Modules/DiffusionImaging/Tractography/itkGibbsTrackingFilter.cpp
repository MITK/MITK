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
#include <mitkFiberBuilder.h>
#include <mitkMetropolisHastingsSampler.h>
//#include <mitkEnergyComputer.h>
#include <itkTensorImageToQBallImageFilter.h>
#include <mitkGibbsEnergyComputer.h>

// ITK
#include <itkImageDuplicator.h>
#include <itkResampleImageFilter.h>
#include <itkTimeProbe.h>

// MISC
#include <fstream>
#include <QFile>
#include <tinyxml.h>
#include <math.h>

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
    m_DuplicateImage(true),
    m_RandomSeed(-1),
    m_LoadParameterFile(""),
    m_LutPath(""),
    m_IsInValidState(true)
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
void
GibbsTrackingFilter< ItkQBallImageType >
::EstimateParticleWeight()
{
    MITK_INFO << "GibbsTrackingFilter: estimating particle weight";

    float minSpacing;
    if(m_QBallImage->GetSpacing()[0]<m_QBallImage->GetSpacing()[1] && m_QBallImage->GetSpacing()[0]<m_QBallImage->GetSpacing()[2])
        minSpacing = m_QBallImage->GetSpacing()[0];
    else if (m_QBallImage->GetSpacing()[1] < m_QBallImage->GetSpacing()[2])
        minSpacing = m_QBallImage->GetSpacing()[1];
    else
        minSpacing = m_QBallImage->GetSpacing()[2];
    float m_ParticleLength = 1.5*minSpacing;
    float m_ParticleWidth = 0.5*minSpacing;

    // seed random generators
    Statistics::MersenneTwisterRandomVariateGenerator::Pointer randGen = Statistics::MersenneTwisterRandomVariateGenerator::New();
    if (m_RandomSeed>-1)
        randGen->SetSeed(m_RandomSeed);
    else
        randGen->SetSeed();

    // instantiate all necessary components
    SphereInterpolator* interpolator = new SphereInterpolator(m_LutPath);
    // handle lookup table not found cases
    if( !interpolator->IsInValidState() )
    {
      m_IsInValidState = false;
      m_AbortTracking = true;
      m_BuildFibers = false;
      mitkThrow() << "Unable to load lookup tables.";
    }
    ParticleGrid* particleGrid = new ParticleGrid(m_MaskImage, m_ParticleLength, m_ParticleGridCellCapacity);
    GibbsEnergyComputer* encomp = new GibbsEnergyComputer(m_QBallImage, m_MaskImage, particleGrid, interpolator, randGen);

    //    EnergyComputer* encomp = new EnergyComputer(m_QBallImage, m_MaskImage, particleGrid, interpolator, randGen);
    MetropolisHastingsSampler* sampler = new MetropolisHastingsSampler(particleGrid, encomp, randGen, m_CurvatureThreshold);

    float alpha = log(m_EndTemperature/m_StartTemperature);
    m_ParticleWeight = 0.01;
    int ppv = 0;
    // main loop
    int neededParts = 3000;
    while (ppv<neededParts)
    {
        if (ppv<1000)
            m_ParticleWeight /= 2;
        else
            m_ParticleWeight = ppv*m_ParticleWeight/neededParts;

        encomp->SetParameters(m_ParticleWeight,m_ParticleWidth,m_ConnectionPotential*m_ParticleLength*m_ParticleLength,m_CurvatureThreshold,m_InexBalance,m_ParticlePotential);
        for( int step = 0; step < 10; step++ )
        {
            // update temperatur for simulated annealing process
            float temperature = m_StartTemperature * exp(alpha*(((1.0)*step)/((1.0)*10)));
            sampler->SetTemperature(temperature);

            for (unsigned long i=0; i<10000; i++)
                sampler->MakeProposal();
        }
        ppv = particleGrid->m_NumParticles;
        particleGrid->ResetGrid();
    }
    delete sampler;
    delete encomp;
    delete particleGrid;
    delete interpolator;

    MITK_INFO << "GibbsTrackingFilter: finished estimating particle weight";
}

// perform global tracking
template< class ItkQBallImageType >
void GibbsTrackingFilter< ItkQBallImageType >::GenerateData()
{
    TimeProbe preClock; preClock.Start();
    // check if input is qball or tensor image and generate qball if necessary
    if (m_QBallImage.IsNull() && m_TensorImage.IsNotNull())
    {
        TensorImageToQBallImageFilter<float,float>::Pointer filter = TensorImageToQBallImageFilter<float,float>::New();
        filter->SetInput( m_TensorImage );
        filter->Update();
        m_QBallImage = filter->GetOutput();
    }
    else if (m_DuplicateImage) // generate local working copy of QBall image (if not disabled)
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

    // load parameter file
    LoadParameters();

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
        EstimateParticleWeight();

    float alpha = log(m_EndTemperature/m_StartTemperature);
    m_Steps = m_Iterations/10000;
    if (m_Steps<10)
        m_Steps = 10;
    if (m_Steps>m_Iterations)
    {
        MITK_INFO << "GibbsTrackingFilter: not enough iterations!";
        m_AbortTracking = true;
    }
    if (m_CurvatureThreshold < mitk::eps)
        m_CurvatureThreshold = 0;
    unsigned long singleIts = (unsigned long)((1.0*m_Iterations) / (1.0*m_Steps));

    // seed random generators
    Statistics::MersenneTwisterRandomVariateGenerator::Pointer randGen = Statistics::MersenneTwisterRandomVariateGenerator::New();
    if (m_RandomSeed>-1)
        randGen->SetSeed(m_RandomSeed);
    else
        randGen->SetSeed();

    // load sphere interpolator to evaluate the ODFs
    SphereInterpolator* interpolator = new SphereInterpolator(m_LutPath);

    // handle lookup table not found cases
    if( !interpolator->IsInValidState() )
    {
      m_IsInValidState = false;
      m_AbortTracking = true;
      m_BuildFibers = false;
      mitkThrow() << "Unable to load lookup tables.";
    }
    // initialize the actual tracking components (ParticleGrid, Metropolis Hastings Sampler and Energy Computer)
    ParticleGrid* particleGrid;
    GibbsEnergyComputer* encomp;
    MetropolisHastingsSampler* sampler;
    try{
        particleGrid = new ParticleGrid(m_MaskImage, m_ParticleLength, m_ParticleGridCellCapacity);
        encomp = new GibbsEnergyComputer(m_QBallImage, m_MaskImage, particleGrid, interpolator, randGen);
        encomp->SetParameters(m_ParticleWeight,m_ParticleWidth,m_ConnectionPotential*m_ParticleLength*m_ParticleLength,m_CurvatureThreshold,m_InexBalance,m_ParticlePotential);
        sampler = new MetropolisHastingsSampler(particleGrid, encomp, randGen, m_CurvatureThreshold);
    }
    catch(...)
    {
        MITK_ERROR  << "Particle grid allocation failed. Not enough memory? Try to increase the particle length.";
        m_IsInValidState = false;
        m_AbortTracking = true;
        m_BuildFibers = false;
        return;
    }

    MITK_INFO << "----------------------------------------";
    MITK_INFO << "Iterations: " << m_Iterations;
    MITK_INFO << "Steps: " << m_Steps;
    MITK_INFO << "Particle length: " << m_ParticleLength;
    MITK_INFO << "Particle width: " << m_ParticleWidth;
    MITK_INFO << "Particle weight: " << m_ParticleWeight;
    MITK_INFO << "Start temperature: " << m_StartTemperature;
    MITK_INFO << "End temperature: " << m_EndTemperature;
    MITK_INFO << "In/Ex balance: " << m_InexBalance;
    MITK_INFO << "Min. fiber length: " << m_MinFiberLength;
    MITK_INFO << "Curvature threshold: " << m_CurvatureThreshold;
    MITK_INFO << "Random seed: " << m_RandomSeed;
    MITK_INFO << "----------------------------------------";

    // main loop
    preClock.Stop();
    TimeProbe clock; clock.Start();
    m_NumAcceptedFibers = 0;
    unsigned long counter = 1;
    if (!m_AbortTracking)
    for( m_CurrentStep = 1; m_CurrentStep <= m_Steps; m_CurrentStep++ )
    {
        // update temperatur for simulated annealing process
        float temperature = m_StartTemperature * exp(alpha*(((1.0)*m_CurrentStep)/((1.0)*m_Steps)));
        sampler->SetTemperature(temperature);

        for (unsigned long i=0; i<singleIts; i++)
        {
            if (m_AbortTracking)
                break;

            sampler->MakeProposal();

            if (m_BuildFibers || (i==singleIts-1 && m_CurrentStep==m_Steps))
            {
                m_ProposalAcceptance = (float)sampler->GetNumAcceptedProposals()/counter;
                m_NumParticles = particleGrid->m_NumParticles;
                m_NumConnections = particleGrid->m_NumConnections;

                FiberBuilder fiberBuilder(particleGrid, m_MaskImage);
                m_FiberPolyData = fiberBuilder.iterate(m_MinFiberLength);
                m_NumAcceptedFibers = m_FiberPolyData->GetNumberOfLines();
                m_BuildFibers = false;
            }
            counter++;
        }

        m_ProposalAcceptance = (float)sampler->GetNumAcceptedProposals()/counter;
        m_NumParticles = particleGrid->m_NumParticles;
        m_NumConnections = particleGrid->m_NumConnections;

        MITK_INFO << "GibbsTrackingFilter: proposal acceptance: " << 100*m_ProposalAcceptance << "%";
        MITK_INFO << "GibbsTrackingFilter: particles: " << m_NumParticles;
        MITK_INFO << "GibbsTrackingFilter: connections: " << m_NumConnections;
        MITK_INFO << "GibbsTrackingFilter: progress: " << 100*(float)m_CurrentStep/m_Steps << "%";
        MITK_INFO << "GibbsTrackingFilter: cell overflows: " << particleGrid->m_NumCellOverflows;
        MITK_INFO << "----------------------------------------";

        if (m_AbortTracking)
            break;
    }
    if (m_AbortTracking)
    {
        FiberBuilder fiberBuilder(particleGrid, m_MaskImage);
        m_FiberPolyData = fiberBuilder.iterate(m_MinFiberLength);
        m_NumAcceptedFibers = m_FiberPolyData->GetNumberOfLines();
    }
    clock.Stop();

    delete sampler;
    delete encomp;
    delete interpolator;
    delete particleGrid;
    m_AbortTracking = true;
    m_BuildFibers = false;

    int h = clock.GetTotal()/3600;
    int m = ((int)clock.GetTotal()%3600)/60;
    int s = (int)clock.GetTotal()%60;
    MITK_INFO << "GibbsTrackingFilter: finished gibbs tracking in " << h << "h, " << m << "m and " << s << "s";
    m = (int)preClock.GetTotal()/60;
    s = (int)preClock.GetTotal()%60;
    MITK_INFO << "GibbsTrackingFilter: preparation of the data took " << m << "m and " << s << "s";
    MITK_INFO << "GibbsTrackingFilter: " << m_NumAcceptedFibers << " fibers accepted";

    SaveParameters();
}

template< class ItkQBallImageType >
void GibbsTrackingFilter< ItkQBallImageType >::PrepareMaskImage()
{
    if(m_MaskImage.IsNull())
    {
        MITK_INFO << "GibbsTrackingFilter: generating default mask image";
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
        MITK_INFO << "GibbsTrackingFilter: resampling mask image";
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
        MITK_INFO << "GibbsTrackingFilter: resampling finished";
    }
}

// load tracking paramters from xml file (.gtp)
template< class ItkQBallImageType >
bool GibbsTrackingFilter< ItkQBallImageType >::LoadParameters()
{
    m_AbortTracking = true;
    try
    {
        if( m_LoadParameterFile.length()==0 )
        {
            m_AbortTracking = false;
            return true;
        }

        MITK_INFO << "GibbsTrackingFilter: loading parameter file " << m_LoadParameterFile;

        TiXmlDocument doc( m_LoadParameterFile );
        doc.LoadFile();

        TiXmlHandle hDoc(&doc);
        TiXmlElement* pElem;
        TiXmlHandle hRoot(0);

        pElem = hDoc.FirstChildElement().Element();
        hRoot = TiXmlHandle(pElem);
        pElem = hRoot.FirstChildElement("parameter_set").Element();

        QString iterations(pElem->Attribute("iterations"));
        m_Iterations = iterations.toULong();

        QString particleLength(pElem->Attribute("particle_length"));
        m_ParticleLength = particleLength.toFloat();

        QString particleWidth(pElem->Attribute("particle_width"));
        m_ParticleWidth = particleWidth.toFloat();

        QString partWeight(pElem->Attribute("particle_weight"));
        m_ParticleWeight = partWeight.toFloat();

        QString startTemp(pElem->Attribute("temp_start"));
        m_StartTemperature = startTemp.toFloat();

        QString endTemp(pElem->Attribute("temp_end"));
        m_EndTemperature = endTemp.toFloat();

        QString inExBalance(pElem->Attribute("inexbalance"));
        m_InexBalance = inExBalance.toFloat();

        QString fiberLength(pElem->Attribute("fiber_length"));
        m_MinFiberLength = fiberLength.toFloat();

        QString curvThres(pElem->Attribute("curvature_threshold"));
        m_CurvatureThreshold = cos(curvThres.toFloat()*M_PI/180);
        m_AbortTracking = false;
        MITK_INFO << "GibbsTrackingFilter: parameter file loaded successfully";
        return true;
    }
    catch(...)
    {
        MITK_INFO << "GibbsTrackingFilter: could not load parameter file";
        return false;
    }
}

// save current tracking paramters to xml file (.gtp)
template< class ItkQBallImageType >
bool GibbsTrackingFilter< ItkQBallImageType >::SaveParameters()
{
    try
    {
        if( m_SaveParameterFile.length()==0 )
        {
            MITK_INFO << "GibbsTrackingFilter: no filename specified to save parameters";
            return true;
        }

        MITK_INFO << "GibbsTrackingFilter: saving parameter file " << m_SaveParameterFile;

        TiXmlDocument documentXML;
        TiXmlDeclaration* declXML = new TiXmlDeclaration( "1.0", "", "" );
        documentXML.LinkEndChild( declXML );

        TiXmlElement* mainXML = new TiXmlElement("global_tracking_parameter_file");
        mainXML->SetAttribute("file_version",  "0.1");
        documentXML.LinkEndChild(mainXML);

        TiXmlElement* paramXML = new TiXmlElement("parameter_set");
        paramXML->SetAttribute("iterations", QString::number(m_Iterations).toStdString());
        paramXML->SetAttribute("particle_length", QString::number(m_ParticleLength).toStdString());
        paramXML->SetAttribute("particle_width", QString::number(m_ParticleWidth).toStdString());
        paramXML->SetAttribute("particle_weight", QString::number(m_ParticleWeight).toStdString());
        paramXML->SetAttribute("temp_start", QString::number(m_StartTemperature).toStdString());
        paramXML->SetAttribute("temp_end", QString::number(m_EndTemperature).toStdString());
        paramXML->SetAttribute("inexbalance", QString::number(m_InexBalance).toStdString());
        paramXML->SetAttribute("fiber_length", QString::number(m_MinFiberLength).toStdString());
        paramXML->SetAttribute("curvature_threshold", QString::number(m_CurvatureThreshold).toStdString());
        mainXML->LinkEndChild(paramXML);

        QString filename(m_SaveParameterFile.c_str());
        if(!filename.endsWith(".gtp"))
            filename += ".gtp";
        documentXML.SaveFile( filename.toStdString() );

        MITK_INFO << "GibbsTrackingFilter: parameter file saved successfully";
        return true;
    }
    catch(...)
    {
        MITK_INFO << "GibbsTrackingFilter: could not save parameter file";
        return false;
    }
}

}




