/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTGLOBALEVENTFILTER_H_
#define BERRYQTGLOBALEVENTFILTER_H_

#include <QObject>

class QEvent;

namespace berry {

class QtGlobalEventFilter : public QObject
{
  Q_OBJECT

public:

  QtGlobalEventFilter(QObject* parent = nullptr);

  bool eventFilter(QObject* obj, QEvent* event) override;
};

}

#endif /* BERRYQTGLOBALEVENTFILTER_H_ */
