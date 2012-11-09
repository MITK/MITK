/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYFILEEDITORINPUT_H_
#define BERRYFILEEDITORINPUT_H_

#include "berryIPathEditorInput.h"
#include <org_blueberry_ui_Export.h>


namespace berry {

class BERRY_UI FileEditorInput : public IPathEditorInput
{

public:

  berryObjectMacro(FileEditorInput);

  FileEditorInput(const Poco::Path& path);

  Poco::Path GetPath() const;

  bool Exists() const;

  std::string GetName() const ;

  std::string GetToolTipText() const;

  bool operator==(const Object* o) const;

private:

  Poco::Path m_Path;
};

}

#endif /* BERRYFILEEDITORINPUT_H_ */
