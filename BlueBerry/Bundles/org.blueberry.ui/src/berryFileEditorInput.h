/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYFILEEDITORINPUT_H_
#define BERRYFILEEDITORINPUT_H_

#include "berryIPathEditorInput.h"

namespace berry {

class FileEditorInput : public IPathEditorInput
{

public:

  berryObjectMacro(FileEditorInput);

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

#endif /* BERRYFILEEDITORINPUT_H_ */
