/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBoundingObjectWidget_h
#define QmitkBoundingObjectWidget_h

// includes
#include <MitkQtWidgetsExtExports.h>
#include <QComboBox>
#include <QPushButton>
#include <QTreeWidget>
#include <mitkBoundingObject.h>
#include <mitkBoundingObjectGroup.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <QWidget>

/**
 * \brief Widget for creating, managing, and selecting bounding objects.
 *
 * Provides a tree view listing bounding objects (cuboid, cone, ellipsoid, cylinder)
 * with controls to add, remove, rename, and toggle visibility and inversion.
 * Selected bounding objects receive an AffineBaseDataInteractor3D for interactive
 * 3D manipulation. A union BoundingObjectGroup of all objects can be retrieved.
 *
 * \pre A mitk::DataStorage must be set via SetDataStorage() before creating objects.
 *
 * \sa mitk::BoundingObject, mitk::BoundingObjectGroup
 */
class MITKQTWIDGETSEXT_EXPORT QmitkBoundingObjectWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Construct the bounding object widget.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QmitkBoundingObjectWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  /** \brief Destructor. */
  ~QmitkBoundingObjectWidget() override;

  /**
   * \brief Set the data storage used to store bounding object nodes.
   * \param[in] dataStorage Pointer to the data storage instance.
   */
  void SetDataStorage(mitk::DataStorage *dataStorage);

  /**
   * \brief Get the currently assigned data storage.
   * \return Pointer to the data storage, or nullptr if not set.
   */
  mitk::DataStorage *GetDataStorage();

  /**
   * \brief Get the currently selected bounding object.
   * \return Smart pointer to the selected mitk::BoundingObject, or nullptr if none is selected.
   */
  mitk::BoundingObject::Pointer GetSelectedBoundingObject();

  /**
   * \brief Get the data node of the currently selected bounding object.
   * \return Smart pointer to the selected mitk::DataNode, or nullptr if none is selected.
   */
  mitk::DataNode::Pointer GetSelectedBoundingObjectNode();

  /**
   * \brief Get a data node containing a BoundingObjectGroup with all bounding objects.
   *
   * Creates a union BoundingObjectGroup from all nodes in the data storage that have
   * the "bounding object" property set to true.
   *
   * \return Data node containing the group, or nullptr if no bounding objects exist.
   */
  mitk::DataNode::Pointer GetAllBoundingObjects();

  /**
   * \brief Enable or disable the widget and toggle bounding object visibility.
   * \param[in] flag True to enable, false to disable.
   */
  void setEnabled(bool flag);

  /**
   * \brief ITK observer callback for bounding object modification events.
   * \param[in] e The ITK event object (AffineInteractionEvent).
   */
  void OnBoundingObjectModified(const itk::EventObject &e);

  /** \brief Remove all bounding object items from the list and reset the counter. */
  void RemoveAllItems();

signals:
  /** \brief Emitted when bounding objects are added, removed, or modified. */
  void BoundingObjectsChanged();

protected slots:
  void CreateBoundingObject(int type);
  void OnDelButtonClicked();
  void SelectionChanged();
  void OnItemDoubleClicked(QTreeWidgetItem *item, int col);
  void OnItemDataChanged(QTreeWidgetItem *item, int col);

protected:
  void AddItem(mitk::DataNode *node);
  void RemoveItem();

  mitk::DataStorage *m_DataStorage;
  QTreeWidget *m_TreeWidget;
  QComboBox *m_addComboBox;
  QPushButton *m_DelButton;
  QPushButton *m_SaveButton;
  QPushButton *m_LoadButton;
  QTreeWidgetItem *m_lastSelectedItem;
  unsigned long m_lastAffineObserver;

  typedef std::map<QTreeWidgetItem *, mitk::DataNode *> ItemNodeMapType;
  ItemNodeMapType m_ItemNodeMap;

  unsigned int m_BoundingObjectCounter;

  enum BoundingObjectType
  {
    CUBOID,
    CONE,
    ELLIPSOID,
    CYLINDER,
  };
};
#endif
