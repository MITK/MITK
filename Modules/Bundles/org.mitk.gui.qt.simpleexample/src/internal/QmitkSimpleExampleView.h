/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17332 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKSIMPLEEXAMPLEVIEW_H_INCLUDED
#define _QMITKSIMPLEEXAMPLEVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkSimpleExampleViewControls.h"

#include <QmitkRenderWindow.h>

/*!
 * \ingroup org_mitk_gui_qt_simpleexample_internal
 *
 * \brief QmitkSimpleExampleView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkSimpleExampleView : public QObject, public QmitkFunctionality
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkSimpleExampleView();
  virtual ~QmitkSimpleExampleView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

protected slots:

  /*!
  qt slot for event processing from a qt widget defining the stereo mode of widget 4
  */
  void stereoSelectionChanged(int id);

  /*!
  initialize the transversal, sagittal, coronal and temporal slider according to the image dimensions
  */
  void initNavigators();

  /*!
  generate a movie as *.avi from the active render window
  */
  void generateMovie();

  /*!
  return the renderwindow of which the movie shall be created, what depends on the toggled button
  */
  QmitkRenderWindow* GetMovieRenderWindow();

  void OnRenderWindow1Clicked();
  void OnRenderWindow2Clicked();
  void OnRenderWindow3Clicked();
  void OnRenderWindow4Clicked();
  void OnTakeHighResolutionScreenshot(); ///< takes screenshot of the 3D window in 4x resolution of the render window
  void OnTakeScreenshot(); ///< takes screenshot of the selected render window

  void OnTransversalRotationChanged( int angle );
  void OnCoronalRotationChanged( int angle );
  void OnSagittalRotationChanged( int angle );
 
protected:

  Ui::QmitkSimpleExampleViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  void TakeScreenshot(vtkRenderer* renderer, unsigned int magnificationFactor, QString fileName); ///< writes a screenshot in JPEG or PNG format to the file fileName

  bool m_NavigatorsInitialized;
};
#endif // _QMITKSIMPLEEXAMPLEVIEW_H_INCLUDED
