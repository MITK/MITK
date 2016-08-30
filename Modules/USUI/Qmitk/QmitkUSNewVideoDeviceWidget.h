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

#ifndef _QmitkUSNewVideoDeviceWidget_H_INCLUDED
#define _QmitkUSNewVideoDeviceWidget_H_INCLUDED

#include "MitkUSUIExports.h"
#include "ui_QmitkUSNewVideoDeviceWidgetControls.h"
#include "mitkUSVideoDevice.h"
#include "mitkUSIGTLDevice.h"

//QT headers
#include <QWidget>
#include <QListWidgetItem>

//mitk header

/**
* @brief This Widget enables the USer to create and connect Video Devices.
*
* @ingroup USUI
*/
class MITKUSUI_EXPORT QmitkUSNewVideoDeviceWidget :public QWidget
{
  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkUSNewVideoDeviceWidget(QWidget* p = 0, Qt::WindowFlags f1 = 0);
  virtual ~QmitkUSNewVideoDeviceWidget();

  /* @brief This method is part of the widget an needs not to be called seperately. */
  virtual void CreateQtPartControl(QWidget *parent);
  /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
  virtual void CreateConnections();

signals:

  void Finished();

  public slots:

  /*
  \brief Activates the widget and displays the given device's Data to edit.
  */
  void EditDevice(mitk::USDevice::Pointer device);

  /*
  \brief Activates the widget with fields empty.
  */
  void CreateNewDevice();

  protected slots:

  /*
  \brief Called, when the the user clicks the "Done" button (Labeled either "Add Device" or "Edit Device", depending on the situation.
  */
  void OnClickedDone();

  void OnClickedFinishedEditing();

  /*
  \brief Called, when the button "Cancel" was clicked
  */
  void OnClickedCancel();

  /*
  \brief Called, when the Use selects one of the Radiobuttons
  */
  void OnDeviceTypeSelection();

  void OnOpenFileButtonClicked();

  void OnClickedRemoveProbe();

  void OnClickedRemoveDepth();

  void OnClickedAddDepths();

  void OnProbeChanged(const QString & probename);

protected:

  Ui::QmitkUSNewVideoDeviceWidgetControls* m_Controls; ///< member holding the UI elements of this widget

  /*
  \brief Constructs a ListItem from the given device for display in the list of active devices
  */
  QListWidgetItem* ConstructItemFromDevice(mitk::USDevice::Pointer device);

  void ChangeUIEditingUSVideoDevice();

  void CleanUpAfterEditingOfDevice();

  void CleanUpAfterCreatingNewDevice();

  void AddProbesToDevice(mitk::USVideoDevice::Pointer device);

  /*
  \brief Displays whether this widget is active or not. It gets activated by either sending a Signal to
  *  the "CreateNewDevice" Slot or to the "EditDevice" Slot. If the user finishes editing the device, a
  * "EditingComplete" Signal is sent, and the widget is set to inactive again. Clicking Cancel also
  *  deactivates it.
  */
  bool m_Active;

  /**
  *   \brief This is the device to edit. It is either the device transmitted in the "EditDevice" signal, or a new one
  *  if the "CreateNewDevice slot was called.
  */
  mitk::USVideoDevice::Pointer m_TargetDevice;
};

#endif // _QmitkUSNewVideoDeviceWidget_H_INCLUDED
