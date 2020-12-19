/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDiffSliceOperation_h_Included
#define mitkDiffSliceOperation_h_Included

#include "mitkCompressedImageContainer.h"
#include <MitkSegmentationExports.h>
#include <mitkOperation.h>

#include <vtkSmartPointer.h>

namespace mitk
{
  class Image;

  /** \brief An Operation for applying an edited slice to the volume.
    \sa DiffSliceOperationApplier

    The information for the operation is specified by properties:

     imageVolume            the volume where the slice was extracted from.
     slice                  the slice to be applied.
     timestep               the timestep in an 4D image.
     currentWorldGeometry   specifies the axis where the slice has to be applied in the volume.

    This Operation can be used to realize undo-redo functionality for e.g. segmentation purposes.
  */
  class MITKSEGMENTATION_EXPORT DiffSliceOperation : public Operation
  {
  public:
    mitkClassMacro(DiffSliceOperation, OperationActor);

    // itkFactorylessNewMacro(Self)
    // itkCloneMacro(Self)

    // mitkNewMacro4Param(DiffSliceOperation,mitk::Image,mitk::Image,unsigned int, mitk::PlaneGeometry);

    /** \brief Creates an empty instance.
      Note that it is not valid yet. The properties of the object have to be set.
    */
    DiffSliceOperation();

    /** \brief */
    DiffSliceOperation(mitk::Image *imageVolume,
                       const mitk::Image *slice,
                       const SlicedGeometry3D *sliceGeometry,
                       const TimeStepType timestep,
                       const BaseGeometry *currentWorldGeometry);

    /** \brief Check if it is a valid operation.*/
    bool IsValid();

    /** \brief Get th image volume.*/
    mitk::Image *GetImage() { return this->m_Image; }
    const mitk::Image* GetImage() const { return this->m_Image; }

    /** \brief Get the slice that is applied in the operation.*/
    Image::Pointer GetSlice();

    /** \brief Set timeStep*/
    TimeStepType GetTimeStep() const { return this->m_TimeStep; }
    /** \brief Get the axis where the slice has to be applied in the volume.*/
    const SlicedGeometry3D *GetSliceGeometry() const { return this->m_SliceGeometry; }
    /** \brief Get the axis where the slice has to be applied in the volume.*/
    const BaseGeometry *GetWorldGeometry() const { return this->m_WorldGeometry; }
  protected:
    ~DiffSliceOperation() override;

    /** \brief Callback for image observer.*/
    void OnImageDeleted();

    CompressedImageContainer::Pointer m_zlibSliceContainer;

    mitk::Image *m_Image;

    vtkSmartPointer<vtkImageData> m_Slice;

    SlicedGeometry3D::ConstPointer m_SliceGeometry;

    TimeStepType m_TimeStep;

    BaseGeometry::ConstPointer m_WorldGeometry;

    bool m_ImageIsValid;

    unsigned long m_DeleteObserverTag;

    mitk::BaseGeometry::ConstPointer m_GuardReferenceGeometry;
  };
}
#endif
