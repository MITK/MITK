/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkRegistrationManipulationWidget_h
#define QmitkRegistrationManipulationWidget_h


#include <itkEuler3DTransform.h>
#include <mapRegistration.h>
#include <mapContinuous.h>

#include <mitkPoint.h>

#include <QWidget>

#include <MitkMatchPointRegistrationUIExports.h>
#include <memory>

namespace Ui
{
  class QmitkRegistrationManipulationWidget;
}

/**
 * \class QmitkRegistrationManipulationWidget
 * \brief Widget for interactively manipulating a 3D rigid registration (rotation and translation).
 *
 * This widget provides sliders and spin boxes for adjusting the rotation (Euler angles
 * around X, Y, Z in degrees) and translation (X, Y, Z in mm) of a 3D rigid transform
 * based on itk::Euler3DTransform. The manipulated registration can optionally be composed
 * with a preceding (baseline) registration.
 *
 * The widget maintains both direct and inverse transforms internally and emits
 * RegistrationChanged whenever the user modifies any parameter. The interim registration
 * can be queried at any time via GetInterimRegistration(), while GenerateRegistration()
 * creates a finalized independent copy (optionally composed with the preceding registration).
 *
 * The center of rotation can be configured to be relative to the target space (in which case
 * it is automatically updated through the inverse transform) or relative to the moving space.
 *
 * \sa QmitkRegistrationJob, mitk::MAPRegistrationWrapper
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkRegistrationManipulationWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the registration manipulation widget.
   * \param[in] parent Optional parent widget.
   */
  QmitkRegistrationManipulationWidget(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkRegistrationManipulationWidget() override;

  /** \brief Type alias for a 3D-to-3D MatchPoint registration. */
  typedef map::core::Registration<3, 3> MAPRegistrationType;

  /**
   * \brief Initializes the widget with an identity transform.
   *
   * Resets all rotation and translation parameters to zero and clears
   * any preceding registration.
   */
  void Initialize();

  /**
   * \brief Initializes the widget with a preceding (baseline) registration.
   *
   * Resets the current manipulation transform to identity and composes it with
   * the given preceding registration. The final registration produced by
   * GenerateRegistration() will combine both.
   *
   * \param[in] precedingRegistration Pointer to the baseline registration to build upon.
   * \pre \p precedingRegistration must not be \c nullptr.
   */
  void Initialize(MAPRegistrationType* precedingRegistration);

  /**
   * \brief Initializes the widget with a translation derived from two reference points.
   *
   * Computes the translation offset as \p targetReference - \p movingReference and
   * sets this as the initial transform. No preceding registration is used.
   *
   * \param[in] movingReference A reference point in the moving image space.
   * \param[in] targetReference The corresponding reference point in the target image space.
   */
  void Initialize(const mitk::Point3D& movingReference, const mitk::Point3D& targetReference);

  /**
   * \brief Returns the current interim registration reflecting the widget's transform state.
   *
   * The returned registration is the internal working registration and is updated
   * live as the user manipulates the controls. It can be used for real-time
   * visualization updates.
   *
   * \return Pointer to the current interim registration. Ownership remains with the widget.
   */
  map::core::RegistrationBase* GetInterimRegistration() const;

  /**
   * \brief Generates a new independent registration representing the current manipulation state.
   *
   * Creates a new registration with both direct and inverse mapping kernels. If a preceding
   * registration was set via Initialize(), the result is the composition of the preceding
   * registration and the current manipulation transform.
   *
   * \return Smart pointer to the newly created registration. Ownership is transferred to the caller.
   */
  map::core::RegistrationBase::Pointer GenerateRegistration()const ;

public Q_SLOTS:
  /**
   * \brief Sets the center of rotation for the Euler transform.
   *
   * The center point is interpreted in either target or moving space depending on
   * the value of SetCenterOfRotationIsRelativeToTarget().
   *
   * \param[in] center The 3D center of rotation point.
   */
  void SetCenterOfRotation(const mitk::Point3D& center);

  /**
   * \brief Controls whether the center of rotation is specified relative to the target space.
   *
   * If \p targetRelative is true, the center of rotation is given in target space coordinates
   * and will be mapped through the inverse transform to obtain the moving-space center.
   * If false, the center is used directly in moving space.
   *
   * \param[in] targetRelative True for target-relative center, false for moving-relative center.
   */
  void SetCenterOfRotationIsRelativeToTarget(bool targetRelative);

signals:
  /**
   * \brief Emitted whenever the user modifies the registration transform via the widget controls.
   * \param[in] registration Pointer to the updated interim registration.
   */
  void RegistrationChanged(map::core::RegistrationBase *registration);

protected slots:

  void OnRotXChanged(double);
  void OnRotYChanged(double);
  void OnRotZChanged(double);

  void OnTransXChanged(double);
  void OnTransYChanged(double);
  void OnTransZChanged(double);

  void OnRotXSlideChanged(int);
  void OnRotYSlideChanged(int);
  void OnRotZSlideChanged(int);

  void OnTransXSlideChanged(int);
  void OnTransYSlideChanged(int);
  void OnTransZSlideChanged(int);

private:

  /** Initializes/resets the transforms.*/
  void ResetTransforms();
  /** Initialize the sub widgets according to the internal state of the transforms.*/
  void InitControls();

  /**
  * Updates the widgets that manipulate the transform according to the transform.*/
  void UpdateTransformWidgets();

  /**
  * Updates the transform according to the widgets that manipulate the transform.*/
  void UpdateTransform(bool updateRotation = false);

  void ConfigureTransformCenter();

  std::unique_ptr<Ui::QmitkRegistrationManipulationWidget> m_Controls;

  MAPRegistrationType::Pointer m_PreRegistration;

  using TransformType = itk::Euler3DTransform<::map::core::continuous::ScalarType>;
  TransformType::Pointer m_InverseCurrentTransform;
  TransformType::Pointer m_DirectCurrentTransform;

  MAPRegistrationType::Pointer m_CurrentRegistration;
  mitk::Point3D m_CenterOfRotation;
  /** This flag indicates if the center of rotation is specified relative to the target
  (true; thus must be updated when the inverse transform is changed) or relative to the moving
  (false; thus must not be updated).*/
  bool m_CenterOfRotationIsRelativeToTarget;

  bool m_internalUpdate;
};

#endif
