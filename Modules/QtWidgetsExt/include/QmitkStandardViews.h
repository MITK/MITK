/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkStandardViews_h
#define QmitkStandardViews_h

#include <MitkQtWidgetsExtExports.h>
#include <mitkCameraController.h>
#include <mitkCommon.h>

#include <QWidget>

class QClickableLabel;
class vtkRenderWindow;

/**
 * \brief Widget displaying a clickable image for selecting standard anatomical view directions.
 *
 * Shows a small picture with labeled hotspots (Left, Right, Top, Bottom, Front, Back).
 * Clicking a hotspot sets the corresponding standard view on the assigned
 * mitk::CameraController and emits the StandardViewDefined signal.
 *
 * \sa mitk::CameraController, QClickableLabel
 */
class MITKQTWIDGETSEXT_EXPORT QmitkStandardViews : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Construct the standard views widget.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QmitkStandardViews(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  /** \brief Destructor. */
  ~QmitkStandardViews() override;

  /**
   * \brief Set the camera controller to apply standard views to.
   * \param[in] controller The mitk::CameraController instance.
   */
  void SetCameraController(mitk::CameraController *controller);

  /**
   * \brief Set the camera controller by looking it up from a render window.
   * \param[in] window The vtkRenderWindow whose camera controller to use.
   */
  void SetCameraControllerFromRenderWindow(vtkRenderWindow *window);

signals:
  /**
   * \brief Emitted when a standard view hotspot is clicked.
   * \param[in] view The selected standard view direction.
   */
  void StandardViewDefined(mitk::CameraController::StandardView view);

protected slots:

  void hotspotClicked(const QString &s);

protected:
  QClickableLabel *m_ClickablePicture;

  mitk::CameraController::Pointer m_CameraController;
};

#endif
