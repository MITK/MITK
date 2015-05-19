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

#ifndef MITKVTKABSTRACTTRANSFORM_H_HEADER_INCLUDED_C1C68A2C
#define MITKVTKABSTRACTTRANSFORM_H_HEADER_INCLUDED_C1C68A2C

#include <MitkCoreExports.h>
#include "itkTransform.h"

class vtkAbstractTransform;

namespace itk {

//##Documentation
//## @brief Adapter from vtkAbstractTransform to itk::Transform<TScalarType, 3, 3>
//## @ingroup Geometry
template <class TScalarType>
class VtkAbstractTransform : public itk::Transform<TScalarType, 3, 3>
{
public:
  typedef VtkAbstractTransform  Self;
  typedef Transform< TScalarType, 3, 3 >  Superclass;
  typedef SmartPointer<Self>        Pointer;
  typedef SmartPointer<const Self>  ConstPointer;
  typedef typename Superclass::OutputPointType OutputPointType;
  typedef typename Superclass::OutputVectorType OutputVectorType;
  typedef typename Superclass::OutputVnlVectorType OutputVnlVectorType;
  typedef typename Superclass::OutputCovariantVectorType OutputCovariantVectorType;
  typedef typename Superclass::InputPointType InputPointType;
  typedef typename Superclass::InputVectorType InputVectorType;
  typedef typename Superclass::InputVnlVectorType InputVnlVectorType;
  typedef typename Superclass::InputCovariantVectorType InputCovariantVectorType;
  typedef typename Superclass::ParametersType ParametersType;
  typedef typename Superclass::JacobianType JacobianType;


  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  //##Documentation
  //## @brief Get the vtkAbstractTransform (stored in m_VtkAbstractTransform)
  virtual vtkAbstractTransform* GetVtkAbstractTransform() const;
  //##Documentation
  //## @brief Get the inverse vtkAbstractTransform (stored in m_InverseVtkAbstractTransform)
  virtual vtkAbstractTransform* GetInverseVtkAbstractTransform() const;

  //##Documentation
  //## @brief Set the vtkAbstractTransform (stored in m_VtkAbstractTransform)
  virtual void SetVtkAbstractTransform(vtkAbstractTransform* aVtkAbstractTransform);

  using Superclass::TransformVector;
  using Superclass::TransformCovariantVector;

  virtual OutputPointType TransformPoint(const InputPointType  & ) const override;
  virtual OutputVectorType TransformVector(const InputVectorType &) const override;
  virtual OutputVnlVectorType TransformVector(const InputVnlVectorType &) const override;
  virtual OutputCovariantVectorType TransformCovariantVector(const InputCovariantVectorType &) const override;

  virtual InputPointType BackTransform(const OutputPointType  &point ) const;
  virtual InputVectorType BackTransform(const OutputVectorType &vector) const;
  virtual InputVnlVectorType BackTransform(const OutputVnlVectorType &vector) const;
  virtual InputCovariantVectorType BackTransform(const OutputCovariantVectorType &vector) const;

  virtual void SetParameters(const ParametersType&) override;
  virtual void SetFixedParameters(const ParametersType&) override;
  virtual void ComputeJacobianWithRespectToParameters(const InputPointType&, JacobianType&) const override;
  virtual void ComputeJacobianWithRespectToPosition(const InputPointType&, JacobianType&) const override;

  virtual unsigned long GetMTime() const override;

protected:
  VtkAbstractTransform();
  virtual ~VtkAbstractTransform();

  //##Documentation
  //## @brief Instance of the vtkAbstractTransform
  vtkAbstractTransform* m_VtkAbstractTransform;

  //##Documentation
  //## @brief Instance of the vtkAbstractTransform
  vtkAbstractTransform* m_InverseVtkAbstractTransform;

  mutable unsigned long m_LastVtkAbstractTransformTimeStamp;
};

} // namespace itk

#ifndef MITK_MANUAL_INSTANTIATION
#include "itkVtkAbstractTransform.txx"
#endif

#endif /* MITKVTKABSTRACTTRANSFORM_H_HEADER_INCLUDED_C1C68A2C */
