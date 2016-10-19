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
#ifndef QMITKUSZONEMANAGEMENTWIDGET_H
#define QMITKUSZONEMANAGEMENTWIDGET_H

#include <QWidget>
#include "mitkDataStorage.h"

namespace itk {
template<class T> class SmartPointer;
}

namespace mitk {
class USZonesInteractor;
}

namespace Ui {
class QmitkUSZoneManagementWidget;
}

class QmitkUSZonesDataModel;
class QItemSelection;
class QModelIndex;

/**
 * \brief Shows a table of the zone nodes and allows to change properties and add and delete zone nodes.
 *
 * A data node interactor (mitk::USZonesInteractor) is used for creating new zone nodes and a QTableModel
 * (QmitkUSZonesDataModel) is used for handling the zones data.
 *
 * When using the widget, one has to call OnStartAddingZone() every time he wants the interaction for adding
 * new zone nodes to be active.
 */
class QmitkUSZoneManagementWidget : public QWidget
{
  Q_OBJECT

signals:
  /**
   * \brief Emmited whenever a new zone was added to the data model.
   */
  void ZoneAdded();

  /**
   * \brief Emmited whenever a new zone was removed from the data mode.
   */
  void ZoneRemoved();

public slots:
  /**
   * \brief Removes all rows from the data model which are selected in the table.
   */
  void RemoveSelectedRows();

  /**
   * \brief Creates a new zone node and activates the data interactor on this node.
   * This slot has to be called whenever a new zone should be added. Interactions are active
   * then, so the user can create the zone.
   */
  void OnStartAddingZone();

  /**
   * \brief Aborts the creation of a new zone and deletes the corresponding node.
   */
  void OnAbortAddingZone();

  /**
   * \brief Removes all nodes from the data model.
   */
  void OnResetZones();

protected slots:
  void OnSelectionChanged(const QItemSelection& selected, const QItemSelection & deselected);

  /**
   * \brief Updates zone radius according to the value of the zone size slider.
   * \param value new radius of the zone
   */
  void OnZoneSizeSliderValueChanged(int value);

  /**
   * \brief Updates maximum number of added zones and selects the last row of the table.
   * It is called every time a row was added to the data model.
   */
  void OnRowInsertion(const QModelIndex& parent, int start, int end);

  /**
   * \brief Updates the zone size slider when data was changed.
   * It is called every time a row was changed in the data model.
   */
  void OnDataChanged(const QModelIndex& topLeft, const QModelIndex& /*bottomRight*/);

public:
  explicit QmitkUSZoneManagementWidget(QWidget* parent = 0);
  ~QmitkUSZoneManagementWidget();

  /**
   * \brief Sets the state machine file for being used by the mitk::USZonesInteractor
   * \param filename name of the state machine file
   */
  void SetStateMachineFilename(const std::string& filename);

  /**
   * \brief Setter for the DataStorage where the zone nodes will be stored.
   * The nodes will be derivates of the node specified by the base node name.
   *
   * \param dataStorage data storage where the zone nodes will be stored
   * \param baseNodeName optional name of the node which will be the source node of all zone nodes (defaults "Zones")
   */
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage, const char* baseNodeName = "Zones");

  /**
   * \brief Setter for the DataStorage where the zone nodes will be stored.
   * The nodes will be derivates of the base nodes.
   *
   * \param dataStorage data storage where the zone nodes will be strored
   * \param baseNode data node which will be the source node of all zone nodes
   */
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage, itk::SmartPointer<mitk::DataNode> baseNode);

  /**
   * \brief Get all zone nodes from the data storage.
   */
  mitk::DataStorage::SetOfObjects::ConstPointer GetZoneNodes();

protected:
  QmitkUSZonesDataModel*                      m_ZonesDataModel;

  itk::SmartPointer<mitk::USZonesInteractor>  m_Interactor;
  mitk::DataStorage::Pointer                  m_DataStorage;
  mitk::DataNode::Pointer                     m_BaseNode;

  std::string                                 m_StateMachineFileName;

private:
  Ui::QmitkUSZoneManagementWidget* ui;

  unsigned int m_CurMaxNumOfZones;
};

#endif // QMITKUSZONEMANAGEMENTWIDGET_H
