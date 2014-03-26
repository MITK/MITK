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

#ifndef GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD
#define GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD

#include <MitkExports.h>
#include <mitkCommon.h>

#include "itkScalableAffineTransform.h"
#include <itkIndex.h>

#include "mitkBaseGeometry.h"

class vtkLinearTransform;

namespace mitk {
  //##Documentation
  //## @brief Standard typedef for time-bounds
  typedef itk::FixedArray<ScalarType,2>  TimeBounds;
  typedef itk::FixedArray<ScalarType, 3> FixedArrayType;

  //##Documentation
  //## @brief Standard 3D-BoundingBox typedef
  //##
  //## Standard 3D-BoundingBox typedef to get rid of template arguments (3D, type).
  typedef itk::BoundingBox<unsigned long, 3, ScalarType> BoundingBox;

  //##Documentation
  //## @brief Describes the geometry of a data object
  //##
  //## At least, it can return the bounding box of the data object.
  //##
  //## The class holds
  //## \li a bounding box which is axes-parallel in intrinsic coordinates
  //## (often integer indices of pixels), to be accessed by
  //## GetBoundingBox()
  //## \li a transform to convert intrinsic coordinates into a
  //## world-coordinate system with coordinates in millimeters
  //## and milliseconds (all are floating point values), to
  //## be accessed by GetIndexToWorldTransform()
  //## \li a life span, i.e. a bounding box in time in ms (with
  //## start and end time), to be accessed by GetTimeBounds().
  //## The default is minus infinity to plus infinity.
  //##
  //## Geometry3D and its sub-classes allow converting between
  //## intrinsic coordinates (called index or unit coordinates)
  //## and world-coordinates (called world or mm coordinates),
  //## e.g. WorldToIndex.
  //## In case you need integer index coordinates, provide an
  //## mitk::Index3D (or itk::Index) as target variable to
  //## WorldToIndex, otherwise you will get a continuous index
  //## (floating point values).
  //##
  //## An important sub-class is SlicedGeometry3D, which descibes
  //## data objects consisting of slices, e.g., objects of type Image.
  //## Conversions between world coordinates (in mm) and unit coordinates
  //## (e.g., pixels in the case of an Image) can be performed.
  //##
  //## For more information on related classes, see \ref Geometry.
  //##
  //## Geometry3D instances referring to an Image need a slightly
  //## different definition of corners, see SetImageGeometry. This
  //## is usualy automatically called by Image.
  //##
  //## Geometry3D have to be initialized in the method GenerateOutputInformation()
  //## of BaseProcess (or CopyInformation/ UpdateOutputInformation of BaseData,
  //## if possible, e.g., by analyzing pic tags in Image) subclasses. See also
  //## itk::ProcessObject::GenerateOutputInformation(),
  //## itk::DataObject::CopyInformation() and
  //## itk::DataObject::UpdateOutputInformation().
  //##
  //## Rule: everything is in mm (ms) if not stated otherwise.
  //## @ingroup Geometry
  class MITK_CORE_EXPORT Geometry3D : public BaseGeometry
  {
  public:
    mitkClassMacro(Geometry3D, mitk::BaseGeometry);

    typedef itk::QuaternionRigidTransform< ScalarType > QuaternionTransformType;
    typedef QuaternionTransformType::VnlQuaternionType VnlQuaternionType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);
    mitkNewMacro1Param(Self,Self);

    //##Documentation
    //## @brief clones the geometry
    //##
    //## Overwrite in all sub-classes.
    //## Normally looks like:
    //## \code
    //##  Self::Pointer newGeometry = new Self(*this);
    //##  newGeometry->UnRegister();
    //##  return newGeometry.GetPointer();
    //## \endcode
    virtual itk::LightObject::Pointer InternalClone() const;

  protected:
    Geometry3D();
    Geometry3D(const Geometry3D& other);

    virtual ~Geometry3D();

    virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

    static const std::string INDEX_TO_OBJECT_TRANSFORM;
    static const std::string OBJECT_TO_NODE_TRANSFORM;
    static const std::string INDEX_TO_NODE_TRANSFORM;
    static const std::string INDEX_TO_WORLD_TRANSFORM;
  };
} // namespace mitk

#endif /* GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
