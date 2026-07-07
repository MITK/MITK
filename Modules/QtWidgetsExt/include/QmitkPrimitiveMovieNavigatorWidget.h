/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPrimitiveMovieNavigatorWidget_h
#define QmitkPrimitiveMovieNavigatorWidget_h

#include <MitkQtWidgetsExtExports.h>

#include <mitkStepper.h>

#include <QTimer>
#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkPrimitiveMovieNavigator;
}

/**
 * \brief Simple widget for movie-style navigation through time steps or slices.
 *
 * Provides play/stop buttons, a spin box for direct step selection, and a
 * configurable timer interval. Drives a mitk::Stepper to iterate through
 * steps in a loop.
 *
 * \sa mitk::Stepper
 */
class MITKQTWIDGETSEXT_EXPORT QmitkPrimitiveMovieNavigatorWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Construct the movie navigator widget.
   * \param[in] parent The parent widget.
   * \param[in] fl Window flags.
   */
  QmitkPrimitiveMovieNavigatorWidget(QWidget *parent = nullptr, Qt::WindowFlags fl = {});

  /** \brief Destructor. */
  ~QmitkPrimitiveMovieNavigatorWidget() override;

  /**
   * \brief Get the current timer interval.
   * \return The interval in milliseconds.
   */
  virtual int getTimerInterval();

public slots:
  /** \brief Refresh the spin box range and value from the stepper. */
  virtual void Refetch();

  /**
   * \brief Set the stepper to drive.
   * \param[in] stepper The mitk::Stepper to navigate. May be nullptr.
   */
  virtual void SetStepper(mitk::Stepper *stepper);

  /** \brief Start the movie playback (looping through steps). */
  virtual void goButton_clicked();

  /** \brief Stop the movie playback. */
  virtual void stopButton_clicked();

  /**
   * \brief Handle spin box value changes by updating the stepper position.
   * \param[in] value The new spin box value.
   */
  virtual void spinBoxValueChanged(int value);

  /**
   * \brief Set the timer interval for playback speed.
   * \param[in] timerIntervalInMS The interval in milliseconds.
   */
  virtual void setTimerInterval(int timerIntervalInMS);

protected:
  std::unique_ptr<Ui::QmitkPrimitiveMovieNavigator> m_Controls;
  mitk::Stepper::Pointer m_Stepper;
  bool m_InRefetch;
  QTimer *m_Timer;
  int m_TimerIntervalInMS;

private slots:
  virtual void next();
};

#endif
