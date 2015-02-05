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

#include "itkVtkAbstractTransform.h"
#include <vtkAbstractTransform.h>
#include <mitkNumericTypes.h>

namespace itk {

template <class TScalarType>
itk::VtkAbstractTransform<TScalarType>::VtkAbstractTransform() :
  m_VtkAbstractTransform(NULL), m_InverseVtkAbstractTransform(NULL),
  m_LastVtkAbstractTransformTimeStamp(0)
{

}

template <class TScalarType>
itk::VtkAbstractTransform<TScalarType>::~VtkAbstractTransform()
{
  if(m_VtkAbstractTransform!=NULL)
    m_VtkAbstractTransform->UnRegister(NULL);
}

template <class TScalarType>
vtkAbstractTransform* itk::VtkAbstractTransform<TScalarType>::GetVtkAbstractTransform() const
{
  return m_VtkAbstractTransform;
}

template <class TScalarType>
vtkAbstractTransform* itk::VtkAbstractTransform<TScalarType>::GetInverseVtkAbstractTransform() const
{
  return m_InverseVtkAbstractTransform;
}

template <class TScalarType>
void itk::VtkAbstractTransform<TScalarType>::SetVtkAbstractTransform(vtkAbstractTransform* aVtkAbstractTransform)
{
  if(m_VtkAbstractTransform==aVtkAbstractTransform)
    return;

  if(m_VtkAbstractTransform!=NULL)
    m_VtkAbstractTransform->UnRegister(NULL);

  m_VtkAbstractTransform=aVtkAbstractTransform;
  if(m_VtkAbstractTransform!=NULL)
  {
    m_VtkAbstractTransform->Register(NULL);
    m_InverseVtkAbstractTransform=m_VtkAbstractTransform->GetInverse(); // memory managed by m_VtkAbstractTransform
  }

  m_LastVtkAbstractTransformTimeStamp = m_VtkAbstractTransform->GetMTime();

  this->Modified();
}

// Transform a point
template<class TScalarType>
typename itk::VtkAbstractTransform<TScalarType>::OutputPointType
itk::VtkAbstractTransform<TScalarType>::
TransformPoint(const InputPointType &point) const
{
  assert(m_VtkAbstractTransform!=NULL);

  OutputPointType outputpoint;
  vnl_vector<TScalarType> vnl_vec;
  mitk::ScalarType vtkpt[3];
  mitk::itk2vtk(point, vtkpt);
  m_VtkAbstractTransform->TransformPoint(vtkpt, vtkpt);
  mitk::vtk2itk(vtkpt, outputpoint);
  return outputpoint;
}


// Transform a vector
template<class TScalarType>
typename itk::VtkAbstractTransform<TScalarType>::OutputVectorType
itk::VtkAbstractTransform<TScalarType>::
TransformVector(const InputVectorType &vect) const
{
  assert(m_VtkAbstractTransform!=NULL);

  OutputVectorType outputvector;
  vnl_vector<TScalarType> vnl_vec;
  mitk::ScalarType vtkpt[3]={0,0,0};
  mitk::ScalarType vtkvec[3];
  mitk::vnl2vtk<TScalarType, mitk::ScalarType>(vect.GetVnlVector(), vtkvec);
  m_VtkAbstractTransform->TransformVectorAtPoint(vtkpt, vtkvec, vtkvec);
  mitk::vtk2itk(vtkvec, outputvector);
  return outputvector;
}


// Transform a vnl_vector_fixed
template<class TScalarType>
typename itk::VtkAbstractTransform<TScalarType>::OutputVnlVectorType
itk::VtkAbstractTransform<TScalarType>::
TransformVector(const InputVnlVectorType &vect) const
{
  assert(m_VtkAbstractTransform!=NULL);

  OutputVnlVectorType outputvector;
  mitk::ScalarType vtkpt[3]={0,0,0};
  mitk::ScalarType vtkvec[3];
  mitk::vnl2vtk<TScalarType, mitk::ScalarType>(vect, vtkvec);
  m_VtkAbstractTransform->TransformVectorAtPoint(vtkpt, vtkvec, vtkvec);
  mitk::vtk2itk(vtkvec, outputvector);
  return outputvector;
}

// Transform a CovariantVector
template<class TScalarType>
typename itk::VtkAbstractTransform<TScalarType>::OutputCovariantVectorType
itk::VtkAbstractTransform<TScalarType>::
TransformCovariantVector(const InputCovariantVectorType &/*vec*/) const
{
  itkExceptionMacro( << "implement before using!" );
  OutputCovariantVectorType  result;    // Converted vector

//  for (unsigned int i = 0; i < NDimensions; i++)
//    {
//    result[i] = NumericTraits<mitk::ScalarType>::Zero;
//    for (unsigned int j = 0; j < NDimensions; j++)
//      {
//      result[i] += m_Inverse[j][i]*vec[j]; // Inverse transposed
//      }
//    }
  return result;
}

// Back transform a point
template<class TScalarType>
typename VtkAbstractTransform<TScalarType>::InputPointType
itk::VtkAbstractTransform<TScalarType>::
BackTransform(const OutputPointType &point) const
{
  assert(m_VtkAbstractTransform!=NULL);

  OutputPointType outputpoint;
  mitk::ScalarType vtkpt[3];
  mitk::itk2vtk(point, vtkpt);
  m_InverseVtkAbstractTransform->TransformPoint(vtkpt, vtkpt);
  mitk::vtk2itk(vtkpt, outputpoint);
  return outputpoint;
}

// Back transform a vector
template<class TScalarType>
typename VtkAbstractTransform<TScalarType>::InputVectorType
itk::VtkAbstractTransform<TScalarType>::
BackTransform(const OutputVectorType &vect ) const
{
  assert(m_VtkAbstractTransform!=NULL);

  OutputVectorType outputvector;
  mitk::ScalarType vtkpt[3]={0,0,0};
  mitk::ScalarType vtkvec[3];
  mitk::itk2vtk(vect, vtkvec);
  m_InverseVtkAbstractTransform->TransformVectorAtPoint(vtkpt, vtkvec, vtkvec);
  mitk::vtk2itk(vtkvec, outputvector);
  return outputvector;
}

// Back transform a vnl_vector
template<class TScalarType>
typename VtkAbstractTransform<TScalarType>::InputVnlVectorType
itk::VtkAbstractTransform<TScalarType>::
BackTransform(const OutputVnlVectorType &vect ) const
{
  assert(m_InverseVtkAbstractTransform!=NULL);

  OutputVnlVectorType outputvector;
  mitk::ScalarType vtkpt[3]={0,0,0};
  mitk::ScalarType vtkvec[3];
  mitk::itk2vtk(vect, vtkvec);
  m_InverseVtkAbstractTransform->TransformVectorAtPoint(vtkpt, vtkvec, vtkvec);
  mitk::vtk2itk(vtkvec, outputvector);
  return outputvector;
}

// Back Transform a CovariantVector
template<class TScalarType>
typename VtkAbstractTransform<TScalarType>::InputCovariantVectorType
itk::VtkAbstractTransform<TScalarType>::
BackTransform(const OutputCovariantVectorType &vec) const
{
  itkExceptionMacro( << "implement before using!" );
//  for (unsigned int i = 0; i < NDimensions; i++)
//    {
//    result[i] = NumericTraits<mitk::ScalarType>::Zero;
//    for (unsigned int j = 0; j < NDimensions; j++)
//      {
//      result[i] += m_Matrix[j][i]*vec[j]; // Direct matrix transposed
//      }
//    }
  return vec;
}

template<class TScalarType>
unsigned long
itk::VtkAbstractTransform<TScalarType>::GetMTime() const
{
  if((m_VtkAbstractTransform != NULL) && (m_LastVtkAbstractTransformTimeStamp < m_VtkAbstractTransform->GetMTime()))
  {
    m_LastVtkAbstractTransformTimeStamp=m_VtkAbstractTransform->GetMTime();
    this->Modified();
  }

  return Superclass::GetMTime();
}

template <class TScalarType>
void itk::VtkAbstractTransform<TScalarType>::SetParameters(const ParametersType&)
{
  // TODO
}

template <class TScalarType>
void itk::VtkAbstractTransform<TScalarType>::SetFixedParameters(const ParametersType&)
{
  // TODO
}

template <class TScalarType>
void itk::VtkAbstractTransform<TScalarType>::ComputeJacobianWithRespectToParameters(const InputPointType&, JacobianType&) const
{
  // TODO
}

template <class TScalarType>
void itk::VtkAbstractTransform<TScalarType>::ComputeJacobianWithRespectToPosition(const InputPointType&, JacobianType&) const
{
  // TODO
}

} // namespace itk
