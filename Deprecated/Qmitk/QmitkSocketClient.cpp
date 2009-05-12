/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13599 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <QmitkSocketClient.h>
#include <mitkSocketClient.h>
#include <iostream>

/**
 * Konstruktor
 */
QmitkSocketClient::QmitkSocketClient() :  socket( this ), connectionAdvance(0), maxConnectionAdvance(2000) {

  QObject::connect( &socket, SIGNAL(connected()), this, SLOT(connected()) );  
  QObject::connect( &socket, SIGNAL(connectionClosed()), this, SLOT(connectionClosed()) );    
  connect( &socket, SIGNAL( error( int ) ) , this, SLOT ( error( int ) ) );  
  connect( &timer, SIGNAL( timeout() ) , this, SLOT ( timeout() ) );
  mitk::SocketClient::SetImplementationInstance(this);
}

/** 
 *
 */
QmitkSocketClient::~QmitkSocketClient() {

  disconnect( &socket, SIGNAL(connected()), this, SLOT(connected()) );  
  disconnect( &socket, SIGNAL(connectionClosed()), this, SLOT(connectionClosed()) );
}


/**
 * setzt die maximale Anzahl an Verbindungsversuche
 * default sind 2000
 */
void QmitkSocketClient::setMaxConnectionAdvance( int maxConnectionAdvance ) {

  this->maxConnectionAdvance = maxConnectionAdvance;
}

/**
 *
 */
void QmitkSocketClient::open( const char* ipAddress, unsigned short port ) {

  this->ipAddress = ipAddress;
  this->port = port;
  socket.connectToHost ( ipAddress, port );
}

/**
 *
 */
void QmitkSocketClient::connected() {

  std::cout << "conected to " << ipAddress.ascii() << " port " << port << std::endl;
}

/**
 *
 */
void QmitkSocketClient::connectionClosed() {

  disconnect( &socket, SIGNAL(readyRead()), this, SLOT(readyRead()) );
  std::cout << "connectionClosed" << std::endl;
}

/**
 *
 */
bool QmitkSocketClient::send( unsigned int messagetype, unsigned int bodySize, char* body ) {

  if ( messagetype > 7 )
    std::cout << "fehler!!!!!!!" << std::endl;

  char* wb = writeBuffer;  
  *((unsigned int*) wb) = messagetype;
  wb += 4;

  *(unsigned int*) wb = bodySize;
  wb += 4;

  for ( unsigned int i=0; i < bodySize; i++ )
    *(wb++) = *(body++);

  int length = 8 + bodySize;

  int a = socket.writeBlock ( writeBuffer, length);

  if ( a == length )
    return true;
  else 
    return false;
}

/**
 *
 */
void QmitkSocketClient::error( int nr ) {

  std::cout << "Error: Ende " << nr << " Versuch Nummer: "<< connectionAdvance << std::endl;

  if ( connectionAdvance < maxConnectionAdvance ) {

    connectionAdvance++;
    std::cout << "In einer Sekunde wird ein erneuter Versuch unternommen" << std::endl;
    timer.start ( 2000, true );
  }
}

/**
 *
 */
void QmitkSocketClient::timeout() {
  
  std::cout << "Neuer versuch eine Verbindung aufzubauen zu " << ipAddress.ascii() << " Port: " << port << std::endl;
  socket.connectToHost( ipAddress, port );    
}
