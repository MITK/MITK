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

#ifndef MITKLANDMARKPROJECTORBASEDCURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define MITKLANDMARKPROJECTORBASEDCURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkLandmarkProjector.h"

#include "mitkAbstractTransformGeometry.h"
#include "mitkPointSet.h"

namespace mitk {
  //##Documentation
  //## @brief Superclass of AbstractTransformGeometry sub-classes defined
  //## by a set of landmarks.
  //##
  //## @ingroup Geometry
  class MITK_CORE_EXPORT LandmarkProjectorBasedCurvedGeometry : public AbstractTransformGeometry
  {
  public:
    mitkClassMacro(LandmarkProjectorBasedCurvedGeometry, AbstractTransformGeometry);

    void SetLandmarkProjector(mitk::LandmarkProjector* aLandmarkProjector);
    itkGetConstObjectMacro(LandmarkProjector, mitk::LandmarkProjector);

    virtual void SetFrameGeometry(const mitk::BaseGeometry* frameGeometry);

    virtual void ComputeGeometry();

    itkGetConstMacro(InterpolatingAbstractTransform, vtkAbstractTransform*);

    itk::LightObject::Pointer InternalClone() const;

    //##Documentation
    //## @brief Set the landmarks through which the geometry shall pass
    itkSetConstObjectMacro(TargetLandmarks, mitk::PointSet::DataType::PointsContainer);
    //##Documentation
    //## @brief Get the landmarks through which the geometry shall pass
    itkGetConstObjectMacro(TargetLandmarks, mitk::PointSet::DataType::PointsContainer);

  protected:
    LandmarkProjectorBasedCurvedGeometry();

    LandmarkProjectorBasedCurvedGeometry(const LandmarkProjectorBasedCurvedGeometry& other);

    virtual ~LandmarkProjectorBasedCurvedGeometry();

    mitk::LandmarkProjector::Pointer m_LandmarkProjector;

    vtkAbstractTransform* m_InterpolatingAbstractTransform;

    mitk::PointSet::DataType::PointsContainer::ConstPointer m_TargetLandmarks;

    //##Documentation
    //## @brief Pre- and Post-functions are empty in BaseGeometry
    //##
    //## These virtual functions allow for a different beahiour in subclasses.
    //## Do implement them in every subclass of BaseGeometry. If not needed, use {}.
    //## If this class is inherited from a subclass of BaseGeometry, call {Superclass::Pre...();};, example: DisplayGeometry class
    virtual void PostInitialize(){Superclass::PostInitialize();};
    virtual void PostInitializeGeometry(mitk::BaseGeometry::Self * newGeometry) const{Superclass::PostInitializeGeometry(newGeometry);};
    virtual void PreSetSpacing(const mitk::Vector3D& aSpacing){Superclass::PreSetSpacing(aSpacing);};
    virtual void PreSetBounds( const BoundingBox::BoundsArrayType &bounds ){Superclass::PreSetBounds(bounds);};
    virtual void PreSetIndexToWorldTransform( AffineTransform3D *transform){Superclass::PreSetIndexToWorldTransform(transform);};
    virtual void PostSetExtentInMM(int direction, ScalarType extentInMM){Superclass::PostSetExtentInMM(direction,extentInMM);};
    virtual void PostSetIndexToWorldTransform(mitk::AffineTransform3D* transform){Superclass::PostSetIndexToWorldTransform(transform);};
  };
} // namespace mitk

#endif /* MITKLANDMARKPROJECTORBASEDCURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
