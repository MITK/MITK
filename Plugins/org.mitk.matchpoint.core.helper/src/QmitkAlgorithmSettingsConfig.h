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

#include <QWidget>
#include <QSortFilterProxyModel>

#include <mapDeploymentDLLInfo.h>

#include "mitkQMAPAlgorithmModel.h"

#include "org_mitk_matchpoint_core_helper_Export.h"

#include "ui_QmitkAlgorithmSettingsConfig.h"

/**
 * \class QmitkAlgorithmSettingsConfig
 * \brief Widget that views the information and profile of an algorithm stored in an DLLInfo object.
 */
class MITK_MATCHPOINT_CORE_HELPER_EXPORT QmitkAlgorithmSettingsConfig : public QWidget, private Ui::QmitkAlgorithmSettingsConfig
{
  Q_OBJECT

public:
  QmitkAlgorithmSettingsConfig(QWidget* parent=0);

  /**
   * \brief Changes the current algorithm and updates widget accordingly.
   * \param pointer to the algorithm instance.
   */
  void setAlgorithm(map::algorithm::RegistrationAlgorithmBase* alg);

  map::algorithm::RegistrationAlgorithmBase* getAlgorithm();

protected:
  /** Pointer to the algorithm that should be configured */
  map::algorithm::RegistrationAlgorithmBase::Pointer m_currentAlg;

  // control the properties of the selected algorithm
  mitk::QMAPAlgorithmModel* m_AlgorithmModel;
  QSortFilterProxyModel* m_ProxyModel;
};

#endif // QmitkAlgorithmSettingsConfig_H
