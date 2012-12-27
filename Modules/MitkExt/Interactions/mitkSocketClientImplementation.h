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


#ifndef MITKSOCKETCLIENTIMPLEMENTATION_H
#define MITKSOCKETCLIENTIMPLEMENTATION_H

#include "mitkCommon.h"
#include "MitkExtExports.h"

namespace mitk
{

  //##Documentation
  //## @brief GUI indepentent Interface for all Gui depentent implementations of a ProgressBar.
  class MitkExt_EXPORT SocketClientImplementation
  {

  public:

    //##Documentation
    //## @brief Constructor
    SocketClientImplementation(){};

    //##Documentation
    //## @brief Destructor
    virtual ~SocketClientImplementation(){};

    virtual void open( const char* ipAdress, unsigned short port )=0;

    virtual void setMaxConnectionAdvance( int maxConnectionAdvance )=0;

    virtual bool send( unsigned int messagetype, unsigned int bodySize = 0, char* body = NULL )=0;
  };

}// end namespace mitk

#endif /* define MITKSOCKETCLIENTIMPLEMENTATION_H */
