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
#include <itkDiffusionOdfGeneralizedFaImageFilter.h>
#include <mitkStandardFileLocations.h>
#include <mitkFiberBuilder.h>
#include <mitkMetropolisHastingsSampler.h>
//#include <mitkEnergyComputer.h>
#include <itkTensorImageToOdfImageFilter.h>
#include <mitkGibbsEnergyComputer.h>

// ITK
#include <itkImageDuplicator.h>
#include <itkResampleImageFilter.h>
#include <itkTimeProbe.h>

// MISC
#include <fstream>
// #include <QFile>
#include <tinyxml.h>
#include <boost/progress.hpp>
#include <mitkLexicalCast.h>
#include <boost/algorithm/string.hpp>

namespace itk{

template< class ItkOdfImageType >
GibbsTrackingFilter< ItkOdfImageType >::GibbsTrackingFilter():
  m_StartTemperature(0.1),
  m_EndTemperature(0.001),
  m_Iterations(1e9),
  m_CurrentIteration(0.0),
  m_CurrentStep(0),
  m_ParticleWeight(0),
  m_ParticleWidth(0),
  m_ParticleLength(0),
  m_ConnectionPotential(10),
  m_InexBalance(0),
  m_ParticlePotential(0.2),
  m_MinFiberLength(10),
  m_AbortTracking(false),
  m_NumAcceptedFibers(0),
  m_BuildFibers(false),
  m_ProposalAcceptance(0),
  m_CurvatureThreshold(0.7),
  m_DuplicateImage(true),
  m_NumParticles(0),
  m_NumConnections(0),
  m_RandomSeed(-1),
  m_LoadParameterFile(""),
  m_LutPath(""),
  m_IsInValidState(true)
{

}

template< class ItkOdfImageType >
GibbsTrackingFilter< ItkOdfImageType >::~GibbsTrackingFilter()
{

}

// fill output fiber bundle datastructure
template< class ItkOdfImageType >
typename GibbsTrackingFilter< ItkOdfImageType >::FiberPolyDataType GibbsTrackingFilter< ItkOdfImageType >::GetFiberBundle()
{
  if (!m_AbortTracking)
  {
    m_BuildFibers = true;
    while (m_BuildFibers){}
  }

  return m_FiberPolyData;
}

template< class ItkOdfImageType >
void
GibbsTrackingFilter< ItkOdfImageType >
::EstimateParticleWeight()
{
  MITK_INFO << "GibbsTrackingFilter: estimating particle weight";

  float minSpacing;
  if(m_OdfImage->GetSpacing()[0]<m_OdfImage->GetSpacing()[1] && m_OdfImage->GetSpacing()[0]<m_OdfImage->GetSpacing()[2])
    minSpacing = m_OdfImage->GetSpacing()[0];
  else if (m_OdfImage->GetSpacing()[1] < m_OdfImage->GetSpacing()[2])
    minSpacing = m_OdfImage->GetSpacing()[1];
  else
    minSpacing = m_OdfImage->GetSpacing()[2];
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
  GibbsEnergyComputer* encomp = new GibbsEnergyComputer(m_OdfImage, m_MaskImage, particleGrid, interpolator, randGen);

  //    EnergyComputer* encomp = new EnergyComputer(m_OdfImage, m_MaskImage, particleGrid, interpolator, randGen);
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
template< class ItkOdfImageType >
void GibbsTrackingFilter< ItkOdfImageType >::GenerateData()
{
  TimeProbe preClock; preClock.Start();
  // check if input is Odf or tensor image and generate Odf if necessary
  if (m_OdfImage.IsNull() && m_TensorImage.IsNotNull())
  {
    TensorImageToOdfImageFilter<float,float>::Pointer filter = TensorImageToOdfImageFilter<float,float>::New();
    filter->SetInput( m_TensorImage );
    filter->Update();
    m_OdfImage = filter->GetOutput();
  }
  else if (m_DuplicateImage) // generate local working copy of Odf image (if not disabled)
  {
    typedef itk::ImageDuplicator< ItkOdfImageType > DuplicateFilterType;
    typename DuplicateFilterType::Pointer duplicator = DuplicateFilterType::New();
    duplicator->SetInputImage( m_OdfImage );
    duplicator->Update();
    m_OdfImage = duplicator->GetOutput();
  }

  // perform mean subtraction on odfs
  typedef ImageRegionIterator< ItkOdfImageType > InputIteratorType;
  InputIteratorType it(m_OdfImage, m_OdfImage->GetLargestPossibleRegion() );
  it.GoToBegin();
  while (!it.IsAtEnd())
  {
    itk::OrientationDistributionFunction<float, ODF_SAMPLING_SIZE> odf(it.Get().GetDataPointer());
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
  if(m_OdfImage->GetSpacing()[0]<m_OdfImage->GetSpacing()[1] && m_OdfImage->GetSpacing()[0]<m_OdfImage->GetSpacing()[2])
    minSpacing = m_OdfImage->GetSpacing()[0];
  else if (m_OdfImage->GetSpacing()[1] < m_OdfImage->GetSpacing()[2])
    minSpacing = m_OdfImage->GetSpacing()[1];
  else
    minSpacing = m_OdfImage->GetSpacing()[2];

  if(m_ParticleLength == 0)
    m_ParticleLength = 1.5*minSpacing;
  if(m_ParticleWidth == 0)
    m_ParticleWidth = 0.5*minSpacing;

  if(m_ParticleWeight == 0)
    EstimateParticleWeight();

  float alpha = log(m_EndTemperature/m_StartTemperature);

  if (m_CurvatureThreshold < mitk::eps)
    m_CurvatureThreshold = 0;

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
    encomp = new GibbsEnergyComputer(m_OdfImage, m_MaskImage, particleGrid, interpolator, randGen);
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
  m_CurrentIteration = 0;
  bool just_built_fibers = false;
  boost::progress_display disp(m_Iterations);
  if (!m_AbortTracking)
    while (m_CurrentIteration<m_Iterations)
    {
      just_built_fibers = false;
      ++disp;
      m_CurrentIteration++;
      if (m_AbortTracking)
        break;

      // update temperatur for simulated annealing process
      float temperature = m_StartTemperature * exp(alpha*m_CurrentIteration/m_Iterations);
      sampler->SetTemperature(temperature);
      sampler->MakeProposal();

      m_ProposalAcceptance = (float)sampler->GetNumAcceptedProposals()/m_CurrentIteration;
      m_NumParticles = particleGrid->m_NumParticles;
      m_NumConnections = particleGrid->m_NumConnections;

      if (m_AbortTracking)
        break;

      if (m_BuildFibers)
      {
        FiberBuilder fiberBuilder(particleGrid, m_MaskImage);
        m_FiberPolyData = fiberBuilder.iterate(m_MinFiberLength);
        m_NumAcceptedFibers = m_FiberPolyData->GetNumberOfLines();
        m_BuildFibers = false;
        just_built_fibers = true;
      }
    }
  if (!just_built_fibers)
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

  //    sampler->PrintProposalTimes();

  SaveParameters();
}

template< class ItkOdfImageType >
void GibbsTrackingFilter< ItkOdfImageType >::PrepareMaskImage()
{
  if(m_MaskImage.IsNull())
  {
    MITK_INFO << "GibbsTrackingFilter: generating default mask image";
    m_MaskImage = ItkFloatImageType::New();
    m_MaskImage->SetSpacing( m_OdfImage->GetSpacing() );
    m_MaskImage->SetOrigin( m_OdfImage->GetOrigin() );
    m_MaskImage->SetDirection( m_OdfImage->GetDirection() );
    m_MaskImage->SetRegions( m_OdfImage->GetLargestPossibleRegion() );
    m_MaskImage->Allocate();
    m_MaskImage->FillBuffer(1.0);
  }
  else if ( m_MaskImage->GetLargestPossibleRegion().GetSize()[0]!=m_OdfImage->GetLargestPossibleRegion().GetSize()[0] ||
            m_MaskImage->GetLargestPossibleRegion().GetSize()[1]!=m_OdfImage->GetLargestPossibleRegion().GetSize()[1] ||
            m_MaskImage->GetLargestPossibleRegion().GetSize()[2]!=m_OdfImage->GetLargestPossibleRegion().GetSize()[2] ||
            m_MaskImage->GetSpacing()[0]!=m_OdfImage->GetSpacing()[0] ||
            m_MaskImage->GetSpacing()[1]!=m_OdfImage->GetSpacing()[1] ||
            m_MaskImage->GetSpacing()[2]!=m_OdfImage->GetSpacing()[2] )
  {
    MITK_INFO << "GibbsTrackingFilter: resampling mask image";
    typedef itk::ResampleImageFilter< ItkFloatImageType, ItkFloatImageType, float > ResamplerType;
    ResamplerType::Pointer resampler = ResamplerType::New();
    resampler->SetOutputSpacing( m_OdfImage->GetSpacing() );
    resampler->SetOutputOrigin( m_OdfImage->GetOrigin() );
    resampler->SetOutputDirection( m_OdfImage->GetDirection() );
    resampler->SetSize( m_OdfImage->GetLargestPossibleRegion().GetSize() );

    resampler->SetInput( m_MaskImage );
    resampler->SetDefaultPixelValue(0.0);
    resampler->Update();
    m_MaskImage = resampler->GetOutput();
    MITK_INFO << "GibbsTrackingFilter: resampling finished";
  }
}

// load tracking paramters from xml file (.gtp)
template< class ItkOdfImageType >
bool GibbsTrackingFilter< ItkOdfImageType >::LoadParameters()
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
    TiXmlHandle hRoot(nullptr);

    pElem = hDoc.FirstChildElement().Element();
    hRoot = TiXmlHandle(pElem);
    pElem = hRoot.FirstChildElement("parameter_set").Element();

    std::string iterations(pElem->Attribute("iterations"));
    m_Iterations = boost::lexical_cast<double>(iterations);

    std::string particleLength(pElem->Attribute("particle_length"));
    m_ParticleLength = boost::lexical_cast<float>(particleLength);

    std::string particleWidth(pElem->Attribute("particle_width"));
    m_ParticleWidth = boost::lexical_cast<float>(particleWidth);

    std::string partWeight(pElem->Attribute("particle_weight"));
    m_ParticleWeight = boost::lexical_cast<float>(partWeight);

    std::string startTemp(pElem->Attribute("temp_start"));
    m_StartTemperature = boost::lexical_cast<float>(startTemp);

    std::string endTemp(pElem->Attribute("temp_end"));
    m_EndTemperature = boost::lexical_cast<float>(endTemp);

    std::string inExBalance(pElem->Attribute("inexbalance"));
    m_InexBalance = boost::lexical_cast<float>(inExBalance);

    std::string fiberLength(pElem->Attribute("fiber_length"));
    m_MinFiberLength = boost::lexical_cast<float>(fiberLength);

    std::string curvThres(pElem->Attribute("curvature_threshold"));
    m_CurvatureThreshold = cos(boost::lexical_cast<float>(curvThres)*itk::Math::pi/180);
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
template< class ItkOdfImageType >
bool GibbsTrackingFilter< ItkOdfImageType >::SaveParameters()
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
    paramXML->SetAttribute("iterations", boost::lexical_cast<std::string>(m_Iterations));
    paramXML->SetAttribute("particle_length", boost::lexical_cast<std::string>(m_ParticleLength));
    paramXML->SetAttribute("particle_width", boost::lexical_cast<std::string>(m_ParticleWidth));
    paramXML->SetAttribute("particle_weight", boost::lexical_cast<std::string>(m_ParticleWeight));
    paramXML->SetAttribute("temp_start", boost::lexical_cast<std::string>(m_StartTemperature));
    paramXML->SetAttribute("temp_end", boost::lexical_cast<std::string>(m_EndTemperature));
    paramXML->SetAttribute("inexbalance", boost::lexical_cast<std::string>(m_InexBalance));
    paramXML->SetAttribute("fiber_length", boost::lexical_cast<std::string>(m_MinFiberLength));
    paramXML->SetAttribute("curvature_threshold", boost::lexical_cast<std::string>(m_CurvatureThreshold));
    mainXML->LinkEndChild(paramXML);

    if(!boost::algorithm::ends_with(m_SaveParameterFile, ".gtp"))
      m_SaveParameterFile.append(".gtp");
    documentXML.SaveFile( m_SaveParameterFile );

    MITK_INFO << "GibbsTrackingFilter: parameter file saved successfully";
    return true;
  }
  catch(...)
  {
    MITK_INFO << "GibbsTrackingFilter: could not save parameter file";
    return false;
  }
}

template< class ItkOdfImageType >
void GibbsTrackingFilter< ItkOdfImageType >::SetDicomProperties(mitk::FiberBundle::Pointer fib)
{
  std::string model_code_value = "-";
  std::string model_code_meaning = "-";
  std::string algo_code_value = "sup181_ee03";
  std::string algo_code_meaning = "Global";


  fib->SetProperty("DICOM.anatomy.value", mitk::StringProperty::New("T-A0095"));
  fib->SetProperty("DICOM.anatomy.meaning", mitk::StringProperty::New("White matter of brain and spinal cord"));

  fib->SetProperty("DICOM.algo_code.value", mitk::StringProperty::New(algo_code_value));
  fib->SetProperty("DICOM.algo_code.meaning", mitk::StringProperty::New(algo_code_meaning));

  fib->SetProperty("DICOM.model_code.value", mitk::StringProperty::New(model_code_value));
  fib->SetProperty("DICOM.model_code.meaning", mitk::StringProperty::New(model_code_meaning));
}

}




