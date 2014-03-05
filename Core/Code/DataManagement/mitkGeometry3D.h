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
    //void testXYZ(){ int a=1; }; //xxxxxxxxxxxxxxxx

    typedef itk::QuaternionRigidTransform< ScalarType > QuaternionTransformType;
    typedef QuaternionTransformType::VnlQuaternionType VnlQuaternionType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);
    mitkNewMacro1Param(Self,Self);

    //##Documentation
    //## @brief When switching from an Image Geometry to a normal Geometry (and the other way around), you have to change the origin as well (See Geometry Documentation)! This function will change the "isImageGeometry" bool flag and changes the origin respectively.
    virtual void ChangeImageGeometryConsideringOriginOffset( const bool isAnImageGeometry );

    //##Documentation
    //## @brief Get the position of the corner number \a id (in world coordinates)
    //##
    //## See SetImageGeometry for how a corner is defined on images.
    Point3D GetCornerPoint(int id) const;

    //##Documentation
    //## @brief Get the position of a corner (in world coordinates)
    //##
    //## See SetImageGeometry for how a corner is defined on images.
    Point3D GetCornerPoint(bool xFront=true, bool yFront=true, bool zFront=true) const;

    //##Documentation
    //## @brief Is this an ImageGeometry?
    //##
    //## For more information, see SetImageGeometry
    itkGetConstMacro(ImageGeometry, bool);
    //##Documentation
    //## @brief Define that this Geometry3D is refering to an Image
    //##
    //## A geometry referring to an Image needs a slightly different
    //## definition of the position of the corners (see GetCornerPoint).
    //## The position of a voxel is defined by the position of its center.
    //## If we would use the origin (position of the (center of) the first
    //## voxel) as a corner and display this point, it would seem to be
    //## \em not at the corner but a bit within the image. Even worse for
    //## the opposite corner of the image: here the corner would appear
    //## outside the image (by half of the voxel diameter). Thus, we have
    //## to correct for this and to be able to do that, we need to know
    //## that the Geometry3D is referring to an Image.
    itkSetMacro(ImageGeometry, bool);
    itkBooleanMacro(ImageGeometry);

    //##Documentation
    //## @brief Test whether the point \a p ((continous!)index coordinates in units) is
    //## inside the bounding box
    bool IsIndexInside(const mitk::Point3D& index) const
    {
      bool inside = false;
      //if it is an image geometry, we need to convert the index to discrete values
      //this is done by applying the rounding function also used in WorldToIndex (see line 323)
      if (m_ImageGeometry)
      {
        mitk::Point3D discretIndex;
        discretIndex[0]=itk::Math::RoundHalfIntegerUp<mitk::ScalarType>( index[0] );
        discretIndex[1]=itk::Math::RoundHalfIntegerUp<mitk::ScalarType>( index[1] );
        discretIndex[2]=itk::Math::RoundHalfIntegerUp<mitk::ScalarType>( index[2] );

        inside = m_BoundingBox->IsInside(discretIndex);
        //we have to check if the index is at the upper border of each dimension,
        // because the boundingbox is not centerbased
        if (inside)
        {
          const BoundingBox::BoundsArrayType& bounds = m_BoundingBox->GetBounds();
          if((discretIndex[0] == bounds[1]) ||
            (discretIndex[1] == bounds[3]) ||
            (discretIndex[2] == bounds[5]))
            inside = false;
        }
      }
      else
        inside = m_BoundingBox->IsInside(index);

      return inside;
    }

    //##Documentation
    //## @brief Convenience method for working with ITK indices
    template <unsigned int VIndexDimension>
    bool IsIndexInside(const itk::Index<VIndexDimension> &index) const
    {
      int i, dim=index.GetIndexDimension();
      Point3D pt_index;
      pt_index.Fill(0);
      for ( i = 0; i < dim; ++i )
      {
        pt_index[i] = index[i];
      }
      return IsIndexInside(pt_index);
    }

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

    //Umzug:

    //##Documentation
    //## @brief Get the parametric bounding-box
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    itkGetConstObjectMacro(ParametricBoundingBox, BoundingBox);

    //##Documentation
    //## @brief Get the parametric bounds
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    const BoundingBox::BoundsArrayType& GetParametricBounds() const
    {
      assert(m_ParametricBoundingBox.IsNotNull());
      return m_ParametricBoundingBox->GetBounds();
    }

    //##Documentation
    //## @brief Get the parametric extent
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    mitk::ScalarType GetParametricExtent(int direction) const
    {
      if (direction < 0 || direction>=3)
        mitkThrow() << "Invalid direction. Must be between either 0, 1 or 2. ";
      assert(m_ParametricBoundingBox.IsNotNull());

      BoundingBoxType::BoundsArrayType bounds = m_ParametricBoundingBox->GetBounds();
      return bounds[direction*2+1]-bounds[direction*2];
    }

    //##Documentation
    //## @brief Get the parametric extent in mm
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    virtual mitk::ScalarType GetParametricExtentInMM(int direction) const
    {
      return GetExtentInMM(direction);
    }

    //##Documentation
    //## @brief Get the parametric transform
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    virtual const Transform3D* GetParametricTransform() const
    {
      return m_IndexToWorldTransform;
    }

  protected:
    Geometry3D();
    Geometry3D(const Geometry3D& other);

    virtual ~Geometry3D();

    virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

    //##Documentation
    //## @brief Set the parametric bounds
    //##
    //## Protected in this class, made public in some sub-classes, e.g.,
    //## ExternAbstractTransformGeometry.
    virtual void SetParametricBounds(const BoundingBox::BoundsArrayType& bounds);

    mutable mitk::BoundingBox::Pointer m_ParametricBoundingBox;

    bool m_ImageGeometry;

    virtual void InternPostInitialize();
    virtual void InternPostInitializeGeometry(Geometry3D* newGeometry) const;

    static const std::string INDEX_TO_OBJECT_TRANSFORM;
    static const std::string OBJECT_TO_NODE_TRANSFORM;
    static const std::string INDEX_TO_NODE_TRANSFORM;
    static const std::string INDEX_TO_WORLD_TRANSFORM;
  };

  //
  // Static compare functions mainly for testing
  //
  /**
  * @brief Equal A function comparing two geometries for beeing identical.
  *
  * @ingroup MITKTestingAPI
  *
  * The function compares the spacing, origin, axisvectors, extents, the matrix of the
  * IndexToWorldTransform (elementwise), the bounding (elementwise) and the ImageGeometry flag.
  *
  * The parameter eps is a tolarence value for all methods which are internally used for comparion.
  * If you want to use different tolarance values for different parts of the geometry, feel free to use
  * the other comparison methods and write your own implementation of Equal.
  * @param rightHandSide Compare this against leftHandSide.
  * @param leftHandSide Compare this against rightHandSide.
  * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  * @return True, if all comparison are true. False in any other case.
  */
  MITK_CORE_EXPORT bool Equal(const mitk::Geometry3D* leftHandSide, const mitk::Geometry3D* rightHandSide, ScalarType eps, bool verbose);
} // namespace mitk

#endif /* GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
