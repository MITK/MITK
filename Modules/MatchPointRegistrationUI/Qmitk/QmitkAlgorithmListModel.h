/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAlgorithmListModel_h
#define QmitkAlgorithmListModel_h

#include <QAbstractTableModel>
#include <QStringList>

// MITK
#include "MitkMatchPointRegistrationUIExports.h"

// MatchPoint
#include <mapDeploymentDLLDirectoryBrowser.h>

/*!
  \class QmitkAlgorithmListModel
  Model that takes a list of MatchPoint algorithm dll handles and represents it as model in context of the QT
  view-model-concept.
  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.
*/
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkAlgorithmListModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  QmitkAlgorithmListModel(QObject *parent = nullptr);
  ~QmitkAlgorithmListModel() override{};

  void SetAlgorithms(::map::deployment::DLLDirectoryBrowser::DLLInfoListType algList);

  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
  ::map::deployment::DLLDirectoryBrowser::DLLInfoListType m_AlgList;
};

#endif // mitkQmitkAlgorithmListModel_h
