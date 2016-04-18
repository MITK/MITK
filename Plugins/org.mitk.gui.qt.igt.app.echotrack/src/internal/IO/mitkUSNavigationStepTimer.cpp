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

#include "mitkUSNavigationStepTimer.h"

#include <fstream>
#include <sstream>

mitk::USNavigationStepTimer::USNavigationStepTimer()
  : m_CurrentIndex(0)
{
}

mitk::USNavigationStepTimer::~USNavigationStepTimer()
{
  // make sure that the timer will be stopped and an active
  // measurement will be written to the output file
  this->Stop();
}

void mitk::USNavigationStepTimer::Stop()
{
  this->FinishCurrentIndex();
}

void mitk::USNavigationStepTimer::Reset()
{
  m_Durations.clear();
  m_TimeProbe = itk::TimeProbe();
}

void mitk::USNavigationStepTimer::SetOutputFileName(std::string filename)
{
  m_OutputFileName = filename;

  // write all durations to the file, which were measured up to now
  if ( ! m_Durations.empty() ) { this->WriteEverythingToFile(); }
}

void mitk::USNavigationStepTimer::SetActiveIndex(unsigned int index, std::string description)
{
  this->FinishCurrentIndex();

  m_CurrentIndex = index;
  m_CurrentDescription = description;

  m_TimeProbe.Start();
}

double mitk::USNavigationStepTimer::GetTotalDuration()
{
  double duration = 0;

  // sum all durations
  for ( DurationsVector::iterator it = m_Durations.begin();
    it != m_Durations.end(); ++it )
  {
    duration += it->duration;
  }

  return duration;
}

double mitk::USNavigationStepTimer::GetDurationForIndex(unsigned int index)
{
  double duration = 0;

  // sum durations of the given index
  for ( DurationsVector::iterator it = m_Durations.begin();
    it != m_Durations.end(); ++it )
  {
    if ( it->index == index ) { duration += it->duration; }
  }

  return duration;
}

void mitk::USNavigationStepTimer::FinishCurrentIndex()
{
  if ( m_TimeProbe.GetNumberOfStarts() > 0 )
  {
    // save measurement of the current step
    m_TimeProbe.Stop();
    m_Durations.push_back(DurationForIndex(m_CurrentIndex, m_TimeProbe.GetTotal(), m_CurrentDescription));

    if ( ! m_OutputFileName.empty() )
    {
      this->WriteLineToFile(*(m_Durations.end()-1));
    }
  }

  m_TimeProbe = itk::TimeProbe();
}

void mitk::USNavigationStepTimer::WriteEverythingToFile()
{
  std::ofstream filestream(m_OutputFileName.c_str());
  if ( filestream.is_open() )
  {
    for ( DurationsVector::iterator it = m_Durations.begin();
      it != m_Durations.end(); ++it )
    {
      filestream << it->index << ";" << it->duration << ";" << it->description << std::endl;
    }
    filestream.close();
  }
  else
  {
    std::string filename = m_OutputFileName;
    m_OutputFileName.clear();
    mitkThrow() << "Cannot open file '" << filename << "' for writing.";
  }
}

void mitk::USNavigationStepTimer::WriteLineToFile(const DurationForIndex& element)
{
  std::ofstream filestream(m_OutputFileName.c_str(), std::ofstream::out | std::ofstream::app);
  if ( filestream.is_open() )
  {
    filestream << element.index << ";" << element.duration << ";" << element.description << std::endl;
    filestream.close();
  }
  else
  {
    std::string filename = m_OutputFileName;
    m_OutputFileName.clear();
    mitkThrow() << "Cannot open file '" << filename << "' for writing.";
  }
}

void mitk::USNavigationStepTimer::ReadFromFile(std::string filename)
{
  m_Durations.clear();

  std::ifstream filestream(filename.c_str());
  if (! filestream.is_open() )
  {
    mitkThrow() << "Cannot open file '" << filename << "' for reading.";
  }

  std::string line;
  while ( std::getline(filestream, line) )
  {
    size_t position = line.find(";");

    // make sure that the format of the file is correct
    if ( position == std::string::npos )
    {
      mitkThrow() << "File \"" << "\" contains invalid line (no ';' found).";
    }

    // the index is the first part of the line
    std::stringstream streamIndex(line.substr(0, position));
    unsigned int index;
    streamIndex >> index;

    line = line.substr(position+1);
    position = line.find(";");

    // make sure that the format of the file is correct
    if ( position == std::string::npos )
    {
      mitkThrow() << "File \"" << "\" contains invalid line (no ';' found).";
    }

    // the duration is the second part
    std::stringstream streamDuration(line.substr(0, position+1));
    double duration;
    streamDuration >> duration;

    // the description is the third part (last part)
    std::string description = line.substr(position+1);

    m_Durations.push_back(DurationForIndex(index, duration, description));
  }
}
