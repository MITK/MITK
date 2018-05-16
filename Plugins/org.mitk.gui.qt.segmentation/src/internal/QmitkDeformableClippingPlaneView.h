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
  virtual ~QmitkDeformableClippingPlaneView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

protected slots:

    void OnComboBoxSelectionChanged(const mitk::DataNode* node);
    void OnCreateNewClippingPlane();
    void OnCalculateClippingVolume();

    void OnTranslationMode(bool check);
    void OnRotationMode(bool check);
    void OnDeformationMode(bool check);

protected:

  virtual void OnSelectionChanged(mitk::DataNode* node);
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;
  virtual void NodeRemoved(const mitk::DataNode* node) override;
  virtual void NodeChanged(const mitk::DataNode* node) override;

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
