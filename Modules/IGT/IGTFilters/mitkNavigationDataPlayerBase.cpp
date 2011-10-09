/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-02-10 18:08:54 +0100 (Di, 10 Feb 2009) $
Version:   $Revision: 16228 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataPlayerBase.h"


mitk::NavigationDataPlayerBase::NavigationDataPlayerBase()
{
 m_StreamValid = true;
 m_ErrorMessage = "";
}

mitk::NavigationDataPlayerBase::~NavigationDataPlayerBase()
{
}

 



void mitk::NavigationDataPlayerBase::UpdateOutputInformation()
{
  this->Modified();  // make sure that we need to be updated
  Superclass::UpdateOutputInformation();
}



mitk::NavigationData::Pointer mitk::NavigationDataPlayerBase::ReadNavigationData(TiXmlElement* elem)
{
  mitk::NavigationData::Pointer nd = mitk::NavigationData::New();

  mitk::NavigationData::PositionType position;
  mitk::NavigationData::OrientationType orientation(0.0,0.0,0.0,0.0);
  mitk::NavigationData::TimeStampType timestamp = -1; 
  mitk::NavigationData::CovarianceMatrixType matrix;

  bool hasPosition = true;    
  bool hasOrientation = true; 
  bool dataValid = false;

  position.Fill(0.0);
  matrix.SetIdentity();


  elem->QueryDoubleAttribute("Time",&timestamp);
  if (timestamp == -1)
  {
    return NULL;  //the calling method should check the return value if it is valid/not NULL
  }

  elem->QueryFloatAttribute("X", &position[0]);
  elem->QueryFloatAttribute("Y", &position[1]);
  elem->QueryFloatAttribute("Z", &position[2]);

  elem->QueryFloatAttribute("QX", &orientation[0]);
  elem->QueryFloatAttribute("QY", &orientation[1]);
  elem->QueryFloatAttribute("QZ", &orientation[2]);
  elem->QueryFloatAttribute("QR", &orientation[3]);

  elem->QueryFloatAttribute("C00", &matrix[0][0]);
  elem->QueryFloatAttribute("C01", &matrix[0][1]);
  elem->QueryFloatAttribute("C02", &matrix[0][2]);
  elem->QueryFloatAttribute("C03", &matrix[0][3]);
  elem->QueryFloatAttribute("C04", &matrix[0][4]);
  elem->QueryFloatAttribute("C05", &matrix[0][5]);
  elem->QueryFloatAttribute("C10", &matrix[1][0]);
  elem->QueryFloatAttribute("C11", &matrix[1][1]);
  elem->QueryFloatAttribute("C12", &matrix[1][2]);
  elem->QueryFloatAttribute("C13", &matrix[1][3]);
  elem->QueryFloatAttribute("C14", &matrix[1][4]);
  elem->QueryFloatAttribute("C15", &matrix[1][5]);

  int tmpval = 0;
  elem->QueryIntAttribute("Valid", &tmpval);
  if (tmpval == 0)
    dataValid = false;
  else
    dataValid = true;

  tmpval = 0;
  elem->QueryIntAttribute("hO", &tmpval);
  if (tmpval == 0)
    hasOrientation = false;
  else
    hasOrientation = true;

  tmpval = 0;
  elem->QueryIntAttribute("hP", &tmpval);
  if (tmpval == 0)
    hasPosition = false;
  else
    hasPosition = true;

  nd->SetTimeStamp(timestamp);
  nd->SetPosition(position);
  nd->SetOrientation(orientation);
  nd->SetCovErrorMatrix(matrix);
  nd->SetDataValid(dataValid);
  nd->SetHasOrientation(hasOrientation);
  nd->SetHasPosition(hasPosition);


  return nd;
}
