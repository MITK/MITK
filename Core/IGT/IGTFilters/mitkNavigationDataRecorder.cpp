/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkNavigationDataRecorder.h"
#include <fstream>

#include <mitkTimeStamp.h>

mitk::NavigationDataRecorder::NavigationDataRecorder()
{
  m_NumberOfInputs = 0;
  m_RecordingMode = Console;
  m_Recording = false;
  m_NumberOfRecordedFiles = 0;
  m_Stream = NULL;
  //m_Stream->tie(std::cout);

  //To get a start time
  mitk::TimeStamp::GetInstance()->StartTracking(this);

}

mitk::NavigationDataRecorder::~NavigationDataRecorder()
{
}


void mitk::NavigationDataRecorder::GenerateData()
{

}

void mitk::NavigationDataRecorder::AddNavigationData( const NavigationData* nd )
{
  // Process object is not const-correct so the const_cast is required here
  this->SetNthInput(m_NumberOfInputs, 
    const_cast< mitk::NavigationData * >( nd ) );

  m_NumberOfInputs++;

  this->Modified();
}

void mitk::NavigationDataRecorder::SetRecordingMode( RecordingMode mode )
{
  m_RecordingMode = mode;
  this->Modified();
}

void mitk::NavigationDataRecorder::Update()
{
  if (m_Recording)
  {
    DataObjectPointerArray inputs = this->GetInputs(); //get all inputs
    mitk::NavigationData::TimeStampType timestamp=0.0;
    timestamp = mitk::TimeStamp::GetInstance()->GetElapsed();

    for (unsigned int index=0; index<inputs.size(); index++)
    {
      mitk::NavigationData* nd = dynamic_cast<mitk::NavigationData*>(inputs[index].GetPointer()) ;
      mitk::NavigationData::PositionType position;
      mitk::NavigationData::OrientationType orientation;

      position = nd->GetPosition();
      orientation = nd->GetOrientation();
      
      //use this one if you want the timestamps of the source
      //timestamp = nd->GetTimeStamp();

      //a timestamp is never < 0! this case happens only if you are using the timestamp of the nd object instead of getting a new one
      //TODO to avoid possible corrupted files check if this was index 0 and do not write another tool / if neccessary overwrite other indexes
      if (timestamp >= 0)
      {             
        *m_Stream << "        " << "<ND Time=\"" << timestamp << "\" Tool=\"" << index << "\" X=\"" << position[0] << "\" Y=\"" << position[1] << "\" Z=\"" << position[2] 
        << " QX=\"" << orientation[0] << "\" QY=\"" << orientation[1] << "\" QZ=\"" << orientation[2] << "\" QR=\"" << orientation[3] << "\" />" << std::endl;
      }
    }
  }
}

void mitk::NavigationDataRecorder::StartRecording()
{
  if (m_Recording)
  {
    std::cout << "Already recording please stop before start new recording session" << std::endl;
    return;
  }
  if (m_Stream == NULL)
  {
    std::stringstream ss;
    std::ostream* stream;

    //TODO Save date to filename
    ss << m_FilePath << "/" << m_FileName << "-" << m_NumberOfRecordedFiles << ".xml";
    switch(m_RecordingMode)
    {
    case Console:
      stream = &std::cout;
      break;
    case NormalFile:
      
      //Check if there is a file name and path
      if (m_FileName == "" || m_FilePath == "")
      {
        stream = &std::cout;
        std::cout << "No file name or file path set the output is redirected to the console";
      }
      else
      {
        stream = new std::ofstream(ss.str().c_str());
      }
      
      break;
    case ZipFile:
      stream = &std::cout;
      std::cout << "Sorry no ZipFile support yet";
      break;
    default:
      stream = &std::cout;
      break;
    }
    StartRecording(stream);
  }

  


}
void mitk::NavigationDataRecorder::StartRecording(std::ostream* stream)
{
  if (m_Recording)
  {
    std::cout << "Already recording please stop before start new recording session" << std::endl;
    return;
  }

  m_Stream = stream;
  m_Stream->precision(10);
  //TODO store date and GMT time
  if (m_Stream)
  {
    *m_Stream << "<?xml version=\"1.0\" ?>" << std::endl;
    *m_Stream << "<Version Ver=\"1\" />" << std::endl;
    *m_Stream << "    " << "<Data ToolCount=\"" << (m_NumberOfInputs) << "\">" << std::endl;

    m_Recording = true;
  }
}
void mitk::NavigationDataRecorder::StopRecording()
{
  if (!m_Recording)
  {
    std::cout << "You have to start a recording first" << std::endl;
    return;
  }
  if (m_Stream)
  {
    *m_Stream << "</Data>" << std::endl;
  }

  m_NumberOfRecordedFiles++;
  m_Stream = NULL;
  m_Recording = false;
}