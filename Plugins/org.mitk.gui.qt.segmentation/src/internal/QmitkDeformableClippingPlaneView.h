/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _QMITKDEFORMABLECLIPPINGPLANEVIEW_H_INCLUDED
#define _QMITKDEFORMABLECLIPPINGPLANEVIEW_H_INCLUDED


#include <ui_QmitkDeformableClippingPlaneViewControls.h>
#include <mitkImage.h>
#include <QmitkAbstractView.h>

typedef itk::RGBPixel< float > Color;

/*!
* \ingroup org_mitk_gui_qt_deformableSurface
*
* \brief QmitkDeformableClippingPlaneView
*
* Document your class here.
*/
class QmitkDeformableClippingPlaneView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkDeformableClippingPlaneView();
  ~QmitkDeformableClippingPlaneView() override;

  void CreateQtPartControl(QWidget *parent) override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  ///
  /// Sets the focus to an internal widget.
  ///
  void SetFocus() override;

protected slots:

    void OnComboBoxSelectionChanged(const mitk::DataNode* node);
    void OnCreateNewClippingPlane();
    void OnCalculateClippingVolume();

    void OnTranslationMode(bool check);
    void OnRotationMode(bool check);
    void OnDeformationMode(bool check);

protected:

  virtual void OnSelectionChanged(mitk::DataNode* node);
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;
  void NodeRemoved(const mitk::DataNode* node) override;
  void NodeChanged(const mitk::DataNode* node) override;

  void UpdateView();

  Ui::QmitkDeformableClippingPlaneViewControls m_Controls;

private:
  mitk::DataStorage::SetOfObjects::ConstPointer GetAllClippingPlanes();
  mitk::Color GetLabelColor(int label);
  void DeactivateInteractionButtons();

  mitk::DataNode::Pointer m_ReferenceNode;
  mitk::DataNode::Pointer m_WorkingNode;
};

#endif
