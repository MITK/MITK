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

#ifndef CHERRYNATIVETABITEM_H_
#define CHERRYNATIVETABITEM_H_

#include "../util/cherryAbstractTabItem.h"

namespace cherry
{

class NativeTabFolder;

class NativeTabItem: public AbstractTabItem
{

private:

  Object::Pointer data;
  NativeTabFolder* parent;

public:

  NativeTabItem(NativeTabFolder* parent, int index);

  /* (non-Javadoc)
   * @see AbstractTabItem#getBounds()
   */
  QRect GetBounds();

  void SetInfo(const PartInfo& info);

  void Dispose();

  /* (non-Javadoc)
   * @see AbstractTabItem#getData()
   */
  Object::Pointer GetData();

  /* (non-Javadoc)
   * @see AbstractTabItem#setData(java.lang.Object)
   */
  void SetData(Object::Pointer d);

};

}

#endif /* CHERRYNATIVETABITEM_H_ */
