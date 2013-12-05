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

#include "mitkNavigationDataPlayerBase.h"


mitk::NavigationDataPlayerBase::NavigationDataPlayerBase() : m_StreamValid(true), m_ErrorMessage("")
{
  m_Name ="Navigation Data Player Source";
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
  if (elem == NULL) {mitkThrow() << "Error: Element is NULL!";}

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

  elem->QueryDoubleAttribute("X", &position[0]);
  elem->QueryDoubleAttribute("Y", &position[1]);
  elem->QueryDoubleAttribute("Z", &position[2]);

  elem->QueryDoubleAttribute("QX", &orientation[0]);
  elem->QueryDoubleAttribute("QY", &orientation[1]);
  elem->QueryDoubleAttribute("QZ", &orientation[2]);
  elem->QueryDoubleAttribute("QR", &orientation[3]);

  elem->QueryDoubleAttribute("C00", &matrix[0][0]);
  elem->QueryDoubleAttribute("C01", &matrix[0][1]);
  elem->QueryDoubleAttribute("C02", &matrix[0][2]);
  elem->QueryDoubleAttribute("C03", &matrix[0][3]);
  elem->QueryDoubleAttribute("C04", &matrix[0][4]);
  elem->QueryDoubleAttribute("C05", &matrix[0][5]);
  elem->QueryDoubleAttribute("C10", &matrix[1][0]);
  elem->QueryDoubleAttribute("C11", &matrix[1][1]);
  elem->QueryDoubleAttribute("C12", &matrix[1][2]);
  elem->QueryDoubleAttribute("C13", &matrix[1][3]);
  elem->QueryDoubleAttribute("C14", &matrix[1][4]);
  elem->QueryDoubleAttribute("C15", &matrix[1][5]);

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

  nd->SetIGTTimeStamp(timestamp);
  nd->SetPosition(position);
  nd->SetOrientation(orientation);
  nd->SetCovErrorMatrix(matrix);
  nd->SetDataValid(dataValid);
  nd->SetHasOrientation(hasOrientation);
  nd->SetHasPosition(hasPosition);


  return nd;
}
