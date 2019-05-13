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

#ifndef QMITKUSNAVIGATIONCALIBRATIONSDATAMODEL_H
#define QMITKUSNAVIGATIONCALIBRATIONSDATAMODEL_H

#include <QAbstractTableModel>

#include <mitkMessage.h>
#include <itkSmartPointer.h>

namespace mitk {
  class AbstractUltrasoundTrackerDevice;
  class USControlInterfaceBMode;
  class USControlInterfaceProbes;
}

class QmitkUSNavigationCalibrationsDataModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit QmitkUSNavigationCalibrationsDataModel(QObject *parent = 0);
  virtual ~QmitkUSNavigationCalibrationsDataModel();

  void SetCombinedModality(itk::SmartPointer<mitk::AbstractUltrasoundTrackerDevice> combinedModality);

  void OnDeviceChanged(const std::string&, const std::string&);

  /** \brief Return number of rows of the model. */
  virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

  /** \brief Return number of columns (3) of the model. */
  virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;

  /** \brief Return names for the columns, numbers for the rows and invalid for DisplayRole. */
  virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  /** \brief Return selectable and enabled for column 1 (size); selectable, enabled and editable for every other column. */
  virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;

  /** \brief Return model data of the selected cell. */
  virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

  /** \brief Set model data for the selected cell. */
  virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

  using QAbstractTableModel::removeRows;
  /** \brief Remove given rows from the model.
   *  \param removeFromDataStorage zone nodes are removed from the data storage too, if this is set to true
   */
  virtual bool removeRows ( int row, int count, const QModelIndex& parent, bool removeFromDataStorage );

private:
  itk::SmartPointer<mitk::AbstractUltrasoundTrackerDevice>       m_CombinedModality;
  itk::SmartPointer<mitk::USControlInterfaceBMode>  m_ControlInterfaceBMode;

  mitk::MessageDelegate2<QmitkUSNavigationCalibrationsDataModel, const std::string&, const std::string&> m_ListenerDeviceChanged;
};

#endif // QMITKUSNAVIGATIONCALIBRATIONSDATAMODEL_H
