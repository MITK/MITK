/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(QMITK_MOVIEMAKER_H__INCLUDED)
#define QMITK_MOVIEMAKER_H__INCLUDED

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

class QmitkStdMultiWidget;
class QmitkMovieMakerControls;
class QmitkStepperAdapter;
class vtkCamera;
class QTimer;
class QTime;

/**
 * \brief Functionality for creating movies (AVIs)
 * \ingroup Functionalities
 * 
 * 
 * - \ref QmitkMovieMakerOverview
 * - \ref QmitkMovieMakerImplementation
 * - \ref QmitkMovieMakerTodo
 * 
 * \section QmitkMovieMakerOverview Overview
 * 
 * The MovieMaker functionality extends existing modes of data visualization
 * with animation capabilities. The animation aspect determines which
 * aspect of a visualization gets animated. This aspect can be spatial
 * (i.e. the camera position and orientation in a 3D view, or the selected
 * slice in a 2D view), temporal (the time step in a time-based dataset), or
 * something more sophisticated (for example, volume rendering transfer
 * functions might be changed dynamically as part of an animation to produce
 * interesting effects). Currently, the following animation modes are
 * supported:
 * 
 * - Rotation of the camera around the dataset (3D views)
 * - Stepping through the dataset slice by slice (2D views)
 * - Stepping through a time-based dataset (2D and 3D views)
 * 
 * As can be seen in the screenshot below, the functionality provides cine
 * controls to start, pause and stop the animation. The direction of the
 * animation can be selected (forward, backward or ping-pong). The animation
 * can be set to either the spatial or the temporal aspect, or to both
 * aspects combined. The cycle time determines the duration of one animation
 * loop.
 * 
 * In a multi-view scenario, the animation is applied to the currently
 * focussed view. Other views which display the same data may be affected as
 * well.
 * 
 * Animations can be written on disk; the corresponding button creates an
 * animation consisting of one single loop and writes every animation frame
 * into an AVI file.
 * 
 * \image html QmitkMovieMakerGUI.png
 * 
 * \section QmitkMovieMakerImplementation Implementation
 * 
 * The following diagram provides an overview over the classes collaborating
 * with QmitkMovieMaker.
 * 
 * \image html QmitkMovieMakerClasses.png
 * 
 * The respective roles of these classes are:
 * - QmitkMovieMaker
 *   - Provides a class interface for configuring and controlling the
 *     animation
 *   - Is associated with the user interface
 *   - As a Functionality, controls initialization and maintainance of the
 *     involved components
 * - mitk::BaseController
 *   - Controls an animation aspect (slice selection, camera angle, ...)
 *   - Subclass mitk::SliceNavigationController: Transfers the animation
 *     requests to the associated Renderer for selecting a slice in a 3D
 *     dataset
 *   - Subclass mitk::CameraRotationController: Modifies the camera angle
 * - mitk::Stepper
 *   - Link between MovieMaker and Controller
 *   - Holds the current step position of the associated animation aspect
 *     (e.g. the camera rotation angle, or the slice number)
 * - mitk::FocusManager
 *   - Determines which of the available RenderWindows (and thereby which
 *     Renderers) the animation shall be applied to
 * - mitk::BaseRenderer
 *   - Controls the rendering of a dataset
 *   - Target class of the animation
 * - mitk::MovieGenerator
 *   - Writes an AVI file
 * 
 * 
 * \section QmitkMovieMakerTodo Future plans
 * 
 * The basic animation capabilities of this functionality could be extended
 * to allow the creation of more sophisticated animations. Potentially useful
 * features include:
 * 
 * - <em>Camera paths</em>: Allow the definition of complex camera paths along
 *   which the camera can then be moved during an animation. This might be
 *   done by defining camera keyframes between which camera movements are
 *   interpolated (as in standard 3D animation applications). Paths derived
 *   from anatomical features (e.g. vessel graphs) could also serve as a
 *   basis for camera movements.
 * - <em>Color / transfer function animation</em>: Choosing suitable color
 *   and transfer functions is critical to the quality and benefit of volume
 *   rendering. Dynamically changing transfer functions could visually enrich
 *   an animation. One way of achieving this could be to define transfer
 *   function keyframes between which an animation interpolates.
 * 
 */
class QmitkMovieMaker : public QmitkFunctionality
{  
  Q_OBJECT
  
public:  
  /** \brief Constructor. */
  QmitkMovieMaker(QObject *parent=0, const char *name=0,
    QmitkStdMultiWidget *mitkStdMultiWidget = NULL,
    mitk::DataTreeIteratorBase* dataIt = NULL);

  /** \brief Destructor. */  
  virtual ~QmitkMovieMaker();

  /** \brief Method for creating the widget containing the application 
   * controls, like sliders, buttons etc.  
   */  
  virtual QWidget * CreateControlWidget(QWidget *parent);

  /** \brief Method for creating the applications main widget. */  
  virtual QWidget * CreateMainWidget(QWidget * parent);

  /** \brief Method for creating the connections of main and control widget.
   */  
  virtual void CreateConnections();

  /** \brief Method for creating an QAction object, i.e. button & menu entry.
   * @param parent the parent QWidget
   */  
  virtual QAction * CreateAction(QActionGroup *parent);

  virtual void Activated();
  
  virtual void Deactivated();

  /** \brief Called when another window receives the focus. */
  void FocusChange();

signals:
  void StartBlockControls();
  void EndBlockControls();
  void EndBlockControlsMovieDeactive();

public slots:
   
  /** \brief Start playing the animation by restarting the timer. */
  void StartPlaying();

  /** \brief Pauses playing the animation by stopping the timer. */
  void PausePlaying();

  /** \brief Stops playing the animation and resets the stepper. */
  void StopPlaying();

  /** \brief Sets animation looping ON/OFF. */
  void SetLooping( bool looping );

  /** \brief Sets the direction: 0 = forward, 1 = backward, 2 = pingpong. */
  void SetDirection( int direction );

  /** \brief Sets the animation aspect:
   * 0 = spatial, 1 = temporal, 2 = combined.
   */
  void SetAspect( int aspect );

  /** \brief Sets a specified stepper window, which is moving. */
  void SetStepperWindow( int window );

  /** \brief Sets a specified recording window, from which the movie is generated. */
  void SetRecordingWindow( int window );

  /** \brief Advances the animation by one frame.
   * Exactly how much the stepper advances depends on the time elapsed since
   * the last call to this function.
   */
  void AdvanceAnimation();
  

protected slots:  

  void RenderSlot();
  void GenerateMovie();
  
  void GenerateScreenshot();


protected:  

  QObject *parentWidget;
  QVTKWidget * widget;
  vtkEventQtSlotConnect * connections;
  vtkRenderWindow * renderWindow;
  mitk::VtkPropRenderer::Pointer m_PropRenderer;
  
  QmitkMovieMakerControls* m_Controls;
  
  QmitkStdMultiWidget* m_MultiWidget;

private:
  mitk::BaseController* GetSpatialController();

  mitk::BaseController* GetTemporalController();

  void UpdateLooping();
  
  void UpdateDirection();
  
  mitk::Stepper* GetAspectStepper();

  QmitkStepperAdapter* m_StepperAdapter;

  typedef itk::SimpleMemberCommand< QmitkMovieMaker > MemberCommand;
  MemberCommand::Pointer m_FocusManagerCallback;

  QTimer* m_Timer;

  QTime* m_Time;

  bool m_Looping;
  int m_Direction;

  int m_Aspect;
  mitk::MultiStepper::Pointer m_Stepper;
  mitk::BaseRenderer * m_RecordingRenderer;

  mitk::MovieGenerator::Pointer m_movieGenerator;

  unsigned long m_FocusManagerObserverTag;

};
#endif // !defined(QMITK_MOVIEMAKER_H__INCLUDED)

