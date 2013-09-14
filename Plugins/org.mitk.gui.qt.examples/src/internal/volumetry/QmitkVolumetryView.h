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

#ifndef _QMITKVOLUMETRYVIEW_H_INCLUDED
#define _QMITKVOLUMETRYVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkVolumetryViewControls.h"



/*!
 * \ingroup org_mitk_gui_qt_volumetry_internal
 *
 * \brief QmitkVolumetryView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkVolumetryView : public QmitkFunctionality
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkVolumetryView();

  virtual ~QmitkVolumetryView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

protected:

  mitk::Image* GetImage();

  mitk::DataNode* GetOverlayNode();

  void CreateOverlayChild();

  void UpdateSliderLabel();

  void UpdateSlider();

  const mitk::DataNode* GetImageNode();


protected slots:


  void OnCalculateVolume();

  void OnTimeSeriesButtonClicked();

  void OnThresholdSliderChanged( int value );

  void OnSaveCsvButtonClicked();

  void OnImageSelected(const mitk::DataNode* item);



protected:

  Ui::QmitkVolumetryViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  /// store weak pointer of the DataNode
  mitk::WeakPointer<mitk::DataNode> m_SelectedDataNode;


  mitk::DataNode::Pointer m_OverlayNode;

  mitk::DataStorage::Pointer m_DataStorage;

};




#endif // _QMITKVOLUMETRYVIEW_H_INCLUDED

