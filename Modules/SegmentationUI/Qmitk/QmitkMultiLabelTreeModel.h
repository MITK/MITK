/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkMultiLabelTreeModel_h
#define QmitkMultiLabelTreeModel_h

#include "mitkLabelSetImage.h"

// qt
#include <QAbstractItemModel>

#include "MitkSegmentationUIExports.h"


class QmitkMultiLabelSegTreeItem;

/*!
\class QmitkMultiLabelTreeModel
The class is used to represent the information of an MITK MultiLabel segmentation instance (labels, spacial groups...).
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMultiLabelTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
  using LabelValueType = mitk::LabelSetImage::LabelValueType;
  using GroupIndexType = mitk::LabelSetImage::GroupIndexType;

  QmitkMultiLabelTreeModel(QObject *parent = nullptr);
  ~QmitkMultiLabelTreeModel() override;

  void SetSegmentation(mitk::LabelSetImage* segmentation);
  const mitk::LabelSetImage* GetSegmentation() const;

  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;

  /** returns the index of a passed label value (always first column). If label value does not exist in
  segmentation or segmentation is not set an invalid index will be returned.*/
  QModelIndex indexOfLabel(mitk::Label::PixelType labelValue) const;
  QModelIndex indexOfGroup(mitk::LabelSetImage::GroupIndexType groupIndex) const;
  /** Returns the index to the next node in the tree that behaves like an instance (label node with only one instance
  or instance node). If current index is at the end, an invalid index is returned.*/
  QModelIndex ClosestLabelInstanceIndex(const QModelIndex& currentIndex) const;
  /** Returns the index to the first child node (or itself) in the tree that behaves like an instance (label node with only one instance
  or instance node). If current index is at the end, an invalid index is returned. If an invalid index is passed into the methods,
  the search starts at the root; thus the whole tree is search for the first label instance.*/
  QModelIndex FirstLabelInstanceIndex(const QModelIndex& currentIndex) const;

  ///** Returns the index to the next node in the tree that behaves like an instance (label node with only one instance
  //or instance node). If current index is at the end, an invalid index is returned.*/
  //QModelIndex PrevLabelInstanceIndex(const QModelIndex& currentIndex) const;

  /** Returns a vector containing all label values of the passed currentIndex or its child items.*/
  std::vector <LabelValueType> GetLabelsInSubTree(const QModelIndex& currentIndex) const;

  /** Returns a vector containing all label values of all label instances that belong to the same label
   * class like the passed index.
   *
   * If index points to a group or invalid, nothing will be returned.
   * @pre currentIndex must be valid and point to a label (class or instance).
   */
  std::vector <LabelValueType> GetLabelInstancesOfSameLabelClass(const QModelIndex& currentIndex) const;

  enum TableColumns
  {
    NAME_COL = 0,
    LOCKED_COL,
    COLOR_COL,
    VISIBLE_COL
  };

  enum ItemModelRole
  {
    /**This role returns the label object that is associated with an index.
       - On group level it always returns an invalid QVariant
       - On label level (with multiple instances) it returns the first label instance).
       - On instance level it returns the label instance object.*/
    LabelDataRole = 64,
    /**This role returns only the label value of the label that would be returned by
       LabelDataRole.*/
    LabelValueRole = 65,
    /**Simelar to LabelDataRole, but only returns a valid QVariant if index points only to
      a specific instance (so either instance level or label level with only one instance).
      You can use that role if you want to assure that only one specific label instance is
      referenced by the index.*/
    LabelInstanceDataRole = 66,
    /**Simelar to LabelValueRole, but like LabelInstanceDataRole only returns a valid QVariant
      if index points only to a specific instance (so either instance level or label
      level with only one instance).
      You can use that role if you want to assure that only one specific label instance is
      referenced by the index.*/
    LabelInstanceValueRole = 67,
    /**This role returns the group ID the item/index belongs to.*/
    GroupIDRole = 68
  };

  bool GetAllowVisibilityModification() const;
  bool GetAllowLockModification() const;

public Q_SLOTS:
  void SetAllowVisibilityModification(bool vmod);
  void SetAllowLockModification(bool lmod);

Q_SIGNALS:
  void dataAvailable();
  /** Is emitted whenever the model changes are finished (usually a bit later than dataAvailable()).*/
  void modelChanged();

protected:
  void OnLabelAdded(LabelValueType labelValue);
  void OnLabelModified(LabelValueType labelValue);
  void OnLabelRemoved(LabelValueType labelValue);
  void OnGroupAdded(GroupIndexType groupIndex);
  void OnGroupModified(GroupIndexType groupIndex);
  void OnGroupRemoved(GroupIndexType groupIndex);

private:
  void AddObserver();
  void RemoveObserver();

  void UpdateInternalTree();
  void GenerateInternalGroupTree(unsigned int layerID, QmitkMultiLabelSegTreeItem* layerItem);
  QmitkMultiLabelSegTreeItem* GenerateInternalTree();

  /* builds a hierarchical tree model for the image statistics
  1. Level: Image
  --> 2. Level: Mask [if exist]
      --> 3. Level: Timestep [if >1 exist] */
  void BuildHierarchicalModel();

  mitk::LabelSetImage::Pointer m_Segmentation;

  std::mutex m_Mutex;
  std::unique_ptr<QmitkMultiLabelSegTreeItem> m_RootItem;

  bool m_Observed;
  bool m_ShowGroups = true;

  bool m_ShowVisibility = true;
  bool m_ShowLock = true;
  bool m_ShowOther = false;

  bool m_AllowVisibilityModification = true;
  bool m_AllowLockModification = true;
};

#endif
