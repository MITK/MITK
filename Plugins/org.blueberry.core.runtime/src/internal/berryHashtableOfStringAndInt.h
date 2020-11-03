/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHASHTABLEOFSTRINGANDINT_H
#define BERRYHASHTABLEOFSTRINGANDINT_H

#include <QHash>

namespace berry {

class HashtableOfStringAndInt : public QHash<QString,int>
{

};

}

#endif // BERRYHASHTABLEOFSTRINGANDINT_H
