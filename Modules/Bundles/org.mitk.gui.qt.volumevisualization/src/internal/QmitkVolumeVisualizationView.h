/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15646 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKVOLUMEVISUALIZATIONVIEW_H_
#define QMITKVOLUMEVISUALIZATIONVIEW_H_

#include <QObject>
#include <QmitkFunctionality.h>
#include <mitkWeakPointer.h>

#include "mitkDataStorage.h"

#include <QmitkDataStorageListModel.h>
#include <QmitkDataStorageComboBox.h>
#include <QmitkTransferFunctionWidget.h>

#include "ui_QmitkVolumeVisualizationViewControls.h"

/**
 * \ingroup org_mitk_gui_qt_volumevisualization_internal
 */
class QmitkVolumeVisualizationView : public QObject, public QmitkFunctionality
{
  Q_OBJECT

public:

  void SetFocus();

  QmitkVolumeVisualizationView();

  virtual ~QmitkVolumeVisualizationView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();


protected slots:

  void OnImageSelected(const mitk::DataTreeNode* item);
  
  void OnEnableRendering( bool state );

  //TF Stlye Tab
  void OnTransferFunctionModeChanged( int mode );

  //Preferences Tab
  void OnEnableClippingPlane( bool state );

  void OnEnableShadingLow( bool );
  void OnEnableShadingMed( bool );
  void OnEnableShadingHigh( bool );

  void OnSetShadingValuesClicked();

  void OnSetDefaultShadingValuesClicked();


protected:

  Ui::QmitkVolumeVisualizationViewControls* m_Controls;


  bool image_ok;


};

#endif /*QMITKVOLUMEVISUALIZATIONVIEW_H_*/
