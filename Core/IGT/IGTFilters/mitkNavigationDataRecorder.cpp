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
}




mitk::NavigationDataRecorder::~NavigationDataRecorder()
{
}


void mitk::NavigationDataRecorder::GenerateData()
{
  DataObjectPointerArray inputs = this->GetInputs(); //get all inputs

  for (unsigned int index=0; index<inputs.size(); index++)
  {
    mitk::NavigationData* nd = dynamic_cast<mitk::NavigationData*>(inputs[index].GetPointer()) ;
    mitk::NavigationData::PositionType position;
    mitk::NavigationData::OrientationType orientation;

    position = nd->GetPosition();
    orientation = nd->GetOrientation();

    TiXmlDocument doc = m_XMLFiles[index];
    TiXmlElement* elementData = new TiXmlElement("ND");
    
    elementData->SetDoubleAttribute("X",position[0]);
    elementData->SetDoubleAttribute("Y",position[1]);
    elementData->SetDoubleAttribute("Z",position[2]);

    //TODO name them right
    elementData->SetDoubleAttribute("q0",orientation[0]);
    elementData->SetDoubleAttribute("q1",orientation[1]);
    elementData->SetDoubleAttribute("q2",orientation[2]);
    elementData->SetDoubleAttribute("z",orientation[3]);

    //TODO timestamp

    doc.LinkEndChild( elementData );
  }
  SaveToDisc();
}

void mitk::NavigationDataRecorder::AddNavigationData( const NavigationData* nd )
{
  
  // Process object is not const-correct so the const_cast is required here
  this->SetNthInput(m_NumberOfInputs, 
    const_cast< mitk::NavigationData * >( nd ) );

  m_NumberOfInputs++;

  TiXmlDocument doc;
  TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
  doc.LinkEndChild( decl );
  TiXmlElement* elementVersion = new TiXmlElement("Version");
  elementVersion->SetAttribute("Version", 1);
  doc.LinkEndChild( elementVersion );
  m_XMLFiles.push_back( doc );

}

void mitk::NavigationDataRecorder::SaveToDisc()
{
  std::stringstream ss;
  for (unsigned int index=0; index < m_XMLFiles.size(); index++)
  {
    ss << m_FileName << " " << index;
    m_XMLFiles[index].SaveFile(ss.str().c_str());
  }
}
