/*=========================================================================

Program:   mbilog - logging for mitk / BlueBerry

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _mbilogBackendCout_H
#define _mbilogBackendCout_H

#include <iostream>

#include "mbilogExports.h"
#include "mbilogAbstractBackend.h"
#include "mbilogLogMessage.h"
#include "mbilogLoggingTypes.h"

namespace mbilog{

  /** Documentation 
   *  \brief Default backend of the mbi logging mechanism. This backend is used if no other backend is registered.
   *         The backend formats the logging messages to a normal string and writes them to std::cout.
   *  \ingroup mbilog
   */
  class MBILOG_DLL_API BackendCout : public AbstractBackend
  {
    public:

      BackendCout();
      virtual ~BackendCout();
	 /** \brief This method is called by the mbi logging mechanism if the object is registered in 
	  *         the mbi logging mechanism and a logging message is emitted. The method formats the 
	  *         logging messages to a normal string (depending on formatting mode) and writes it to std::cout.
	  *
	  *  \param logMessage Logging message.
	  *
	  */
      virtual void ProcessMessage(const mbilog::LogMessage &l );

	  /** \brief Sets the formatting mode. If true long messages will be displayed. Default is false (short/smart messages).
	   *         Long messages provide all informations and are also capable to be postproccessed (e.g. in a web viewer).
	   */
      void SetFull(bool full);

	  /** \brief Method formats the given LogMessage in the smart/short format and writes it to std::cout.
	    * \param threadID Can be set to the threadID where the logging message was emitted. Is 0 by default.
		*/
	  static void FormatSmart(const LogMessage &l,int threadID=0);
      
	  /** \brief Method formats the given LogMessage in the full/long format and writes it to std::cout.
	    * \param threadID Can be set to the threadID where the logging message was emitted. Is 0 by default.
		*/
	  static void FormatFull(const LogMessage &l,int threadID=0);

	  /** \brief Method formats the given LogMessage in the smart/short format and writes it to the given std::ostream.
	    * \param threadID Can be set to the threadID where the logging message was emitted. Is 0 by default.
		*/
      static void FormatSmart(std::ostream &out, const LogMessage &l,int threadID=0);
      
	  /** \brief Method formats the given LogMessage in the full/long format and writes it to the given std::ostream.
	    * \param threadID Can be set to the threadID where the logging message was emitted. Is 0 by default.
		*/
	  static void FormatFull(std::ostream &out, const LogMessage &l,int threadID=0);

    private:

	  /** \brief Writes the system time to the given stream. */
      static void AppendTimeStamp(std::ostream& out);
      
	  /** \brief Special variant of method FormatSmart which uses colored messages (only for windows).*/
	  //TODO: implement for linux?
	  static void FormatSmartWindows(const mbilog::LogMessage &l,int /*threadID*/);

	  /** \brief The formatting mode of this backend. True is full/long message formatting mode. False is short/smart message formatting mode */
      bool useFullOutput;

  };

}


#endif