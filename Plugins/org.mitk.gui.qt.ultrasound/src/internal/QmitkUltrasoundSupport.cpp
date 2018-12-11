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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

//Mitk
#include <mitkDataNode.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkIRenderingManager.h>
#include <mitkImageGenerator.h>

// Qmitk
#include "QmitkUltrasoundSupport.h"

// Qt
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

// Ultrasound
#include "mitkUSDevice.h"
#include "QmitkUSAbstractCustomWidget.h"

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include "usServiceReference.h"
#include "internal/org_mitk_gui_qt_ultrasound_Activator.h"

const std::string QmitkUltrasoundSupport::VIEW_ID = "org.mitk.views.ultrasoundsupport";

QmitkUltrasoundSupport::QmitkUltrasoundSupport()
  : m_Controls(nullptr), m_ControlCustomWidget(0), m_ControlBModeWidget(0),
  m_ControlProbesWidget(0), m_ImageAlreadySetToNode(false),
  m_CurrentImageWidth(0), m_CurrentImageHeight(0)
{
  ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();

  if (pluginContext)
  {
    // to be notified about service event of an USDevice
    pluginContext->connectServiceListener(this, "OnDeviceServiceEvent",
      QString::fromStdString("(" + us::ServiceConstants::OBJECTCLASS() + "=" + us_service_interface_iid<mitk::USDevice>() + ")"));
  }
}

QmitkUltrasoundSupport::~QmitkUltrasoundSupport()
{
  try
  {
    StoreUISettings();
    StopTimers();

    // Get all active devicesand deactivate them to prevent freeze
    std::vector<mitk::USDevice*> devices = this->m_Controls->m_ActiveVideoDevices->GetAllServices<mitk::USDevice>();
    for (size_t i = 0; i < devices.size(); i++)
    {
      mitk::USDevice::Pointer device = devices[i];
      if (device.IsNotNull() && device->GetIsActive())
      {
        device->Deactivate();
        device->Disconnect();
      }
    }
  }
  catch (std::exception &e)
  {
    MITK_ERROR << "Exception during call of destructor! Message: " << e.what();
  }
}

void QmitkUltrasoundSupport::SetFocus()
{
}

void QmitkUltrasoundSupport::CreateQtPartControl(QWidget *parent)
{
  //initialize timers
  m_UpdateTimer = new QTimer(this);
  m_RenderingTimer2d = new QTimer(this);
  m_RenderingTimer3d = new QTimer(this);

  // build up qt view, unless already done
  if (!m_Controls)
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::UltrasoundSupportControls;

    // create GUI widgets from the Qt Designer's .ui file
    m_Controls->setupUi(parent);

    //load persistence data before connecting slots (so no slots are called in this phase...)
    LoadUISettings();

    //connect signals and slots...
    connect(m_Controls->m_DeviceManagerWidget, SIGNAL(NewDeviceButtonClicked()), this, SLOT(OnClickedAddNewDevice())); // Change Widget Visibilities
    connect(m_Controls->m_DeviceManagerWidget, SIGNAL(NewDeviceButtonClicked()), this->m_Controls->m_NewVideoDeviceWidget, SLOT(CreateNewDevice())); // Init NewDeviceWidget
    connect(m_Controls->m_ActiveVideoDevices, SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)), this, SLOT(OnChangedActiveDevice()));
    connect(m_Controls->m_RunImageTimer, SIGNAL(clicked()), this, SLOT(OnChangedActiveDevice()));
    connect(m_Controls->m_ShowImageStream, SIGNAL(clicked()), this, SLOT(OnChangedActiveDevice()));
    connect(m_Controls->m_NewVideoDeviceWidget, SIGNAL(Finished()), this, SLOT(OnNewDeviceWidgetDone())); // After NewDeviceWidget finished editing
    connect(m_Controls->m_FrameRatePipeline, SIGNAL(valueChanged(int)), this, SLOT(OnChangedFramerateLimit()));
    connect(m_Controls->m_FrameRate2d, SIGNAL(valueChanged(int)), this, SLOT(OnChangedFramerateLimit()));
    connect(m_Controls->m_FrameRate3d, SIGNAL(valueChanged(int)), this, SLOT(OnChangedFramerateLimit()));
    connect(m_Controls->m_FreezeButton, SIGNAL(clicked()), this, SLOT(OnClickedFreezeButton()));
    connect(m_UpdateTimer, SIGNAL(timeout()), this, SLOT(UpdateImage()));
    connect(m_RenderingTimer2d, SIGNAL(timeout()), this, SLOT(RenderImage2d()));
    connect(m_RenderingTimer3d, SIGNAL(timeout()), this, SLOT(RenderImage3d()));
    connect(m_Controls->m_Update2DView, SIGNAL(clicked()), this, SLOT(StartTimers()));
    connect(m_Controls->m_Update3DView, SIGNAL(clicked()), this, SLOT(StartTimers()));
    connect(m_Controls->m_DeviceManagerWidget, SIGNAL(EditDeviceButtonClicked(mitk::USDevice::Pointer)), this, SLOT(OnClickedEditDevice())); //Change Widget Visibilities
    connect(m_Controls->m_DeviceManagerWidget, SIGNAL(EditDeviceButtonClicked(mitk::USDevice::Pointer)), this->m_Controls->m_NewVideoDeviceWidget, SLOT(EditDevice(mitk::USDevice::Pointer)));

    connect(m_Controls->m_SetXPoint1, SIGNAL(clicked()), this, SLOT(SetXPoint1()));
    connect(m_Controls->m_SetXPoint2, SIGNAL(clicked()), this, SLOT(SetXPoint2()));
    connect(m_Controls->m_SetYPoint1, SIGNAL(clicked()), this, SLOT(SetYPoint1()));
    connect(m_Controls->m_SetYPoint2, SIGNAL(clicked()), this, SLOT(SetYPoint2()));
    connect(m_Controls->m_SaveSpacing, SIGNAL(clicked()), this, SLOT(WriteSpacingToDevice()));


    // Initializations
    m_Controls->m_NewVideoDeviceWidget->setVisible(false);
    std::string filter = "(&(" + us::ServiceConstants::OBJECTCLASS() + "="
      + "org.mitk.services.UltrasoundDevice)("
      + mitk::USDevice::GetPropertyKeys().US_PROPKEY_ISACTIVE + "=true))";
    m_Controls->m_ActiveVideoDevices->Initialize<mitk::USDevice>(
      mitk::USDevice::GetPropertyKeys().US_PROPKEY_LABEL, filter);
    m_Controls->m_ActiveVideoDevices->SetAutomaticallySelectFirstEntry(true);
    m_FrameCounterPipeline = 0;
    m_FrameCounter2d = 0;
    m_FrameCounter3d = 0;

    m_Controls->tabWidget->setTabEnabled(1, false);
  }
}

void QmitkUltrasoundSupport::OnClickedAddNewDevice()
{
  m_Controls->m_NewVideoDeviceWidget->setVisible(true);
  m_Controls->m_DeviceManagerWidget->setVisible(false);
  m_Controls->m_Headline->setText("Add New Video Device:");
  m_Controls->m_WidgetActiveDevices->setVisible(false);
}

void QmitkUltrasoundSupport::OnClickedEditDevice()
{
  m_Controls->m_NewVideoDeviceWidget->setVisible(true);
  m_Controls->m_DeviceManagerWidget->setVisible(false);
  m_Controls->m_WidgetActiveDevices->setVisible(false);
  m_Controls->m_Headline->setText("Edit Video Device:");
}

void QmitkUltrasoundSupport::UpdateImage()
{
  //Update device
  m_Device->Modified();
  m_Device->Update();


  //Only update the view if the image is shown
  if (m_Controls->m_ShowImageStream->isChecked())
  {
    //Update data nodes
    for (size_t i = 0; i < m_AmountOfOutputs; i++)
    {
      mitk::Image::Pointer curOutput = m_Device->GetOutput(i);
      if (curOutput->IsEmpty())
      {
        m_Node.at(i)->SetName("No Data received yet ...");
        //create a noise image for correct initialization of level window, etc.
        mitk::Image::Pointer randomImage = mitk::ImageGenerator::GenerateRandomImage<float>(32, 32, 1, 1, 1, 1, 1, 255, 0);
        m_Node.at(i)->SetData(randomImage);
        curOutput->SetGeometry(randomImage->GetGeometry());
      }
      else
      {
        std::stringstream nodeName;
        nodeName << "US Viewing Stream - Image " << i;
        m_Node.at(i)->SetName(nodeName.str());
        m_Node.at(i)->SetData(curOutput);
        m_Node.at(i)->Modified();
      }
      // if the geometry changed: reinitialize the ultrasound image
      if ((i==0) && (m_OldGeometry.IsNotNull()) &&
        (curOutput->GetGeometry() != NULL) &&
        (!mitk::Equal(*(m_OldGeometry.GetPointer()), *(curOutput->GetGeometry()), 0.0001, false))
        )
      {
        mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();
        if ((renderWindow != NULL) && (curOutput->GetTimeGeometry()->IsValid()) && (m_Controls->m_ShowImageStream->isChecked()))
        {
          renderWindow->GetRenderingManager()->InitializeViews(
            curOutput->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
          renderWindow->GetRenderingManager()->RequestUpdateAll();
        }
        m_CurrentImageWidth = curOutput->GetDimension(0);
        m_CurrentImageHeight = curOutput->GetDimension(1);
        m_OldGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(curOutput->GetGeometry());
      }
    }
  }



  //Update frame counter
  m_FrameCounterPipeline++;
  if (m_FrameCounterPipeline >0)
  {
    //compute framerate of pipeline update
    int nMilliseconds = m_Clock.restart();
    int fps = 1000.0 / nMilliseconds;
    m_FPSPipeline = fps;
    m_FrameCounterPipeline = 0;

    //display lowest framerate in UI
    int lowestFPS = m_FPSPipeline;
    if (m_Controls->m_Update2DView->isChecked() && (m_FPS2d < lowestFPS)) { lowestFPS = m_FPS2d; }
    if (m_Controls->m_Update3DView->isChecked() && (m_FPS3d < lowestFPS)) { lowestFPS = m_FPS3d; }
    m_Controls->m_FramerateLabel->setText("Current Framerate: " + QString::number(lowestFPS) + " FPS");
  }
}

void QmitkUltrasoundSupport::RenderImage2d()
{
  if (!m_Controls->m_Update2DView->isChecked())
    return;

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);
  m_FrameCounter2d++;
  if (m_FrameCounter2d >0)
  {
    //compute framerate of 2d render window update
    int nMilliseconds = m_Clock2d.restart();
    int fps = 1000.0f / (nMilliseconds);
    m_FPS2d = fps;
    m_FrameCounter2d = 0;
  }
}

void QmitkUltrasoundSupport::RenderImage3d()
{
  if (!m_Controls->m_Update3DView->isChecked())
    return;

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
  m_FrameCounter3d++;
  if (m_FrameCounter3d >0)
  {
    //compute framerate of 2d render window update
    int nMilliseconds = m_Clock3d.restart();
    int fps = 1000.0f / (nMilliseconds);
    m_FPS3d = fps;
    m_FrameCounter3d = 0;
  }
}

void QmitkUltrasoundSupport::OnChangedFramerateLimit()
{
  StopTimers();
  int intervalPipeline = (1000 / m_Controls->m_FrameRatePipeline->value());
  int interval2D = (1000 / m_Controls->m_FrameRate2d->value());
  int interval3D = (1000 / m_Controls->m_FrameRate3d->value());
  SetTimerIntervals(intervalPipeline, interval2D, interval3D);
  StartTimers();
}

void QmitkUltrasoundSupport::OnClickedFreezeButton()
{
  if (m_Device.IsNull())
  {
    MITK_WARN("UltrasoundSupport") << "Freeze button clicked though no device is selected.";
    return;
  }
  if (m_Device->GetIsFreezed())
  {
    m_Device->SetIsFreezed(false);
    m_Controls->m_FreezeButton->setText("Freeze");
  }
  else
  {
    m_Device->SetIsFreezed(true);
    m_Controls->m_FreezeButton->setText("Start Viewing Again");
  }
}

void QmitkUltrasoundSupport::OnChangedActiveDevice()
{
  //clean up and stop timer
  StopTimers();
  this->RemoveControlWidgets();
  for (size_t j = 0; j < m_Node.size(); j++)
  {
    this->GetDataStorage()->Remove(m_Node.at(j));
    m_Node.at(j)->ReleaseData();
  }
  m_Node.clear();
  //get current device, abort if it is invalid
  m_Device = m_Controls->m_ActiveVideoDevices->GetSelectedService<mitk::USDevice>();
  if (m_Device.IsNull())
  {
    m_Controls->tabWidget->setTabEnabled(1, false);
    return;
  }
  m_AmountOfOutputs = m_Device->GetNumberOfIndexedOutputs();
  // clear data storage, create new nodes and add
  for (size_t i = 0; i < m_AmountOfOutputs; i++)
  {
    mitk::DataNode::Pointer currentNode = mitk::DataNode::New();
    std::stringstream nodeName;
    nodeName << "US Viewing Stream - Image " << i;
    currentNode->SetName(nodeName.str());
    //create a dummy image (gray values 0..255) for correct initialization of level window, etc.
    mitk::Image::Pointer dummyImage = mitk::ImageGenerator::GenerateRandomImage<float>(100, 100, 1, 1, 1, 1, 1, 255, 0);
    currentNode->SetData(dummyImage);
    m_OldGeometry = dynamic_cast<mitk::SlicedGeometry3D*>(dummyImage->GetGeometry());
    m_Node.push_back(currentNode);
    //show node if the option is enabled
    if (m_Controls->m_ShowImageStream->isChecked())
    {
        this->GetDataStorage()->Add(m_Node.at(i));
    }
  }

  //create the widgets for this device and enable the widget tab
  this->CreateControlWidgets();
  m_Controls->tabWidget->setTabEnabled(1, true);

  //start timer
  if (m_Controls->m_RunImageTimer->isChecked())
  {
    int intervalPipeline = (1000 / m_Controls->m_FrameRatePipeline->value());
    int interval2D = (1000 / m_Controls->m_FrameRate2d->value());
    int interval3D = (1000 / m_Controls->m_FrameRate3d->value());
    SetTimerIntervals(intervalPipeline, interval2D, interval3D);
    StartTimers();
    m_Controls->m_TimerWidget->setEnabled(true);
  }
  else
  {
    m_Controls->m_TimerWidget->setEnabled(false);
  }
}

void QmitkUltrasoundSupport::OnNewDeviceWidgetDone()
{
  m_Controls->m_NewVideoDeviceWidget->setVisible(false);
  m_Controls->m_DeviceManagerWidget->setVisible(true);
  m_Controls->m_Headline->setText("Ultrasound Devices:");
  m_Controls->m_WidgetActiveDevices->setVisible(true);
}

void QmitkUltrasoundSupport::CreateControlWidgets()
{
  m_ControlProbesWidget = new QmitkUSControlsProbesWidget(m_Device->GetControlInterfaceProbes(), m_Controls->m_ToolBoxControlWidgets);
  m_Controls->probesWidgetContainer->addWidget(m_ControlProbesWidget);

  // create b mode widget for current device
  m_ControlBModeWidget = new QmitkUSControlsBModeWidget(m_Device->GetControlInterfaceBMode(), m_Controls->m_ToolBoxControlWidgets);
  m_Controls->m_ToolBoxControlWidgets->addItem(m_ControlBModeWidget, "B Mode Controls");
  if (!m_Device->GetControlInterfaceBMode())
  {
    m_Controls->m_ToolBoxControlWidgets->setItemEnabled(m_Controls->m_ToolBoxControlWidgets->count() - 1, false);
  }

  // create doppler widget for current device
  m_ControlDopplerWidget = new QmitkUSControlsDopplerWidget(m_Device->GetControlInterfaceDoppler(), m_Controls->m_ToolBoxControlWidgets);
  m_Controls->m_ToolBoxControlWidgets->addItem(m_ControlDopplerWidget, "Doppler Controls");
  if (!m_Device->GetControlInterfaceDoppler())
  {
    m_Controls->m_ToolBoxControlWidgets->setItemEnabled(m_Controls->m_ToolBoxControlWidgets->count() - 1, false);
  }

  ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();
  if (pluginContext)
  {
    std::string filter = "(org.mitk.services.UltrasoundCustomWidget.deviceClass=" + m_Device->GetDeviceClass() + ")";
    //Hint: The following three lines are a workaround. Till now the only US video device was an USVideoDevice.
    // And everything worked fine. However, the ultrasound image source can be an USIGTLDevice (IGTL Client), as well.
    // This second option wasn't considered yet. So, the custom control widget will work correctly only, if
    // the filter declares the device class as org.mitk.modules.us.USVideoDevice. Another option, how to deal with
    // the two possible ultrasound image devices would be to change the returned string of the method
    // std::string QmitkUSControlsCustomVideoDeviceWidget::GetDeviceClass(), which always returns the string
    // org.mitk.modules.us.USVideoDevice of the USVideoDevice class. If there is a possility to change the
    // returned string dynamically between "IGTL Client" and "org.mitk.modules.us.USVideoDevice" the following
    // three lines will not be needed.
    if (m_Device->GetDeviceClass().compare("IGTL Client") == 0)
    {
      filter = "(org.mitk.services.UltrasoundCustomWidget.deviceClass=" + mitk::USVideoDevice::GetDeviceClassStatic() + ")";
    }

    QString interfaceName = QString::fromStdString(us_service_interface_iid<QmitkUSAbstractCustomWidget>());
    m_CustomWidgetServiceReference = pluginContext->getServiceReferences(interfaceName, QString::fromStdString(filter));

    if (m_CustomWidgetServiceReference.size() > 0)
    {
      m_ControlCustomWidget = pluginContext->getService<QmitkUSAbstractCustomWidget>
        (m_CustomWidgetServiceReference.at(0))->CloneForQt(m_Controls->tab2);
      m_ControlCustomWidget->SetDevice(m_Device);
      m_Controls->m_ToolBoxControlWidgets->addItem(m_ControlCustomWidget, "Custom Controls");
    }
    else
    {
      m_Controls->m_ToolBoxControlWidgets->addItem(new QWidget(m_Controls->m_ToolBoxControlWidgets), "Custom Controls");
      m_Controls->m_ToolBoxControlWidgets->setItemEnabled(m_Controls->m_ToolBoxControlWidgets->count() - 1, false);
    }
  }

  // select first enabled control widget
  for (int n = 0; n < m_Controls->m_ToolBoxControlWidgets->count(); ++n)
  {
    if (m_Controls->m_ToolBoxControlWidgets->isItemEnabled(n))
    {
      m_Controls->m_ToolBoxControlWidgets->setCurrentIndex(n);
      break;
    }
  }
}

void QmitkUltrasoundSupport::RemoveControlWidgets()
{
  if (!m_ControlProbesWidget) { return; } //widgets do not exist... nothing to do

  // remove all control widgets from the tool box widget
  while (m_Controls->m_ToolBoxControlWidgets->count() > 0)
  {
    m_Controls->m_ToolBoxControlWidgets->removeItem(0);
  }

  // remove probes widget (which is not part of the tool box widget)
  m_Controls->probesWidgetContainer->removeWidget(m_ControlProbesWidget);
  delete m_ControlProbesWidget;
  m_ControlProbesWidget = 0;

  delete m_ControlBModeWidget;
  m_ControlBModeWidget = 0;

  delete m_ControlDopplerWidget;
  m_ControlDopplerWidget = 0;

  // delete custom widget if it is present
  if (m_ControlCustomWidget)
  {
    ctkPluginContext* pluginContext = mitk::PluginActivator::GetContext();
    delete m_ControlCustomWidget; m_ControlCustomWidget = 0;
    if (m_CustomWidgetServiceReference.size() > 0)
    {
      pluginContext->ungetService(m_CustomWidgetServiceReference.at(0));
    }
  }
}

void QmitkUltrasoundSupport::OnDeviceServiceEvent(const ctkServiceEvent event)
{
  if (m_Device.IsNull() || event.getType() != ctkServiceEvent::MODIFIED)
  {
    return;
  }

  ctkServiceReference service = event.getServiceReference();

  if (m_Device->GetManufacturer() != service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_MANUFACTURER)).toString().toStdString()
    && m_Device->GetName() != service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_NAME)).toString().toStdString())
  {
    return;
  }

  if (!m_Device->GetIsActive() && m_UpdateTimer->isActive())
  {
    StopTimers();
  }

  if (m_CurrentDynamicRange != service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DYNAMIC_RANGE)).toDouble())
  {
    m_CurrentDynamicRange = service.getProperty(QString::fromStdString(mitk::USDevice::GetPropertyKeys().US_PROPKEY_BMODE_DYNAMIC_RANGE)).toDouble();

    // update level window for the current dynamic range
    mitk::LevelWindow levelWindow;
    m_Node.at(0)->GetLevelWindow(levelWindow);
    levelWindow.SetAuto(m_curOutput.at(0), true, true);
    m_Node.at(0)->SetLevelWindow(levelWindow);
  }
}

void QmitkUltrasoundSupport::StoreUISettings()
{
  QSettings settings;
  settings.beginGroup(QString::fromStdString(VIEW_ID));
  settings.setValue("DisplayImage", QVariant(m_Controls->m_ShowImageStream->isChecked()));
  settings.setValue("RunImageTimer", QVariant(m_Controls->m_RunImageTimer->isChecked()));
  settings.setValue("Update2DView", QVariant(m_Controls->m_Update2DView->isChecked()));
  settings.setValue("Update3DView", QVariant(m_Controls->m_Update3DView->isChecked()));
  settings.setValue("UpdateRatePipeline", QVariant(m_Controls->m_FrameRatePipeline->value()));
  settings.setValue("UpdateRate2d", QVariant(m_Controls->m_FrameRate2d->value()));
  settings.setValue("UpdateRate3d", QVariant(m_Controls->m_FrameRate3d->value()));
  settings.endGroup();
}

void QmitkUltrasoundSupport::LoadUISettings()
{
  QSettings settings;
  settings.beginGroup(QString::fromStdString(VIEW_ID));
  m_Controls->m_ShowImageStream->setChecked(settings.value("DisplayImage", true).toBool());
  m_Controls->m_RunImageTimer->setChecked(settings.value("RunImageTimer", true).toBool());
  m_Controls->m_Update2DView->setChecked(settings.value("Update2DView", true).toBool());
  m_Controls->m_Update3DView->setChecked(settings.value("Update3DView", true).toBool());
  m_Controls->m_FrameRatePipeline->setValue(settings.value("UpdateRatePipeline", 50).toInt());
  m_Controls->m_FrameRate2d->setValue(settings.value("UpdateRate2d", 20).toInt());
  m_Controls->m_FrameRate3d->setValue(settings.value("UpdateRate3d", 5).toInt());
  settings.endGroup();
}

void QmitkUltrasoundSupport::StartTimers()
{
  m_Clock.start();
  m_UpdateTimer->start();
  if (m_Controls->m_Update2DView->isChecked()) { m_RenderingTimer2d->start(); }
  if (m_Controls->m_Update3DView->isChecked()) { m_RenderingTimer3d->start(); }
}

void QmitkUltrasoundSupport::StopTimers()
{
  m_UpdateTimer->stop();
  m_RenderingTimer2d->stop();
  m_RenderingTimer3d->stop();
}

void QmitkUltrasoundSupport::SetTimerIntervals(int intervalPipeline, int interval2D, int interval3D)
{
  m_UpdateTimer->setInterval(intervalPipeline);
  m_RenderingTimer2d->setInterval(interval2D);
  m_RenderingTimer3d->setInterval(interval3D);
}


/* Spacing methods */
void QmitkUltrasoundSupport::SetXPoint1()
{
  m_Xpoint1 = this->GetRenderWindowPart()->GetSelectedPosition();
  m_XSpacing = ComputeSpacing(m_Xpoint1, m_Xpoint2, m_Controls->m_XDistance->value());
  m_Controls->m_XSpacing->setText(QString("X Spacing: ") + QString::number(m_XSpacing) + " mm");
}
void QmitkUltrasoundSupport::SetXPoint2()
{
  m_Xpoint2 = this->GetRenderWindowPart()->GetSelectedPosition();
  m_XSpacing = ComputeSpacing(m_Xpoint1, m_Xpoint2, m_Controls->m_XDistance->value());
  m_Controls->m_XSpacing->setText(QString("X Spacing: ") + QString::number(m_XSpacing) + " mm");
}
void QmitkUltrasoundSupport::SetYPoint1()
{
  m_Ypoint1 = this->GetRenderWindowPart()->GetSelectedPosition();
  m_YSpacing = ComputeSpacing(m_Ypoint1, m_Ypoint2, m_Controls->m_YDistance->value());
  m_Controls->m_YSpacing->setText(QString("Y Spacing: ") + QString::number(m_YSpacing) + " mm");
}
void QmitkUltrasoundSupport::SetYPoint2()
{
  m_Ypoint2 = this->GetRenderWindowPart()->GetSelectedPosition();
  m_YSpacing = ComputeSpacing(m_Ypoint1, m_Ypoint2, m_Controls->m_YDistance->value());
  m_Controls->m_YSpacing->setText(QString("Y Spacing: ") + QString::number(m_YSpacing) + " mm");
}
void QmitkUltrasoundSupport::WriteSpacingToDevice()
{
  this->m_Device->SetSpacing(m_XSpacing, m_YSpacing);
  MITK_INFO << "Spacing saved to device object, please save device data to permanently store the spacing.";
}
double QmitkUltrasoundSupport::ComputeSpacing(mitk::Point3D p1, mitk::Point3D p2, double distance)
{
  double spacing = 0;
  double pointDistance = p1.EuclideanDistanceTo(p2);
  spacing = distance / pointDistance;
  return spacing;
}

