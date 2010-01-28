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


#ifndef MITKDelegateManager_H_HEADER_INCLUDED
#define MITKDelegateManager_H_HEADER_INCLUDED

#include "mitkMessage.h"
#include "MitkExtExports.h"
#include "mitkCommon.h"

#include <map>

namespace mitk
{

  ///
  /// \brief Renames the mitk::MessageAbstractDelegate, which is a Functor to a simple function
  /// with no params, to "Command". This should emphasize that it is not used as an event
  /// function but rather as a command.
  ///
  //typedef MessageAbstractDelegate Command;

  ///
  /// \class DelegateManager
  /// \brief A class that stores commands (= mitk::Message) via a unique identifier.
  /// One could also say it is an Event Multiplexer.
  ///
  ///
  class MitkExt_EXPORT DelegateManager
  {
    public:
      ///
      /// \brief Singleton pattern.
      ///
      static DelegateManager* GetInstance();
      ///
      /// \brief Adds or overwrites a Command. *Attention*: This is class is the owner of the Command.
      /// Thus whenever a command is overwritten, the old Command is deleted!
      ///
      void SetCommand(const std::string& _CommandID, MessageAbstractDelegate<>*);
      ///
      /// \brief Removes and *deletes* the Command with the given id.
      ///
      bool RemoveCommand(const std::string& _CommandID);
      ///
      /// \brief Returns the Command with the given id or NULL if the _CommandID is unknown.
      ///
      MessageAbstractDelegate<>* GetCommand(const std::string& _CommandID) const;
      ///
      /// \brief Dtor: Deletes all commands and clears m_CommandMap.
      ///
      virtual ~DelegateManager();
    protected:
      ///
      /// \brief Singleton pattern: protected Ctor.
      ///
      DelegateManager();
      ///
      /// \brief Maps IDs to Commands.
      ///
      std::map<std::string, MessageAbstractDelegate<>*> m_CommandMap;

  };
}


#endif /* MITKDelegateManager_H_HEADER_INCLUDED */


