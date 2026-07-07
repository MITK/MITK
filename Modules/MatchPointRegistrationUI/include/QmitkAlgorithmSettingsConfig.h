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
 * \brief Widget for viewing and editing the configurable meta-properties of a MatchPoint registration algorithm.
 *
 * This widget provides a table view of all meta-properties exposed by a MatchPoint
 * registration algorithm through its MetaPropertyAlgorithmInterface. Properties are
 * displayed via a QmitkMAPAlgorithmModel backed by a QSortFilterProxyModel, supporting
 * case-insensitive filtering and sorted display. Writable properties can be edited
 * directly in the table.
 *
 * \sa QmitkMAPAlgorithmModel, QmitkAlgorithmProfileViewer
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkAlgorithmSettingsConfig : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the algorithm settings configuration widget.
   * \param[in] parent Optional parent widget.
   */
  QmitkAlgorithmSettingsConfig(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkAlgorithmSettingsConfig() override;

  /**
   * \brief Sets the registration algorithm whose properties should be displayed and edited.
   *
   * If the given algorithm differs from the currently set one, the internal model is
   * updated and the view is refreshed.
   *
   * \param[in] alg Pointer to the registration algorithm instance. May be \c nullptr to clear.
   */
  void setAlgorithm(map::algorithm::RegistrationAlgorithmBase *alg);

  /**
   * \brief Returns the currently configured registration algorithm.
   * \return Pointer to the current algorithm, or \c nullptr if none is set.
   */
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
