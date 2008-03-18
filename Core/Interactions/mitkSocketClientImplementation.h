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


#ifndef MITKSOCKETCLIENTIMPLEMENTATION_H
#define MITKSOCKETCLIENTIMPLEMENTATION_H

#include "mitkCommon.h"
	 
namespace mitk 
{

  //##Documentation
  //## @brief GUI indepentent Interface for all Gui depentent implementations of a ProgressBar.
  class MITK_CORE_EXPORT SocketClientImplementation
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
