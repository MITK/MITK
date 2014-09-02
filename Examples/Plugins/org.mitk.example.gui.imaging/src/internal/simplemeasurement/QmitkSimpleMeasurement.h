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

#ifndef MITKSIMPELEMEASUREMENT_H
#define MITKSIMPELEMEASUREMENT_H

#include <QmitkAbstractView.h>
#include <mitkIZombieViewPart.h>

#include "mitkDataNode.h"
#include "mitkPointSetDataInteractor.h"
#include "ui_QmitkSimpleMeasurementControls.h"

namespace Ui {
class QmitkSimpleMeasurementControls;
}

/**
 * \brief SimpleMeasurement
 * Allows to measure distances, angles, etc.
 *
 * \sa QmitkAbstractView
 */
class QmitkSimpleMeasurement : public QmitkAbstractView, public mitk::IZombieViewPart
{
  Q_OBJECT

public:
  QmitkSimpleMeasurement();
  virtual ~QmitkSimpleMeasurement();

private:

  virtual void CreateQtPartControl(QWidget* parent);

  virtual void SetFocus();

  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes);

  virtual void NodeRemoved(const mitk::DataNode* node);

  virtual void Activated();
  virtual void Deactivated();
  virtual void Visible();
  virtual void Hidden();

  virtual void ActivatedZombieView(berry::SmartPointer<berry::IWorkbenchPartReference> zombieView);

private slots:

  void AddDistanceSimpleMeasurement();
  void AddAngleSimpleMeasurement();
  void AddPathSimpleMeasurement();

  void Finished();

private:

  /**
   * controls containing sliders for scrolling through the slices
   */
  Ui::QmitkSimpleMeasurementControls * m_Controls;

  /*
   * Interactor for performing the simplemeasurements.
   */
  mitk::PointSetDataInteractor::Pointer m_PointSetInteractor;

  /*
   * Interactor for performing the simplemeasurements.
   */
  mitk::DataNode::Pointer m_SelectedPointSetNode;

  /** @brief
   *  Node representing the PointSets which were created by this application.
   */
  std::vector<mitk::DataNode::Pointer> m_CreatedDistances;
  std::vector<mitk::DataNode::Pointer> m_CreatedAngles;
  std::vector<mitk::DataNode::Pointer> m_CreatedPaths;

  void StartEditingMeasurement();
  void EndEditingMeasurement();
  void UpdateMeasurementList();

};

#endif // QMITK_MEASUREMENT_H__INCLUDED
