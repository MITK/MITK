/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision: 14610 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef @UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED
#define @UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED

@INCLUDE_DLL_H@
#include <@VIEW_BASE_CLASS_H@>

class QWidget;
class QPushButton;

@BEGIN_NAMESPACE@

class @DLL_DEFINE@ @VIEW_CLASS@ : public @VIEW_BASE_CLASS@
{
public:
  void SetFocus();

protected:
  void CreateQtPartControl(QWidget* parent);

  QPushButton* m_ButtonStart;
};

@END_NAMESPACE@

#endif /*@UBUNDLE_NAMESPACE@_@UVIEW_CLASS@_H_INCLUDED*/
