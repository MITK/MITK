/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKVTKABSTRACTTRANSFORM_H_HEADER_INCLUDED_C1C68A2C
#define MITKVTKABSTRACTTRANSFORM_H_HEADER_INCLUDED_C1C68A2C

#include "mitkCommon.h"
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


  itkNewMacro(Self);
  
  //##Documentation
  //## @brief Get the vtkAbstractTransform (stored in m_VtkAbstractTransform)
  virtual vtkAbstractTransform* GetVtkAbstractTransform() const;
  //##Documentation
  //## @brief Get the inverse vtkAbstractTransform (stored in m_InverseVtkAbstractTransform)
  virtual vtkAbstractTransform* GetInverseVtkAbstractTransform() const;
  
  //##Documentation
  //## @brief Set the vtkAbstractTransform (stored in m_VtkAbstractTransform)
  virtual void SetVtkAbstractTransform(vtkAbstractTransform* aVtkAbstractTransform);

  virtual OutputPointType TransformPoint(const InputPointType  & ) const;
  virtual OutputVectorType    TransformVector(const InputVectorType &) const;
  virtual OutputVnlVectorType TransformVector(const InputVnlVectorType &) const;
  virtual OutputCovariantVectorType TransformCovariantVector(const InputCovariantVectorType &) const;

  virtual InputPointType   BackTransform(const OutputPointType  &point ) const;
  virtual InputVectorType  BackTransform(const OutputVectorType &vector) const;
  virtual InputVnlVectorType BackTransform(const OutputVnlVectorType &vector) const;
  virtual InputCovariantVectorType BackTransform(const OutputCovariantVectorType &vector) const;

  virtual unsigned long GetMTime() const;

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
