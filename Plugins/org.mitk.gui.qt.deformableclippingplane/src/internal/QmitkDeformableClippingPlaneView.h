/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDEFORMABLECLIPPINGPLANEVIEW_H
#define QMITKDEFORMABLECLIPPINGPLANEVIEW_H

#include <ui_QmitkDeformableClippingPlaneViewControls.h>

#include <mitkImage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>
#include <QmitkAbstractView.h>

typedef itk::RGBPixel< float > Color;

/**
 * @brief
 */
class QmitkDeformableClippingPlaneView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkDeformableClippingPlaneView();
  ~QmitkDeformableClippingPlaneView() override;

  void SetFocus() override;

private Q_SLOTS:

    void OnCurrentSelectionChanged(const QList<mitk::DataNode::Pointer>& nodes);
    void OnComboBoxSelectionChanged(const mitk::DataNode* node);
    void OnCreateNewClippingPlane();
    void OnCalculateClippingVolume();

    void OnTranslationMode(bool check);
    void OnRotationMode(bool check);
    void OnDeformationMode(bool check);

private:

  void CreateQtPartControl(QWidget *parent) override;
  virtual void CreateConnections();

  void NodeRemoved(const mitk::DataNode* node) override;
  void NodeChanged(const mitk::DataNode* node) override;

  void UpdateView();

  mitk::DataStorage::SetOfObjects::ConstPointer GetAllClippingPlanes();
  mitk::Color GetLabelColor(int label);
  void DeactivateInteractionButtons();

  Ui::QmitkDeformableClippingPlaneViewControls* m_Controls;

  mitk::NodePredicateAnd::Pointer m_IsImagePredicate;
  mitk::NodePredicateProperty::Pointer m_IsClippingPlanePredicate;

  mitk::DataNode::Pointer m_WorkingNode;

};

#endif
