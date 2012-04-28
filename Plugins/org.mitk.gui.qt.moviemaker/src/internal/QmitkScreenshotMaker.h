/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 15436 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#if !defined(QMITK_ScreenshotMaker_H__INCLUDED)
#define QMITK_ScreenshotMaker_H__INCLUDED

#include "QmitkFunctionality.h"
#include "mitkCameraRotationController.h"
#include "mitkStepper.h"
#include "mitkMultiStepper.h"
#include "mitkMovieGenerator.h"
#include "itkCommand.h"

#include "QVTKWidget.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkRenderWindow.h"
#include "mitkVtkPropRenderer.h"

#include "ui_QmitkScreenshotMakerControls.h"
//#include "../MovieMakerDll.h"

//class QmitkStdMultiWidget;
//class QmitkMovieMakerControls;
class QmitkStepperAdapter;
class vtkCamera;
class QTimer;
class QTime;

/**
 * \brief Functionality for creating movies (AVIs)
 * \ingroup Functionalities
 */
class QmitkScreenshotMaker: public QmitkFunctionality
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
  void CreateQtPartControl(QWidget *parent);
  //  virtual QWidget * CreateControlWidget(QWidget *parent);

  /** \brief Method for creating the connections of main and control widget.
   */
  virtual void CreateConnections();

  /** \brief Method for creating an QAction object, i.e. button & menu entry.
   * @param parent the parent QWidget
   */
  //  virtual QAction * CreateAction(QActionGroup *parent);

  virtual void Activated();

  virtual void Deactivated();

  ///
  /// Called when a StdMultiWidget is available.
  ///
  virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
  ///
  /// Called when no StdMultiWidget is available.
  ///
  virtual void StdMultiWidgetNotAvailable();

  signals:

protected slots:

  void GenerateScreenshot();
  void GenerateMultiplanarScreenshots();
  void Generate3DHighresScreenshot();
  void GenerateMultiplanar3DHighresScreenshot();
  void View1();
  void View2();
  void View3();
  void SelectBackgroundColor();

protected:

  QObject *parentWidget;
  QVTKWidget * widget;
  QmitkStdMultiWidget* m_MultiWidget;
  vtkEventQtSlotConnect * connections;
  vtkRenderWindow * renderWindow;
  mitk::VtkPropRenderer::Pointer m_PropRenderer;

  Ui::QmitkScreenshotMakerControls* m_Controls;

private:

  void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  vtkCamera* GetCam();
  void GenerateHR3DAtlasScreenshots(QString fileName);
  void GenerateMultiplanarScreenshots(QString fileName);

  /*!
  \brief taking a screenshot "from" the specified renderer
  \param magnificationFactor specifying the quality of the screenshot (the magnification of the actual RenderWindow size)
  \param fileName file location and name where the screenshot should be saved
  */
  void TakeScreenshot(vtkRenderer* renderer, unsigned int magnificationFactor, QString fileName);

  QColor m_BackgroundColor;

  mitk::DataNode* m_SelectedNode;
};
#endif // !defined(QMITK_ScreenshotMaker_H__INCLUDED)

