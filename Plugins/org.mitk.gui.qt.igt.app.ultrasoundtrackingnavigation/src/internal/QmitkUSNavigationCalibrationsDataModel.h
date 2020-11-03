/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  explicit QmitkUSNavigationCalibrationsDataModel(QObject *parent = nullptr);
  ~QmitkUSNavigationCalibrationsDataModel() override;

  void SetCombinedModality(itk::SmartPointer<mitk::AbstractUltrasoundTrackerDevice> combinedModality);

  void OnDeviceChanged(const std::string&, const std::string&);

  /** \brief Return number of rows of the model. */
  int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;

  /** \brief Return number of columns (3) of the model. */
  int columnCount ( const QModelIndex & parent = QModelIndex() ) const override;

  /** \brief Return names for the columns, numbers for the rows and invalid for DisplayRole. */
  QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

  /** \brief Return selectable and enabled for column 1 (size); selectable, enabled and editable for every other column. */
  Qt::ItemFlags flags ( const QModelIndex & index ) const override;

  /** \brief Return model data of the selected cell. */
  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;

  /** \brief Set model data for the selected cell. */
  bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole ) override;

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
