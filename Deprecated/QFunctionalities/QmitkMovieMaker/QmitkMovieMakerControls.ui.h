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

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/


void QmitkMovieMakerControls::BTNPlay()
{
  slidAngle->setDisabled( true ); 
  btnMovie->setEnabled( false );
  btnPlay->setEnabled( false );
  btnScreenshot->setEnabled( false );

  // signal that playing will start shortly
  emit StartPlaying();
}

void QmitkMovieMakerControls::BTNPause()
{
  slidAngle->setDisabled( false ); 
  btnMovie->setEnabled( true );
  btnPlay->setEnabled( true );
  btnScreenshot->setEnabled( true );

  // signal that playing has ended
  emit PausePlaying();
}


void QmitkMovieMakerControls::BTNStop()
{
  slidAngle->setDisabled( false ); 
  btnMovie->setEnabled( true );
  btnPlay->setEnabled( true );
  btnScreenshot->setEnabled( true );
  
  // signal that playing has ended
  emit StopPlaying();
}

void QmitkMovieMakerControls::RBTNForward()
{
  emit SwitchDirection( 0 );
}


void QmitkMovieMakerControls::RBTNBackward()
{
  emit SwitchDirection( 1 );
}


void QmitkMovieMakerControls::RBTNPingPong()
{
  emit SwitchDirection( 2 );
}


void QmitkMovieMakerControls::RBTNSpatial()
{
  emit SwitchAspect( 0 );
}


void QmitkMovieMakerControls::RBTNTemporal()
{
  emit SwitchAspect( 1 );
}

void QmitkMovieMakerControls::RBTNCombined()
{
  emit SwitchAspect( 2 );
}

//void QmitkMovieMakerControls::CMBSelectedWindow( int window )
//{
// emit SwitchSelectedWindow( window );
//}

void QmitkMovieMakerControls::cmbSelectedStepperWindow_activated( int window)
{
  emit SwitchSelectedStepperWindow( window );
}

void QmitkMovieMakerControls::cmbSelectedRecordWindow_activated( int window)
{
    emit SwitchSelectedRecordingWindow( window );
}

void QmitkMovieMakerControls::BlockControls()
{
  BlockControls( true );
}


void QmitkMovieMakerControls::UnBlockControls()
{
  BlockControls( false );    
}

void QmitkMovieMakerControls::UnBlockControlsMovieDeactive()
{
  BlockControls( false );    
  btnMovie->setEnabled( false );
}


void QmitkMovieMakerControls::init()
{
  m_Playing = false;
}


void QmitkMovieMakerControls::BlockControls( bool blocked )
{
  slidAngle->setDisabled( blocked );
  spnDuration->setEnabled( ! blocked );
  btnPlay->setEnabled( ! blocked );
  btnMovie->setEnabled( ! blocked );
  btnScreenshot->setEnabled( ! blocked );
}
