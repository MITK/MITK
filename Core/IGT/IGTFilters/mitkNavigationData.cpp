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


mitk::NavigationData::NavigationData() 
: itk::DataObject(), 
  /*m_TimeStamp(NULL),*/ 
  m_Orientation(0.0, 0.0, 0.0, 0.0),
  m_Error(-1.0), 
  m_DataValid(false) 
{
  m_Position.Fill(0.0);
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
  this->SetError(nd->GetError());
}


bool mitk::NavigationData::IsDataValid() const
{
  return m_DataValid;
}


void mitk::NavigationData::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "data valid: " << m_DataValid << std::endl;
  os << indent << "Position: " << m_Position << std::endl;
  os << indent << "Orientation: " << m_Orientation << std::endl;
  os << indent << "Error: " << m_Error << std::endl;
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
