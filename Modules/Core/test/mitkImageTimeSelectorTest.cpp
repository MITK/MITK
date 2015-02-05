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


#include "mitkImage.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageGenerator.h"

#include "mitkTestingMacros.h"

#include "mitkIOUtil.h"
#include <itksys/SystemTools.hxx>

#include <fstream>


/** Global members common for all subtests */
namespace
{
  std::string m_Filename;
  mitk::Image::Pointer m_Image;
} // end of anonymous namespace

/** @brief Global test setup */
static void Setup( )
{
  try
  {
    m_Image = mitk::IOUtil::LoadImage( m_Filename );
  }
  catch( const itk::ExceptionObject &e)
  {
    MITK_TEST_FAILED_MSG(<< "(Setup) Caught exception from IOUtil while loading input : " << m_Filename <<"\n" << e.what())
  }
}

static void Valid_AllInputTimesteps_ReturnsTrue()
{
  Setup();

  mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
  timeSelector->SetInput(m_Image);

  // test all timesteps
  const unsigned int maxTimeStep = m_Image->GetTimeSteps();
  for( unsigned int t=0; t<maxTimeStep; t++)
  {
    timeSelector->SetTimeNr(t);
    timeSelector->Update();

    mitk::Image::Pointer currentTimestepImage = timeSelector->GetOutput();

    std::stringstream ss;
    ss << " : Valid image in timestep " << t ;

    MITK_TEST_CONDITION_REQUIRED( currentTimestepImage.IsNotNull()
                                  , ss.str().c_str() );
  }


}

static void Valid_ImageExpandedByTimestep_ReturnsTrue()
{
  Setup();

  mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();

  const unsigned int maxTimeStep = m_Image->GetTimeSteps();
  mitk::TimeGeometry* tsg = m_Image->GetTimeGeometry();
  mitk::ProportionalTimeGeometry* ptg = dynamic_cast<mitk::ProportionalTimeGeometry*>(tsg);
  ptg->Expand(maxTimeStep+1);
  ptg->SetTimeStepGeometry( ptg->GetGeometryForTimeStep(0), maxTimeStep );

  mitk::Image::Pointer expandedImage = mitk::Image::New();
  expandedImage->Initialize( m_Image->GetPixelType(0), *tsg );

  timeSelector->SetInput(expandedImage);

  for( unsigned int t=0; t<maxTimeStep+1; t++)
  {
    timeSelector->SetTimeNr(t);
    timeSelector->Update();

    mitk::Image::Pointer currentTimestepImage = timeSelector->GetOutput();

    std::stringstream ss;
    ss << " : Valid image in timestep " << t ;

    MITK_TEST_CONDITION_REQUIRED( currentTimestepImage.IsNotNull()
                                  , ss.str().c_str() );
  }
}

int mitkImageTimeSelectorTest(int /*argc*/, char* argv[])
{
  MITK_TEST_BEGIN(mitkImageTimeSelectorTest);

  m_Filename = std::string( argv[1] );

  Valid_AllInputTimesteps_ReturnsTrue();
  Valid_ImageExpandedByTimestep_ReturnsTrue();

  MITK_TEST_END();
}
