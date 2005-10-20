/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include "itkCommand.h"

class QmitkStdMultiWidget;
class QmitkMovieMakerControls;
class QmitkStepperAdapter;
class vtkCamera;
class QTimer;
class QTime;

/*!
\brief Functionality for creating movies (AVIs)
\ingroup Functionalities
*/
class QmitkMovieMaker : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  
  /*!  
  \brief default constructor  
  */  
  QmitkMovieMaker(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /*!  
  \brief default destructor  
  */  
  virtual ~QmitkMovieMaker();

  /*!  
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc.  
  */  
  virtual QWidget * CreateControlWidget(QWidget *parent);

  /*!  
  \brief method for creating the applications main widget  
  */  
  virtual QWidget * CreateMainWidget(QWidget * parent);

  /*!  
  \brief method for creating the connections of main and control widget  
  */  
  virtual void CreateConnections();

  /*!  
  \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
  */  
  virtual QAction * CreateAction(QActionGroup *parent);

  virtual void Activated();

  void FocusChange();

signals:
  void StartBlockControls();
  void EndBlockControls();

public slots:

  void StartPlaying();
  void PausePlaying();
  void StopPlaying();

  void SetLooping( bool looping );
  void SetDirection( int direction );
  void SetAspect( int aspect );

  void SetWindow( int window );

  void RotateCamera();

protected slots:  

  void GenerateMovie();

protected:  
  /*!  
  * default main widget containing 4 windows showing 3   
  * orthogonal slices of the volume and a 3d render window  
  */  
  QmitkStdMultiWidget* m_MultiWidget;

  /*!  
  * controls containing sliders for scrolling through the slices  
  */  
  QmitkMovieMakerControls* m_Controls;

private:

  mitk::BaseController* GetSpatialController();

  mitk::BaseController* GetTemporalController();

  void SwitchLooping(bool looping);
  
  void SwitchDirection(int direction);
  
  void SwitchAspect(int aspect);

  mitk::Stepper* GetAspectStepper();

  QmitkStepperAdapter* m_StepperAdapter;

  typedef itk::SimpleMemberCommand< QmitkMovieMaker > MemberCommand;
  MemberCommand::Pointer m_FocusManagerCallback;

  QTimer* m_Timer;

  QTime* m_Time;

  bool m_Looping;
  int m_Direction;

  int m_Aspect;

};
#endif // !defined(QMITK_MOVIEMAKER_H__INCLUDED)
