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


#ifndef CHERRYQTITEMSELECTION_H_
#define CHERRYQTITEMSELECTION_H_

#include "cherryUiQtDll.h"

#include <cherryISelection.h>

#include <QItemSelection>

namespace cherry {

class CHERRY_UI_QT QtItemSelection : public virtual ISelection
{

public:

  cherryObjectMacro(QtItemSelection)

  QtItemSelection();
  QtItemSelection(const QItemSelection& sel);

  bool IsEmpty() const;
  const QItemSelection& GetQItemSelection() const;

private:

  QItemSelection m_QItemSelection;

};

}

#endif /* CHERRYQTITEMSELECTION_H_ */
