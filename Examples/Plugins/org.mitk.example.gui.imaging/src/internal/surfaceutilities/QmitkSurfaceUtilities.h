/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSurfaceUtilities_h
#define QmitkSurfaceUtilities_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkSurfaceUtilitiesControls.h"

/*!
  \brief TODO
  */
class QmitkSurfaceUtilities : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkSurfaceUtilities();
  ~QmitkSurfaceUtilities() override;

  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

protected slots:

  void OnEulerToMatrixClicked();
  void OnMoveToOriginClicked();
  void OnApplyTransformClicked();
  void OnComputeCoG();
  void OnGenerateTargetPoints();
  void OnPerturbeSurface();
  void OnAddOutliers();

protected:
  // methods to read in / write from / to GUI
  itk::Matrix<double, 3, 3> ReadInFromGUI_RotationMatrix();
  itk::Vector<double, 3> ReadInFromGUI_TranslationVector();
  void WriteToGUI_RotationMatrix(itk::Matrix<double, 3, 3> r);
  void WriteToGUI_TranslationVector(itk::Vector<double, 3> t);

  /** @brief Converts euler angles (in degrees!) to a rotation matrix. */
  itk::Matrix<double, 3, 3> ConvertEulerAnglesToRotationMatrix(double alpha, double beta, double gamma);

  Ui::QmitkSurfaceUtilitiesControls m_Controls;
};
#endif
