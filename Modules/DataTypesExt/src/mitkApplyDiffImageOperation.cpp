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


#include "mitkApplyDiffImageOperation.h"

#include <itkCommand.h>

mitk::ApplyDiffImageOperation::ApplyDiffImageOperation(OperationType operationType, Image* image, Image* diffImage, unsigned int timeStep, unsigned int sliceDimension, unsigned int sliceIndex)
: Operation(operationType),
  m_Image( image ),
  m_SliceIndex( sliceIndex ),
  m_SliceDimension( sliceDimension ),
  m_TimeStep( timeStep ),
  m_Factor( 1.0 ),
  m_ImageStillValid( false ),
  m_DeleteTag( 0 )
{
  if (image && diffImage)
  {
    // observe 3D image for DeleteEvent
    m_ImageStillValid = true;

    itk::SimpleMemberCommand< ApplyDiffImageOperation >::Pointer command = itk::SimpleMemberCommand< ApplyDiffImageOperation >::New();
    command->SetCallbackFunction( this, &ApplyDiffImageOperation::OnImageDeleted );
    m_DeleteTag = image->AddObserver( itk::DeleteEvent(), command );

    // keep a compressed version of the image
    zlibContainer = CompressedImageContainer::New();
    zlibContainer->SetImage( diffImage );
  }
}

mitk::ApplyDiffImageOperation::~ApplyDiffImageOperation()
{
  if (m_ImageStillValid)
  {
    m_Image->RemoveObserver( m_DeleteTag );
  }
}

void mitk::ApplyDiffImageOperation::OnImageDeleted()
{
  m_ImageStillValid = false;
}

mitk::Image::Pointer mitk::ApplyDiffImageOperation::GetDiffImage()
{
  // uncompress image to create a valid mitk::Image
  Image::Pointer image = zlibContainer->GetImage().GetPointer();

  return image;
}
