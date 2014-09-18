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

#ifndef MITKTHINPLATESPLINECURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define MITKTHINPLATESPLINECURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkLandmarkProjectorBasedCurvedGeometry.h"

class vtkPoints;
class vtkThinPlateSplineTransform;

namespace mitk {
  //##Documentation
  //## @brief Thin-plate-spline-based landmark-based curved geometry
  //##
  //## @ingroup Geometry
  class MITK_CORE_EXPORT ThinPlateSplineCurvedGeometry : public LandmarkProjectorBasedCurvedGeometry
  {
  public:
    mitkClassMacro(ThinPlateSplineCurvedGeometry, LandmarkProjectorBasedCurvedGeometry);

    itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      virtual void ComputeGeometry();

    virtual itk::LightObject::Pointer InternalClone() const;

    vtkThinPlateSplineTransform* GetThinPlateSplineTransform() const
    {
      return m_ThinPlateSplineTransform;
    }

    virtual void SetSigma(double sigma);
    virtual double GetSigma() const;

    virtual bool IsValid() const;

  protected:
    ThinPlateSplineCurvedGeometry();
    ThinPlateSplineCurvedGeometry(const ThinPlateSplineCurvedGeometry& other );

    virtual ~ThinPlateSplineCurvedGeometry();

    vtkThinPlateSplineTransform* m_ThinPlateSplineTransform;

    vtkPoints* m_VtkTargetLandmarks;
    vtkPoints* m_VtkProjectedLandmarks;

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

#endif /* MITKTHINPLATESPLINECURVEDGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
