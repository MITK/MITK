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

#if !defined(QMITK_ScreenshotMaker_H__INCLUDED)
#define QMITK_ScreenshotMaker_H__INCLUDED

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include "mitkCameraRotationController.h"
#include "mitkStepper.h"
#include "mitkMultiStepper.h"
#include "mitkMovieGenerator.h"
#include "itkCommand.h"

#include "vtkEventQtSlotConnect.h"
#include "vtkRenderWindow.h"
#include "mitkVtkPropRenderer.h"

#include "ui_QmitkScreenshotMakerControls.h"
//#include "../MovieMakerDll.h"

//class QmitkMovieMakerControls;
class QmitkStepperAdapter;
class vtkCamera;
class QTimer;
class QTime;

/**
 * \brief View for creating movies (AVIs)
 */
class QmitkScreenshotMaker: public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  /** \brief Constructor. */
  QmitkScreenshotMaker(QObject *parent=0, const char *name=0);

  /** \brief Destructor. */
  virtual ~QmitkScreenshotMaker();

  /** \brief Method for creating the widget containing the application
   * controls, like sliders, buttons etc.
   */
  virtual void CreateQtPartControl(QWidget *parent) override;
  //  virtual QWidget * CreateControlWidget(QWidget *parent);

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

  /** \brief Method for creating the connections of main and control widget.
   */
  virtual void CreateConnections();

  /** \brief Method for creating an QAction object, i.e. button & menu entry.
   * @param parent the parent QWidget
   */
  //  virtual QAction * CreateAction(QActionGroup *parent);

  ///
  /// Called when a RenderWindowPart becomes available.
  ///
  virtual void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  ///
  /// Called when a RenderWindowPart becomes unavailable.
  ///
  virtual void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  signals:

protected slots:

  void GenerateScreenshot();
  void GenerateMultiplanarScreenshots();
  void Generate3DHighresScreenshot();
  void GenerateMultiplanar3DHighresScreenshot();
  void SelectBackgroundColor();

protected:

  QObject *parentWidget;
  QWidget* m_Parent;
  vtkEventQtSlotConnect * connections;
  vtkRenderWindow * renderWindow;
  mitk::VtkPropRenderer::Pointer m_PropRenderer;

  Ui::QmitkScreenshotMakerControls* m_Controls;

private:

  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  vtkCamera* GetCam();
  void GenerateHR3DAtlasScreenshots(QString fileName, QString filter = "");
  void GenerateMultiplanarScreenshots(QString fileName);

  mitk::DataNode::Pointer GetTopLayerNode();
  void MultichannelScreenshot(mitk::VtkPropRenderer* renderer, QString fileName, QString filter);

  /*!
  \brief taking a screenshot "from" the specified renderer
  \param magnificationFactor specifying the quality of the screenshot (the magnification of the actual RenderWindow size)
  \param fileName file location and name where the screenshot should be saved
  */
  void TakeScreenshot(vtkRenderer* renderer, unsigned int magnificationFactor, QString fileName, QString filter = "");

  QColor m_BackgroundColor;

  mitk::DataNode* m_SelectedNode;
  QString           m_LastPath;
  QString           m_LastFile;
  QString           m_PNGExtension = "PNG File (*.png)";
  QString           m_JPGExtension = "JPEG File (*.jpg)";
};
#endif // !defined(QMITK_ScreenshotMaker_H__INCLUDED)

