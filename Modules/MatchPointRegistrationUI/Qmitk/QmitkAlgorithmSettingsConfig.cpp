/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkAlgorithmSettingsConfig.h>

#include <mapConvert.h>
#include <mapMetaPropertyAccessor.h>

#include <ui_QmitkAlgorithmSettingsConfig.h>

QmitkAlgorithmSettingsConfig::QmitkAlgorithmSettingsConfig(QWidget *parent)
  : QWidget(parent),
    m_Controls(std::make_unique<Ui::QmitkAlgorithmSettingsConfig>())
{
  m_Controls->setupUi(this);

  m_AlgorithmModel = new QmitkMAPAlgorithmModel(this);
  m_ProxyModel = new QSortFilterProxyModel(this);

  // configure property model/view/widget
  m_ProxyModel->setSourceModel(m_AlgorithmModel);
  m_ProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_ProxyModel->setDynamicSortFilter(true);

  m_Controls->m_AlgoPropertiesView->setModel(m_ProxyModel);
  m_Controls->m_AlgoPropertiesView->setSortingEnabled(true);
  m_Controls->m_AlgoPropertiesView->setAlternatingRowColors(true);
  m_Controls->m_AlgoPropertiesView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls->m_AlgoPropertiesView->setSelectionBehavior(QAbstractItemView::SelectItems);
}

QmitkAlgorithmSettingsConfig::~QmitkAlgorithmSettingsConfig()
{
}

void QmitkAlgorithmSettingsConfig::setAlgorithm(map::algorithm::RegistrationAlgorithmBase *alg)
{
  if (alg != this->m_currentAlg)
  {
    this->m_currentAlg = alg;
    this->m_AlgorithmModel->SetAlgorithm(this->m_currentAlg);

    m_Controls->m_AlgoPropertiesView->setWindowModified(true);
    m_Controls->m_AlgoPropertiesView->update();
  }
}

map::algorithm::RegistrationAlgorithmBase *QmitkAlgorithmSettingsConfig::getAlgorithm()
{
  return this->m_currentAlg;
}
