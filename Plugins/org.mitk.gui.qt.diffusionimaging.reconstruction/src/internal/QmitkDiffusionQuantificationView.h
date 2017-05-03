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

#ifndef _QMITKDIFFUSIONQUANTIFICATIONVIEW_H_INCLUDED
#define _QMITKDIFFUSIONQUANTIFICATIONVIEW_H_INCLUDED

#include <QmitkAbstractView.h>

#include <string>

#include "ui_QmitkDiffusionQuantificationViewControls.h"
/*!
 * \ingroup org_mitk_gui_qt_diffusionquantification_internal
 *
 * \brief QmitkDiffusionQuantificationView
 *
 * Document your class here.
 */
class QmitkDiffusionQuantificationView : public QmitkAbstractView
{

  friend struct DqSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkDiffusionQuantificationView();
  QmitkDiffusionQuantificationView(const QmitkDiffusionQuantificationView& other);
  virtual ~QmitkDiffusionQuantificationView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

protected slots:

  void GFACheckboxClicked();

  void GFA();
  void Curvature();
  void FA();
  void RA();
  void AD();
  void RD();
  void ClusterAnisotropy();
  void MD();

  void QBIQuantify(int method);
  void QBIQuantification(mitk::DataStorage::SetOfObjects::Pointer inImages,
    int method) ;

  void TensorQuantify(int method);
  void TensorQuantification(mitk::DataStorage::SetOfObjects::Pointer inImages,
    int method) ;

protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkDiffusionQuantificationViewControls* m_Controls;

  mitk::DataStorage::SetOfObjects::Pointer m_QBallImages;
  mitk::DataStorage::SetOfObjects::Pointer m_TensorImages;

  static const float m_ScaleDAIValues;
};




#endif // _QMITKDIFFUSIONQUANTIFICATIONVIEW_H_INCLUDED

