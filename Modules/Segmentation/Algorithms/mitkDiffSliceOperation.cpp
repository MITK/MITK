/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDiffSliceOperation.h"

#include <mitkImage.h>

#include <itkCommand.h>

mitk::DiffSliceOperation::DiffSliceOperation() : Operation(1)
{
  m_TimeStep = 0;
  m_Image = nullptr;
  m_WorldGeometry = nullptr;
  m_SliceGeometry = nullptr;
  m_ImageIsValid = false;
  m_DeleteObserverTag = 0;
}

mitk::DiffSliceOperation::DiffSliceOperation(Image *imageVolume,
                                             const Image *slice,
                                             const SlicedGeometry3D *sliceGeometry,
                                             TimeStepType timestep,
                                             const BaseGeometry *currentWorldGeometry)
  : Operation(1)

{
  m_WorldGeometry = currentWorldGeometry->Clone();

  /*
  Quick fix for bug 12338.
  Guard object - fix this when clone method of PlaneGeometry is cloning the reference geometry (see bug 13392)*/
  // xxxx m_GuardReferenceGeometry = mitk::BaseGeometry::New();
  m_GuardReferenceGeometry = dynamic_cast<const PlaneGeometry *>(m_WorldGeometry.GetPointer())->GetReferenceGeometry();
  /*---------------------------------------------------------------------------------------------------*/

  m_SliceGeometry = sliceGeometry->Clone();

  m_TimeStep = timestep;

  m_CompressedImageContainer.CompressImage(slice);

  m_Image = imageVolume;
  m_DeleteObserverTag = 0;

  if (m_Image)
  {
    /*add an observer to listen to the delete event of the image, this is necessary because the operation is then
     * invalid*/
    itk::SimpleMemberCommand<DiffSliceOperation>::Pointer command = itk::SimpleMemberCommand<DiffSliceOperation>::New();
    command->SetCallbackFunction(this, &DiffSliceOperation::OnImageDeleted);
    // get the id of the observer, used to remove it later on
    m_DeleteObserverTag = imageVolume->AddObserver(itk::DeleteEvent(), command);

    m_ImageIsValid = true;
  }
  else
    m_ImageIsValid = false;
}

mitk::DiffSliceOperation::~DiffSliceOperation()
{
  m_WorldGeometry = nullptr;

  if (m_ImageIsValid)
  {
    // if the image is still there, we have to remove the observer from it
    m_Image->RemoveObserver(m_DeleteObserverTag);
  }
  m_Image = nullptr;
}

mitk::Image::Pointer mitk::DiffSliceOperation::GetSlice()
{
  return m_CompressedImageContainer.DecompressImage();
}

bool mitk::DiffSliceOperation::IsValid()
{
  return m_ImageIsValid && m_WorldGeometry.IsNotNull(); // TODO improve
}

void mitk::DiffSliceOperation::OnImageDeleted()
{
  // if our imageVolume is removed e.g. from the datastorage the operation is no lnger valid
  m_ImageIsValid = false;
}
