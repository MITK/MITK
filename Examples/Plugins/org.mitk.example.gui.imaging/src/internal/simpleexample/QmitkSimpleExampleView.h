/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSimpleExampleView_h
#define QmitkSimpleExampleView_h

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

class QmitkStepperAdapter;

class vtkRenderer;

namespace Ui
{
  class QmitkSimpleExampleViewControls;
}

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
  ~QmitkSimpleExampleView() override;

private:
  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  void RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart) override;

  /**
   * return the renderwindow of which the movie shall be created, what depends on the combo box
   */
  QmitkRenderWindow *GetSelectedRenderWindow() const;

  /// writes a screenshot in JPEG or PNG format to the file fileName
  void TakeScreenshot(vtkRenderer *renderer, unsigned int magnificationFactor, QString fileName, QString filter = "");

private slots:

  /**
   * Qt slot for reacting on the selected render window from the combo box
   */
  void RenderWindowSelected(const QString &id);

  /**
   * qt slot for event processing from a qt widget defining the stereo mode of a render window
   */
  void StereoSelectionChanged(int id);

  /**
   * initialize the slice and temporal sliders according to the image dimensions
   */
  void InitNavigators();

  /// takes screenshot of the 3D window in 4x resolution of the render window
  void OnTakeHighResolutionScreenshot();

  /// takes screenshot of the selected render window
  void OnTakeScreenshot();

private:
  Ui::QmitkSimpleExampleViewControls *m_Controls;

  bool m_NavigatorsInitialized;

  QScopedPointer<QmitkStepperAdapter> m_SliceStepper;
  QScopedPointer<QmitkStepperAdapter> m_TimeStepper;
  QScopedPointer<QmitkStepperAdapter> m_MovieStepper;

  QWidget *m_Parent;

  QString m_PNGExtension = "PNG File (*.png)";
  QString m_JPGExtension = "JPEG File (*.jpg)";
};
#endif
