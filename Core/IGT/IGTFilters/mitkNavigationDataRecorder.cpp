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



mitk::NavigationDataRecorder::NavigationDataRecorder()
{
  m_NumberOfInputs = 0;
  m_OutputMode = Console;
  m_Recording = false;
  m_NumberOfRecordedFiles = 0;
  m_Stream = NULL;
  //m_Stream->tie(std::cout);
}




mitk::NavigationDataRecorder::~NavigationDataRecorder()
{
}


void mitk::NavigationDataRecorder::GenerateData()
{

  //for (unsigned int index=0; index<inputs.size(); index++)
  //{
  //  mitk::NavigationData* nd = dynamic_cast<mitk::NavigationData*>(inputs[index].GetPointer()) ;
  //  mitk::NavigationData::PositionType position;
  //  mitk::NavigationData::OrientationType orientation;

  //  position = nd->GetPosition();
  //  orientation = nd->GetOrientation();

  //  TiXmlDocument doc = m_XMLFiles[index];
  //  TiXmlElement* elementData = new TiXmlElement("ND");
  //  
  //  elementData->SetDoubleAttribute("X",position[0]);
  //  elementData->SetDoubleAttribute("Y",position[1]);
  //  elementData->SetDoubleAttribute("Z",position[2]);

  //  //TODO name them right
  //  elementData->SetDoubleAttribute("q0",orientation[0]);
  //  elementData->SetDoubleAttribute("q1",orientation[1]);
  //  elementData->SetDoubleAttribute("q2",orientation[2]);
  //  elementData->SetDoubleAttribute("z",orientation[3]);

  //  //TODO timestamp

  //  doc.LinkEndChild( elementData );
  //}
  //SaveToDisc();
}

void mitk::NavigationDataRecorder::AddNavigationData( const NavigationData* nd )
{
  // Process object is not const-correct so the const_cast is required here
  this->SetNthInput(m_NumberOfInputs, 
    const_cast< mitk::NavigationData * >( nd ) );

  m_NumberOfInputs++;

  this->Modified();
}

void mitk::NavigationDataRecorder::SetOutputMode( OutputMode mode )
{
  m_OutputMode = mode;
  this->Modified();
}

void mitk::NavigationDataRecorder::Update()
{
  if (m_Recording)
  {
    DataObjectPointerArray inputs = this->GetInputs(); //get all inputs
    //double timestamp;
    for (unsigned int index=0; index<inputs.size(); index++)
    {
      mitk::NavigationData* nd = dynamic_cast<mitk::NavigationData*>(inputs[index].GetPointer()) ;
      mitk::NavigationData::PositionType position;
      mitk::NavigationData::OrientationType orientation;

      position = nd->GetPosition();
      orientation = nd->GetOrientation();

      //TODO timestamp name the quaternion right
      *m_Stream << "        " << "<ND Tool=\"" << index << "\" X=\"" << position[0] << "\" Y=\"" << position[1] << "\" Z=\"" << position[2] 
      << " Q0=\"" << orientation[0] << "\" Q1=\"" << orientation[1] << "\" Q2=\"" << orientation[2] << "\" Z0=\"" << orientation[3] << "\" />" << std::endl;

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
    switch(m_OutputMode)
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

  if (m_Stream)
  {
    *m_Stream << "<?xml version=\"1.0\" ?>" << std::endl;
    *m_Stream << "<Version Ver=\"1\" />" << std::endl;
    *m_Stream << "    " << "<Data Count=\"" << (m_NumberOfInputs) << "\">" << std::endl;

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