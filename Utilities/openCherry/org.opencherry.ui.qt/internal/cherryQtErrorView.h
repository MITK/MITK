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

#ifndef CHERRYQTERRORVIEW_H_
#define CHERRYQTERRORVIEW_H_

#include <org.opencherry.ui/src/cherryViewPart.h>

#include <QLabel>
#include <QString>

namespace cherry {
  
class QtErrorView : public ViewPart
{
  
public:
  
  cherryClassMacro(QtErrorView);
  
  QtErrorView();
  
  void SetErrorMsg(const std::string& msg);
  void* CreatePartControl(void* parent);
  void SetFocus();
 
private:
  
  QString m_ErrorMsg;
  QLabel* m_Label;
};

} // namespace cherry

#endif /*CHERRYQTERRORVIEW_H_*/
