/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef PCAExample_h
#define PCAExample_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_PCAExampleControls.h"

/**
  \brief PCAExample

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class PCAExample : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  PCAExample();

  ~PCAExample() override;

protected slots:

  /// \brief Called when the user clicks the GUI button
  void BtnPerfomPCAClicked();

protected:
  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  /** Performs a PCA on a (3D) point set.
   *  @param input Point set to work on.
   *  @param[out] eigenVectors Eigenvectors as computed by the PCA. Returns an empty vector
   *                           if PCA failed or did not run before.
   *  @param[out] eigenValues  Eigenvalues as computed by the PCA. Returns an empty vector
   *                           if PCA failed or did not run before. The ID inside the vector
   *                           corresponds to the ID inside the eigenvector vector. For a 3D
   *                           pointset 3 eigenvalues and -vectors will be computed.
   *  @param[out] pointsMean   Returns the mean/center of the pointset
   */
  bool comutePCA(mitk::PointSet::Pointer input, std::vector<mitk::Vector3D> &eigenVectors, std::vector<double> &eigenValues, mitk::Vector3D &pointsMean);

  /** Adds mitk data nodes for all eigenvectors to visualize them in the multi widget. */
  void showEigenvectors(std::vector<mitk::Vector3D> eigenVectors, std::vector<double> eigenValues, mitk::Vector3D center);

  mitk::DataNode::Pointer m_Axis1Node;
  mitk::DataNode::Pointer m_Axis2Node;
  mitk::DataNode::Pointer m_Axis3Node;

  Ui::PCAExampleControls m_Controls;
};

#endif // PCAExample_h
