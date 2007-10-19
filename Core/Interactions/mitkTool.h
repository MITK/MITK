/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
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

#ifndef mitkTool_h_Included
#define mitkTool_h_Included

#include "mitkCommon.h"
#include "mitkStateMachine.h"

#include <iostream>

namespace mitk
{

class ToolManager;

/**
  \brief Base class of all tools used by mitk::ToolManager.

  \sa ToolManager
  \sa SegTool2D

  \ingroup Interaction
  \ingroup Reliver

  There is a separate page describing the \ref QmitkSliceBasedSegmentationTechnicalPage.

  Every tool is a mitk::StateMachine, which can follow any transition pattern that it likes. One important thing to know is, that 
  every derived tool should always call SuperClass::Deactivated() in its own implementation of Deactivated, because mitk::Tool
  resets the StateMachine in this method. Only if you are very sure that you covered all possible things that might happen to your
  own tool, you should consider not to reset the StateMachine from time to time.

  To learn about the MITK implementation of state machines in general, have a look at \ref InteractionPage.

  To derive a non-abstract tool, you inherit from mitk::Tool (or some other base class further down the inheritance tree), and in your
  own parameterless constructor (that is called from the itkNewMacro that you use) you pass a StateMachine pattern name to the superclass.
  Names for valid patterns can be found in StateMachine.xml (which might be enhanced by you).

  You have to implement at least GetXPM() and GetName() to provide some identification.

  Each Tool knows its ToolManager, which can provide the data that the tool should work on.

  \warning Only to be instantiated by mitk::ToolManager (because SetToolManager has to be called). All other uses are unsupported.

  $Author$
*/
class Tool : public StateMachine
{
  public:
    
    mitkClassMacro(Tool, StateMachine);

    // no New(), there should only be subclasses
    
    /**
      \brief Returns an icon in the XPM format.
      
      This icon has to fit into some kind of button in most applications, so make it smaller than 25x25 pixels.

      XPM is e.g. supported by The Gimp. But if you open any XPM file in your text editor, you will see that you could also "draw" it with an editor.
    */
    virtual const char** GetXPM() const = 0;

    /**
      \brief Returns the name of this tool. Make it short!

      This name has to fit into some kind of button in most applications, so take some time to think of a good name!
    */
    virtual const char* GetName() const = 0;
    
    /**
      \brief Name of a group.

      You can group several tools by assigning a group name. Graphical tool selectors might use this information to group tools. (What other reason could there be?)
    */
    virtual const char* GetGroup() const;

  protected:
    
    friend class ToolManager;

    /**
      \brief Logging helper class for Tools.
      Implementations of tools can send debug output to this class (C++ streaming IO).  
      To get a stream, clients call Logger(verboseness). 
      The verboseness parameter should be higher for more detailed messages, 
      lower for everyday output. 
      The default is 1.

      At the moment the verboseness-level is defined at compile-time 
      (and by default 0 - nearly no messages get to the commandline).
     */
    class ToolLogger
    {

      struct NullStream: std::ostream 
      {

        NullStream()
        :std::ios(0),
         std::ostream(0)
        {
        }

      };

      public:

        /**
         \return A valid std::ostream which might or might not be redirected to the commandline.
         \param verboseness 0 for "always". Increasing numbers for more detailed debugging messages.
        */
        static std::ostream& Logger(unsigned int verboseness = 1);

        static unsigned int GetVerboseness();
        static void SetVerboseness( unsigned int verboseness );

      protected:

        static unsigned int s_Verboseness;
        static NullStream s_NullStream;
    };

    void SetToolManager(ToolManager*);
    
    /**
     \brief Called when the tool gets activated (registered to mitk::GlobalInteraction).
     
     Derived tools should call their parents implementation.
    */
    virtual void Activated();
    
    /**
     \brief Called when the tool gets deactivated (unregistered from mitk::GlobalInteraction).

     Derived tools should call their parents implementation.
    */
    virtual void Deactivated();

    Tool(); // purposely hidden
    Tool( const char*); // purposely hidden
    virtual ~Tool();

    ToolManager* m_ToolManager;
};

} // namespace

#endif

