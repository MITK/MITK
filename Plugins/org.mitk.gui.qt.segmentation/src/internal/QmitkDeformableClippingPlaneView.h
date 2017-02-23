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


#include "ui_QmitkDeformableClippingPlaneViewControls.h"
#include "mitkImage.h"
#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>

typedef itk::RGBPixel< float > Color;

/*!
* \ingroup org_mitk_gui_qt_deformableSurface
*
* \brief QmitkDeformableClippingPlaneView
*
* Document your class here.
*/
class QmitkDeformableClippingPlaneView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
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

  /// \brief Called when the view gets activated
  virtual void Activated() override;

  /// \brief Called when the view gets deactivated
  virtual void Deactivated() override;

  /// \brief Called when the view becomes visible
  virtual void Visible() override;

  /// \brief Called when the view becomes hidden
  virtual void Hidden() override;

protected slots:

    /// \brief Called when the user clicks the GUI button/makes a selection
    void OnComboBoxSelectionChanged(const mitk::DataNode* node);
    void OnCreateNewClippingPlane();
    void OnCalculateClippingVolume();

    void OnTranslationMode(bool check);
    void OnRotationMode(bool check);
    void OnDeformationMode(bool check);

protected:

  /*!
  \brief Invoked when the DataManager selection changed
  */
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

#endif // _QMITKDEFORMABLECLIPPINGPLANEVIEW_H_INCLUDED

