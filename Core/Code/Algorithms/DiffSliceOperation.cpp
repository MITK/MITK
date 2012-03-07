/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "DiffSliceOperation.h"

#include <itkCommand.h>

mitk::DiffSliceOperation::DiffSliceOperation():Operation(1)
{
	m_TimeStep = 0;
	m_Slice = NULL;
	m_Image = NULL;
	m_WorldGeometry = NULL;
	m_ImageIsValid = false;
}


mitk::DiffSliceOperation::DiffSliceOperation(mitk::Image* imageVolume,
																						 mitk::Image* slice,
																						 unsigned int timestep,
																						 mitk::Geometry2D* currentWorldGeometry):Operation(1)
																						 
{
	m_WorldGeometry = dynamic_cast<mitk::Geometry2D*> (currentWorldGeometry->Clone().GetPointer());

	m_TimeStep = timestep;

	/*m_zlibSliceContainer = CompressedImageContainer::New();
  m_zlibSliceContainer->SetImage( slice );*/
	m_Slice = slice;

	m_Image = imageVolume;


	itk::SimpleMemberCommand< DiffSliceOperation >::Pointer command = itk::SimpleMemberCommand< DiffSliceOperation >::New();
    command->SetCallbackFunction( this, &DiffSliceOperation::OnImageDeleted );
    m_DeleteTag = imageVolume->AddObserver( itk::DeleteEvent(), command );

		if ( m_Image) 
			m_ImageIsValid = true;
		else
			m_ImageIsValid = false;

}

mitk::DiffSliceOperation::~DiffSliceOperation()
{
	m_Image = NULL;
	m_Slice = NULL;
	m_WorldGeometry->Delete();
	m_WorldGeometry = NULL;
	//m_zlibSliceContainer = NULL;
	if (m_ImageIsValid)
  {
    m_Image->RemoveObserver( m_DeleteTag );
  }
}

mitk::Image* mitk::DiffSliceOperation::GetSlice()
{
	//Image::ConstPointer image = m_zlibSliceContainer->GetImage().GetPointer();
	return m_Slice;
}

bool mitk::DiffSliceOperation::IsValid()
{
  return m_ImageIsValid;//TODO improve
}

void mitk::DiffSliceOperation::OnImageDeleted()
{
  m_ImageIsValid = false;
}