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
#include <tinyxml.h>

#include <itksys/SystemTools.hxx>
mitk::NavigationDataRecorder::NavigationDataRecorder()
{
  m_NumberOfInputs = 0;
  m_RecordingMode = NormalFile;
  m_Recording = false;
  m_NumberOfRecordedFiles = 0;
  m_Stream = NULL;
  m_FileName = "";

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

    for (unsigned int index = 0; index < inputs.size(); index++)
    {
      mitk::NavigationData* nd = dynamic_cast<mitk::NavigationData*>(inputs[index].GetPointer());
      nd->Update();

      mitk::NavigationData::PositionType position;
      mitk::NavigationData::OrientationType orientation(0.0, 0.0, 0.0, 0.0);
      mitk::NavigationData::CovarianceMatrixType matrix;

      bool hasPosition = true;    
      bool hasOrientation = true; 
      bool dataValid = false;

      position.Fill(0.0);
      matrix.SetIdentity();

      position = nd->GetPosition();
      orientation = nd->GetOrientation();
      matrix = nd->GetCovErrorMatrix();
      
      hasPosition = nd->GetHasPosition();
      hasOrientation = nd->GetHasOrientation();
      dataValid = nd->IsDataValid();

      //use this one if you want the timestamps of the source
      //timestamp = nd->GetTimeStamp();

      //a timestamp is never < 0! this case happens only if you are using the timestamp of the nd object instead of getting a new one
      if (timestamp >= 0)
      { 
        TiXmlElement* elem = new TiXmlElement("ND");

        elem->SetDoubleAttribute("Time", timestamp);
        elem->SetDoubleAttribute("Tool", index);
        elem->SetDoubleAttribute("X", position[0]);
        elem->SetDoubleAttribute("Y", position[1]);
        elem->SetDoubleAttribute("Z", position[2]);

        elem->SetDoubleAttribute("QX", orientation[0]);
        elem->SetDoubleAttribute("QY", orientation[1]);
        elem->SetDoubleAttribute("QZ", orientation[2]);
        elem->SetDoubleAttribute("QR", orientation[3]);

        elem->SetDoubleAttribute("C00", matrix[0][0]);
        elem->SetDoubleAttribute("C01", matrix[0][1]);
        elem->SetDoubleAttribute("C02", matrix[0][2]);
        elem->SetDoubleAttribute("C03", matrix[0][3]);
        elem->SetDoubleAttribute("C04", matrix[0][4]);
        elem->SetDoubleAttribute("C05", matrix[0][5]);
        elem->SetDoubleAttribute("C10", matrix[1][0]);
        elem->SetDoubleAttribute("C11", matrix[1][1]);
        elem->SetDoubleAttribute("C12", matrix[1][2]);
        elem->SetDoubleAttribute("C13", matrix[1][3]);
        elem->SetDoubleAttribute("C14", matrix[1][4]);
        elem->SetDoubleAttribute("C15", matrix[1][5]);

        if (dataValid)
          elem->SetAttribute("Valid",1);
        else
          elem->SetAttribute("Valid",0);

        if (hasOrientation)
          elem->SetAttribute("hO",1);
        else
          elem->SetAttribute("hO",0);

        if (hasPosition)
          elem->SetAttribute("hP",1);
        else
          elem->SetAttribute("hP",0);
        
        *m_Stream << "        " << *elem << std::endl;

        delete elem;
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
    
    //An existing extension will be cut and replaced with .xml
    std::string tmpPath = itksys::SystemTools::GetFilenamePath(m_FileName);
    m_FileName = itksys::SystemTools::GetFilenameWithoutExtension(m_FileName);
    ss << tmpPath << "/" <<  m_FileName << "-" << m_NumberOfRecordedFiles << ".xml";
    switch(m_RecordingMode)
    {
      case Console:
        stream = &std::cout;
        break;
      case NormalFile:

        //Check if there is a file name and path
        if (m_FileName == "")
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