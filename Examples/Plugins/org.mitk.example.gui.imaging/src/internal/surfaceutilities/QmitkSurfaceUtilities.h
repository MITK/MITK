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

#if !defined(QMITK_SurfaceUtilities_H__INCLUDED)
#define QMITK_SurfaceUtilities_H__INCLUDED

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
    ~QmitkSurfaceUtilities();

    virtual void CreateQtPartControl(QWidget *parent) override;

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


    //methods to read in / write from / to GUI
    itk::Matrix<double,3,3> ReadInFromGUI_RotationMatrix();
    itk::Vector<double,3> ReadInFromGUI_TranslationVector();
    void WriteToGUI_RotationMatrix(itk::Matrix<double,3,3> r);
    void WriteToGUI_TranslationVector(itk::Vector<double,3> t);

    /** @brief Converts euler angles (in degrees!) to a rotation matrix. */
    itk::Matrix<double,3,3> ConvertEulerAnglesToRotationMatrix(double alpha, double beta, double gamma);


    Ui::QmitkSurfaceUtilitiesControls m_Controls;

};
#endif // !defined(QMITK_ISOSURFACE_H__INCLUDED)
