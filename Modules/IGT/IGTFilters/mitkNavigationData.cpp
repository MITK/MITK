/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationData.h"


mitk::NavigationData::NavigationData() : itk::DataObject(), 
m_Position(), m_Orientation(0.0, 0.0, 0.0, 0.0), m_CovErrorMatrix(),
m_HasPosition(true), m_HasOrientation(true), m_DataValid(false), m_TimeStamp(0.0),
m_Name()
{
  m_Position.Fill(0.0);
  m_CovErrorMatrix.SetIdentity();
}

mitk::NavigationData::~NavigationData()
{
}


void mitk::NavigationData::Graft( const DataObject *data )
{
  // Attempt to cast data to an NavigationData
  const Self* nd;
  try
  {
    nd = dynamic_cast<const Self *>( data );
  }
  catch( ... )
  {
    itkExceptionMacro( << "mitk::NavigationData::Graft cannot cast "
      << typeid(data).name() << " to "
      << typeid(const Self *).name() );
    return;
  }
  if (!nd)
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::NavigationData::Graft cannot cast "
      << typeid(data).name() << " to "
      << typeid(const Self *).name() );
    return;
  }
  // Now copy anything that is needed
  this->SetPosition(nd->GetPosition());
  this->SetOrientation(nd->GetOrientation());
  this->SetDataValid(nd->IsDataValid());
  this->SetTimeStamp(nd->GetTimeStamp());
  this->SetHasPosition(nd->GetHasPosition());
  this->SetHasOrientation(nd->GetHasOrientation());
  this->SetCovErrorMatrix(nd->GetCovErrorMatrix());
}


bool mitk::NavigationData::IsDataValid() const
{
  return m_DataValid;
}


void mitk::NavigationData::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "data valid: "     << this->IsDataValid() << std::endl;
  os << indent << "Position: "       << this->GetPosition() << std::endl;
  os << indent << "TimeStamp: "      << this->GetTimeStamp() << std::endl;
  os << indent << "HasPosition: "    << this->GetHasPosition() << std::endl;
  os << indent << "HasOrientation: " << this->GetHasOrientation() << std::endl;
  os << indent << "CovErrorMatrix: " << this->GetCovErrorMatrix() << std::endl;
}


void mitk::NavigationData::CopyInformation( const DataObject* data )
{
  this->Superclass::CopyInformation( data );

  const Self * nd = NULL;
  try
  {
    nd = dynamic_cast<const Self*>(data);
  }
  catch( ... )
  {
    // data could not be cast back down
    itkExceptionMacro(<< "mitk::NavigationData::CopyInformation() cannot cast "
      << typeid(data).name() << " to "
      << typeid(Self*).name() );
  }
  if ( !nd )
  {
    // pointer could not be cast back down
    itkExceptionMacro(<< "mitk::NavigationData::CopyInformation() cannot cast "
      << typeid(data).name() << " to "
      << typeid(Self*).name() );
  }
  /* copy all meta data */
}


void mitk::NavigationData::SetPositionAccuracy(mitk::ScalarType error)
{
  for ( int i = 0; i < 3; i++ )
    for ( int j = 0; j < 3; j++ ) 
    {
      m_CovErrorMatrix[ i ][ j ] = 0;
      // assume independence of position and orientation
      m_CovErrorMatrix[ i + 3 ][ j ] = 0;
      m_CovErrorMatrix[ i ][ j + 3 ] = 0;
    }
  m_CovErrorMatrix[0][0] = m_CovErrorMatrix[1][1] = m_CovErrorMatrix[2][2] = error * error;
}


void mitk::NavigationData::SetOrientationAccuracy(mitk::ScalarType error)
{
  for ( int i = 0; i < 3; i++ )
    for ( int j = 0; j < 3; j++ ) {
      m_CovErrorMatrix[ i + 3 ][ j + 3 ] = 0;
      // assume independence of position and orientation
      m_CovErrorMatrix[ i + 3 ][ j ] = 0;
      m_CovErrorMatrix[ i ][ j + 3 ] = 0;
    }
  m_CovErrorMatrix[3][3] = m_CovErrorMatrix[4][4] = m_CovErrorMatrix[5][5] = error * error;
}
