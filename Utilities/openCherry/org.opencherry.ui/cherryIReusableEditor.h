/*=========================================================================
 
Program:   openCherry Platform
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

#ifndef CHERRYIREUSABLEEDITOR_H_
#define CHERRYIREUSABLEEDITOR_H_

#include "cherryIEditorPart.h"
#include "cherryIEditorInput.h"

namespace cherry {

/**
 * Interface for reusable editors. 
 * 
 * An editors may support changing its input so that 
 * the workbench may change its contents instead of 
 * opening a new editor.
 */
struct CHERRY_UI IReusableEditor : public IEditorPart {
  
  cherryClassMacro(IReusableEditor)
  
  virtual ~IReusableEditor() {}
  
    /**
     * Sets the input to this editor.
     * 
     * <p><b>Note:</b> Clients must fire the {@link IEditorPart#PROP_INPUT } 
     * property change within their implementation of 
     * <code>setInput()</code>.<p>
     *
     * @param input the editor input
     */
    virtual void SetInput(IEditorInput::Pointer input) = 0;
};

}

#endif /*CHERRYIREUSABLEEDITOR_H_*/
