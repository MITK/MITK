/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13561 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKSOCKET_CLIENT
#define QMITKSOCKET_CLIENT

#include <qsocket.h> 
#include <qtimer.h>
#include <qobject.h>

#include <mitkSocketClientImplementation.h>

/**
 *
 */
class QmitkSocketClient : public QObject, public mitk::SocketClientImplementation 
{

  Q_OBJECT

protected:
  QSocket socket;
  QString ipAddress;
  unsigned short port;
  char readBuffer[256];
  char writeBuffer[256];
  QTimer timer;
  int connectionAdvance;
  int maxConnectionAdvance;

public:
  QmitkSocketClient();
  ~QmitkSocketClient();
  void open( const char* ipAdress, unsigned short port );
  void setMaxConnectionAdvance( int maxConnectionAdvance );
  bool send( unsigned int messagetype, unsigned int bodySize = 0, char* body = NULL );

protected slots:
  void connected ();
  void connectionClosed();
  void error( int );
  void timeout();
};
#endif // QMITKSOCKET_CLIENT
