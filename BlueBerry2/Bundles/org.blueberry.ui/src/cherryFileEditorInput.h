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


#ifndef CHERRYFILEEDITORINPUT_H_
#define CHERRYFILEEDITORINPUT_H_

#include "cherryIPathEditorInput.h"

namespace cherry {

class FileEditorInput : public IPathEditorInput
{

public:

  osgiObjectMacro(FileEditorInput);

  FileEditorInput(const Poco::Path& path);

  Poco::Path GetPath() const;

  bool Exists() const;

  std::string GetName() const ;

  std::string GetToolTipText() const;

  bool operator==(const IEditorInput* o) const;

private:

  Poco::Path m_Path;
};

}

#endif /* CHERRYFILEEDITORINPUT_H_ */
