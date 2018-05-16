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

#ifndef QMITK_ALGORITHM_SETTINGS_CONFIG_H
#define QMITK_ALGORITHM_SETTINGS_CONFIG_H

#include <QSortFilterProxyModel>
#include <QWidget>

#include <mapDeploymentDLLInfo.h>

#include <QmitkMAPAlgorithmModel.h>

#include <MitkMatchPointRegistrationUIExports.h>

#include "ui_QmitkAlgorithmSettingsConfig.h"

/**
 * \class QmitkAlgorithmSettingsConfig
 * \brief Widget that views the information and profile of an algorithm stored in an DLLInfo object.
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkAlgorithmSettingsConfig : public QWidget,
                                                                         private Ui::QmitkAlgorithmSettingsConfig
{
  Q_OBJECT

public:
  QmitkAlgorithmSettingsConfig(QWidget *parent = nullptr);

  /**
   * \brief Changes the current algorithm and updates widget accordingly.
   * \param pointer to the algorithm instance.
   */
  void setAlgorithm(map::algorithm::RegistrationAlgorithmBase *alg);

  map::algorithm::RegistrationAlgorithmBase *getAlgorithm();

protected:
  /** Pointer to the algorithm that should be configured */
  map::algorithm::RegistrationAlgorithmBase::Pointer m_currentAlg;

  // control the properties of the selected algorithm
  QmitkMAPAlgorithmModel *m_AlgorithmModel;
  QSortFilterProxyModel *m_ProxyModel;
};

#endif // QmitkAlgorithmSettingsConfig_H
