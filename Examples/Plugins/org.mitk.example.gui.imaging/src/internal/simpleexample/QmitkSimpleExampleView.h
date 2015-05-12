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

#ifndef _QMITKSIMPLEEXAMPLEVIEW_H_INCLUDED
#define _QMITKSIMPLEEXAMPLEVIEW_H_INCLUDED

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

#include "ui_QmitkSimpleExampleViewControls.h"

class QmitkStepperAdapter;

class vtkRenderer;

/**
 * \brief QmitkSimpleExampleView
 *
 * \sa QmitkAbstractView
 */
class QmitkSimpleExampleView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkSimpleExampleView();
  ~QmitkSimpleExampleView();

private:

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  virtual void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  virtual void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;

  /**
   * return the renderwindow of which the movie shall be created, what depends on the combo box
   */
  QmitkRenderWindow* GetSelectedRenderWindow() const;

  /// writes a screenshot in JPEG or PNG format to the file fileName
  void TakeScreenshot(vtkRenderer* renderer, unsigned int magnificationFactor, QString fileName);

private slots:

  /**
   * Qt slot for reacting on the selected render window from the combo box
   */
  void RenderWindowSelected(const QString& id);

  /**
   * qt slot for event processing from a qt widget defining the stereo mode of a render window
   */
  void StereoSelectionChanged(int id);

  /**
   * initialize the slice and temporal sliders according to the image dimensions
   */
  void InitNavigators();

  /**
   * generate a movie as *.avi from the active render window
   */
  void GenerateMovie();

  /// takes screenshot of the 3D window in 4x resolution of the render window
  void OnTakeHighResolutionScreenshot();

  /// takes screenshot of the selected render window
  void OnTakeScreenshot();

private:

  Ui::QmitkSimpleExampleViewControls* m_Controls;

  bool m_NavigatorsInitialized;

  QScopedPointer<QmitkStepperAdapter> m_SliceStepper;
  QScopedPointer<QmitkStepperAdapter> m_TimeStepper;
  QScopedPointer<QmitkStepperAdapter> m_MovieStepper;

  QWidget* m_Parent;
};
#endif // _QMITKSIMPLEEXAMPLEVIEW_H_INCLUDED
