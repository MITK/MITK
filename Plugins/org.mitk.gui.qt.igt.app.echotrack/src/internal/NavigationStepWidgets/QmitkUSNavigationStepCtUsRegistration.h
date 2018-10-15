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

#ifndef QMITKUSNAVIGATIONSTEPCTUSREGISTRATION_H
#define QMITKUSNAVIGATIONSTEPCTUSREGISTRATION_H

#include "QmitkUSAbstractNavigationStep.h"

namespace itk {
template<class T> class SmartPointer;
}

namespace mitk {
class NodeDisplacementFilter;
class NavigationDataSource;
}

namespace Ui {
class QmitkUSNavigationStepCtUsRegistration;
}

/**
 * \brief Navigation step for marking risk structures.
 * The user can add risk structures by interacting with the render windows. The
 * risk structures are organized in an embedded table view.
 *
 * The risk structures are stored under DATANAME_BASENODE -> DATANAME_ZONES.
 *
 * This step is ready for the next step directly after activating. All actions
 * to be done in this step are optional.
 */
class QmitkUSNavigationStepCtUsRegistration : public QmitkUSAbstractNavigationStep
{
  Q_OBJECT

protected slots:

public:
  explicit QmitkUSNavigationStepCtUsRegistration(QWidget *parent = 0);
  ~QmitkUSNavigationStepCtUsRegistration();

  /**
   * \brief Initialization of the data storage nodes.
   * \return always true
   */
  virtual bool OnStartStep();

  /**
   * \brief Resets widget and filter and removes nodes from the data storage.
   * \return always true
   */
  virtual bool OnStopStep();

  /**
   * \brief There is nothing to be done.
   * \return always true
   */
  virtual bool OnFinishStep();

  /**
   * \brief Selects input for the node displacement filter and emits "ReadyForNextStep" signal.
   * The input selection cannot be done earlier.
   * \return
   */
  virtual bool OnActivateStep();

  virtual bool OnDeactivateStep();

  /**
   * \brief Updates just the tracking validity status.
   */
  virtual void OnUpdate();

  /**
   * The property "settings.interaction-concept" is used.
   */
  virtual void OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode);

  virtual QString GetTitle();

  /**
   * @return a node displacement filter for the zone surfaces
   */
  virtual FilterVector GetFilter();

protected:
  virtual void OnSetCombinedModality();

  itk::SmartPointer<mitk::NavigationDataSource>   m_NavigationDataSource;


private:
  Ui::QmitkUSNavigationStepCtUsRegistration *ui;
};

#endif // QMITKUSNAVIGATIONSTEPCTUSREGISTRATION_H
