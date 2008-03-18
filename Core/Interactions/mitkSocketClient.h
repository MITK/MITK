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


#ifndef MITKSOCKETCLIENT_H
#define MITKSOCKETCLIENT_H
#include <itkObject.h>
#include "mitkCommon.h"

#include "mitkSocketClientImplementation.h"


namespace mitk 
{

  //##Documentation
  //## @brief Sending a message to the applications ProgressBar
  //##
  //## Holds a GUI dependent ProgressBarImplementation and sends the progress further.
  //## All mitk-classes use this class to display progress on GUI-ProgressBar.
  //## The mainapplication has to set the internal held ProgressBarImplementation with SetImplementationInstance(..).
  //## @ingroup Interaction
  class MITK_CORE_EXPORT SocketClient : public itk::Object
  {

  public:

    itkTypeMacro(ProgressBar, itk::Object);

    static SocketClient* GetInstance();

    static void SetImplementationInstance(SocketClientImplementation* implementation);

    void open( const char* ipAdress, unsigned short port );
  
    void setMaxConnectionAdvance( int maxConnectionAdvance );

    bool send( unsigned int messagetype, unsigned int bodySize = 0, char* body = NULL );

  protected:
    SocketClient();
    virtual ~SocketClient();
    static SocketClientImplementation* m_Implementation;
    static SocketClient* m_Instance;
  };

}// end namespace mitk

#endif /* define MITKSOCKETCLIENT_H */
