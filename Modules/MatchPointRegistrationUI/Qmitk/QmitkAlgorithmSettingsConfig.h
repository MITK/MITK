/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAlgorithmSettingsConfig_h
#define QmitkAlgorithmSettingsConfig_h

#include <QSortFilterProxyModel>
#include <QWidget>

#include <mapDeploymentDLLInfo.h>

#include <QmitkMAPAlgorithmModel.h>

#include <MitkMatchPointRegistrationUIExports.h>
#include <memory>

namespace Ui
{
  class QmitkAlgorithmSettingsConfig;
}

/**
 * \class QmitkAlgorithmSettingsConfig
 * \brief Widget that views the information and profile of an algorithm stored in an DLLInfo object.
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkAlgorithmSettingsConfig : public QWidget
{
  Q_OBJECT

public:
  QmitkAlgorithmSettingsConfig(QWidget *parent = nullptr);
  ~QmitkAlgorithmSettingsConfig() override;

  /**
   * \brief Changes the current algorithm and updates widget accordingly.
   * \param alg pointer to the algorithm instance.
   */
  void setAlgorithm(map::algorithm::RegistrationAlgorithmBase *alg);

  map::algorithm::RegistrationAlgorithmBase *getAlgorithm();

protected:
  std::unique_ptr<Ui::QmitkAlgorithmSettingsConfig> m_Controls;

  /** Pointer to the algorithm that should be configured */
  map::algorithm::RegistrationAlgorithmBase::Pointer m_currentAlg;

  // control the properties of the selected algorithm
  QmitkMAPAlgorithmModel *m_AlgorithmModel;
  QSortFilterProxyModel *m_ProxyModel;
};

#endif
