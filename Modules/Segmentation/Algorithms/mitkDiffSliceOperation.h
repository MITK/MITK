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

#ifndef mitkDiffSliceOperation_h_Included
#define mitkDiffSliceOperation_h_Included

#include "SegmentationExports.h"
#include "mitkCommon.h"
#include <mitkOperation.h>
//#include "mitkCompressedImageContainer.h"

#include <mitkImage.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>


namespace mitk
{
  /** \brief An Operation for applying an edited slice to the volume.
    \sa DiffSliceOperationApplier

    The information for the operation is specified by properties:

     imageVolume            the volume where the slice was extracted from.
     slice                  the slice to be applied.
     timestep               the timestep in an 4D image.
     currentWorldGeometry   specifies the axis where the slice has to be applied in the volume.

    This Operation can be used to realize undo-redo functionality for e.g. segmentation purposes.
  */
  class Segmentation_EXPORT DiffSliceOperation : public Operation
  {

  public:

    mitkClassMacro(DiffSliceOperation, OperationActor);

    //itkNewMacro(DiffSliceOperation);

    //mitkNewMacro4Param(DiffSliceOperation,mitk::Image,mitk::Image,unsigned int, mitk::Geometry2D);

    /** \brief Creates an empty instance.
      Note that it is not valid yet. The properties of the object have to be set.
    */
    DiffSliceOperation();

    /** \brief */
    DiffSliceOperation( mitk::Image* imageVolume, vtkImageData* slice, AffineGeometryFrame3D* sliceGeometry, unsigned int timestep, AffineGeometryFrame3D* currentWorldGeometry);

    /** \brief Check if it is a valid operation.*/
    bool IsValid();

    /** \brief Set the image volume.*/
    void SetImage(mitk::Image* image){ this->m_Image = image;}
    /** \brief Get th image volume.*/
    mitk::Image* GetImage(){return this->m_Image;}

    /** \brief Set thee slice to be applied.*/
    void SetImage(vtkImageData* slice){ this->m_Slice = slice;}
    /** \brief Get the slice that is applied in the operation.*/
    vtkImageData* GetSlice();

    /** \brief Get timeStep.*/
    void SetTimeStep(unsigned int timestep){this->m_TimeStep = timestep;}
    /** \brief Set timeStep*/
    unsigned int GetTimeStep(){return this->m_TimeStep;}

    /** \brief Set the axis where the slice has to be applied in the volume.*/
    void SetSliceGeometry(AffineGeometryFrame3D* sliceGeometry){this->m_SliceGeometry = sliceGeometry;}
    /** \brief Get the axis where the slice has to be applied in the volume.*/
    AffineGeometryFrame3D* GetSliceGeometry(){return this->m_SliceGeometry;}

    /** \brief Set the axis where the slice has to be applied in the volume.*/
    void SetCurrentWorldGeometry(AffineGeometryFrame3D* worldGeometry){this->m_WorldGeometry = worldGeometry;}
    /** \brief Get the axis where the slice has to be applied in the volume.*/
    AffineGeometryFrame3D* GetWorldGeometry(){return this->m_WorldGeometry;}


  protected:

    virtual ~DiffSliceOperation();

    /** \brief Callback for image observer.*/
    void OnImageDeleted();

    //CompressedImageContainer::Pointer m_zlibSliceContainer;

    mitk::Image* m_Image;

    vtkSmartPointer<vtkImageData> m_Slice;

    AffineGeometryFrame3D::Pointer m_SliceGeometry;

    unsigned int m_TimeStep;

    AffineGeometryFrame3D::Pointer m_WorldGeometry;

    bool m_ImageIsValid;

    unsigned long m_DeleteObserverTag;

    mitk::Geometry3D::Pointer m_GuardReferenceGeometry;

  };
}
#endif