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
#include "mapRegistration.h"
#include "mapContinuous.h"

#include <mitkPoint.h>
#include <mitkVector.h>

#include <QWidget>

#include <MitkMatchPointRegistrationUIExports.h>
#include <memory>

namespace Ui
{
  class QmitkRegistrationManipulationWidget;
}

/*!
\brief Widget that allows to manually manipulate a registration transform.

The widget operates on an itk::Euler3DTransform and provides slider/spinbox controls
for rotation and translation. It also offers methods to apply incremental changes
(e.g. from mouse interaction in render windows).

\ingroup MatchPointRegistrationUI
*/
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkRegistrationManipulationWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkRegistrationManipulationWidget(QWidget *parent = nullptr);

  ~QmitkRegistrationManipulationWidget() override;

  /** Type of transform that can be provided as preceding transform.*/
  typedef map::core::Registration<3, 3> MAPRegistrationType;

  /** Initializing/reset widget with identity transform.*/
  void Initialize();

  /** Initializing/reset widget with a preceding registration.*/
  void Initialize(MAPRegistrationType* precedingRegistration);

  /** Initializing/reset widget with an translation transform deduced by the to passed reference points.*/
  void Initialize(const mitk::Point3D& movingReference, const mitk::Point3D& targetReference);

  /**This function offers access to a registration instance that represents the internal state of the
  registration currently manipulated by the widget. It can be used for example to update the visualization.*/
  map::core::RegistrationBase* GetInterimRegistration() const;

  /**This function generates a new registration instance that resembles the state when the method was called.
  Ownership of the return goes to the caller.*/
  map::core::RegistrationBase::Pointer GenerateRegistration()const ;

  /** Apply an incremental translation in world coordinates to the current transform.
  * @pre delta must be a valid 3D vector (no NaN/Inf components).
  */
  void ApplyTranslationDelta(const mitk::Vector3D& delta);

  /** Apply an incremental rotation around the given world-space axis.
  * The rotation is composed with the current rotation matrix and decomposed
  * back into Euler angles.
  * @pre axis must be a non-zero vector. It will be normalized internally.
  * @param axis  Rotation axis in world coordinates (e.g. view plane normal).
  * @param angleDeg  Rotation angle in degrees.
  */
  void ApplyRotationDelta(const mitk::Vector3D& axis, double angleDeg);

public Q_SLOTS:
  void SetCenterOfRotation(const mitk::Point3D& center);

  /** Sets the internal m_CenterOfRotationIsRelativeToTarget. see below.*/
  void SetCenterOfRotationIsRelativeToTarget(bool targetRelative);

signals:
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
