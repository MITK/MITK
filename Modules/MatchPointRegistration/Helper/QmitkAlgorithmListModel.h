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

#ifndef QmitkAlgorithmListModel_h
#define QmitkAlgorithmListModel_h

#include <QAbstractTableModel>
#include <QStringList>

//MITK
#include "MitkMatchPointRegistrationExports.h"

// MatchPoint
#include <mapDeploymentDLLDirectoryBrowser.h>

/*!
  \class QmitkAlgorithmListModel
  Model that takes a list of MatchPoint algorithm dll handles and represents it as model in context of the QT view-model-concept.
  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.
*/
class MITKMATCHPOINTREGISTRATION_EXPORT QmitkAlgorithmListModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    QmitkAlgorithmListModel(QObject *parent = nullptr);
    virtual ~QmitkAlgorithmListModel() {};

    void SetAlgorithms(::map::deployment::DLLDirectoryBrowser::DLLInfoListType algList);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:

  ::map::deployment::DLLDirectoryBrowser::DLLInfoListType m_AlgList;
};

#endif // mitkQmitkAlgorithmListModel_h

