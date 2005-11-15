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

  // signal that playing will start shortly
  emit StartPlaying();
}

void QmitkMovieMakerControls::BTNPause()
{
  slidAngle->setDisabled( false ); 
  btnMovie->setEnabled( true );
  btnPlay->setEnabled( true );

  // signal that playing has ended
  emit PausePlaying();
}


void QmitkMovieMakerControls::BTNStop()
{
  slidAngle->setDisabled( false ); 
  btnMovie->setEnabled( true );
  btnPlay->setEnabled( true );

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

void QmitkMovieMakerControls::CMBSelectedWindow( int window )
{
 emit SwitchSelectedWindow( window );
}

void QmitkMovieMakerControls::BlockControls()
{
  slidAngle->setDisabled(true);
  spnDuration->setEnabled(false);
  btnPlay->setEnabled(false);
  btnMovie->setEnabled(false);
}


void QmitkMovieMakerControls::UnBlockControls()
{
  slidAngle->setDisabled(false); 
  spnDuration->setEnabled(true);
  btnPlay->setEnabled(true);
  btnMovie->setEnabled(true);
}


void QmitkMovieMakerControls::init()
{
  m_Playing = false;
}
