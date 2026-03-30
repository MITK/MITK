/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkVtkAbstractTransform_h
#define itkVtkAbstractTransform_h

#include <itkTransform.h>
#include <MitkCoreExports.h>

class vtkAbstractTransform;

namespace itk
{
  /** \brief Adapter from vtkAbstractTransform to itk::Transform<TScalarType, 3, 3>.
   *
   * Wraps a VTK abstract transform so it can be used in the ITK transform framework.
   * Supports forward and inverse point, vector, and covariant vector transformation.
   *
   * \ingroup Geometry
   */
  template <class TScalarType>
  class VtkAbstractTransform : public itk::Transform<TScalarType, 3, 3>
  {
  public:
    typedef VtkAbstractTransform Self;
    typedef Transform<TScalarType, 3, 3> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;
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
    typedef typename Superclass::JacobianPositionType JacobianPositionType;

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Get the vtkAbstractTransform (stored in m_VtkAbstractTransform). */
      virtual vtkAbstractTransform *GetVtkAbstractTransform() const;

    /** \brief Get the inverse vtkAbstractTransform (stored in m_InverseVtkAbstractTransform). */
    virtual vtkAbstractTransform *GetInverseVtkAbstractTransform() const;

    /** \brief Set the vtkAbstractTransform (stored in m_VtkAbstractTransform).
     *
     * Also computes and caches the inverse transform. Registers the new transform
     * and unregisters the old one.
     *
     * \param aVtkAbstractTransform The VTK transform to wrap.
     */
    virtual void SetVtkAbstractTransform(vtkAbstractTransform *aVtkAbstractTransform);

    using Superclass::TransformVector;
    using Superclass::TransformCovariantVector;

    /** \brief Transform a point from input space to output space using the VTK transform. */
    OutputPointType TransformPoint(const InputPointType &) const override;

    /** \brief Transform a vector from input space to output space using the VTK transform. */
    OutputVectorType TransformVector(const InputVectorType &) const override;

    /** \brief Transform a vnl vector from input space to output space using the VTK transform. */
    OutputVnlVectorType TransformVector(const InputVnlVectorType &) const override;

    /** \brief Transform a covariant vector (not yet implemented, will throw). */
    OutputCovariantVectorType TransformCovariantVector(const InputCovariantVectorType &) const override;

    /** \brief Back-transform a point from output space to input space using the inverse VTK transform.
     * \param point The point in output space.
     * \return The point in input space.
     */
    virtual InputPointType BackTransform(const OutputPointType &point) const;

    /** \brief Back-transform a vector from output space to input space using the inverse VTK transform.
     * \param vector The vector in output space.
     * \return The vector in input space.
     */
    virtual InputVectorType BackTransform(const OutputVectorType &vector) const;

    /** \brief Back-transform a vnl vector from output space to input space using the inverse VTK transform.
     * \param vector The vnl vector in output space.
     * \return The vnl vector in input space.
     */
    virtual InputVnlVectorType BackTransform(const OutputVnlVectorType &vector) const;

    /** \brief Back-transform a covariant vector (not yet implemented, will throw).
     * \param vector The covariant vector in output space.
     * \return The covariant vector in input space.
     */
    virtual InputCovariantVectorType BackTransform(const OutputCovariantVectorType &vector) const;

    /** \brief Set the transform parameters (not yet implemented). */
    void SetParameters(const ParametersType &) override;

    /** \brief Set the fixed transform parameters (not yet implemented). */
    void SetFixedParameters(const ParametersType &) override;

    /** \brief Compute the Jacobian with respect to parameters (not yet implemented). */
    void ComputeJacobianWithRespectToParameters(const InputPointType &, JacobianType &) const override;

    /** \brief Compute the Jacobian with respect to position (not yet implemented). */
    void ComputeJacobianWithRespectToPosition(const InputPointType &, JacobianPositionType &) const override;

    /** \brief Get the modification time, taking VTK transform changes into account. */
    itk::ModifiedTimeType GetMTime() const override;

  protected:
    /** \brief Default constructor. Initializes transforms to nullptr. */
    VtkAbstractTransform();

    /** \brief Destructor. Unregisters the owned VTK transform. */
    ~VtkAbstractTransform() override;

    /** \brief The forward vtkAbstractTransform instance. */
    vtkAbstractTransform *m_VtkAbstractTransform;

    /** \brief The inverse vtkAbstractTransform instance (managed by m_VtkAbstractTransform). */
    vtkAbstractTransform *m_InverseVtkAbstractTransform;

    mutable unsigned long m_LastVtkAbstractTransformTimeStamp;
  };

} // namespace itk

#ifndef MITK_MANUAL_INSTANTIATION
#include <itkVtkAbstractTransform.tpp>
#endif

#endif /* itkVtkAbstractTransform_h */
