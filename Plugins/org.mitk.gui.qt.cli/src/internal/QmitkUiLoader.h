/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
 
#ifndef QmitkUiLoader_h
#define QmitkUiLoader_h

#include <QUiLoader>
#include <QStringList>

class QmitkUiLoader : public QUiLoader
{

  Q_OBJECT

public:
  QmitkUiLoader(QObject *parent=0);
  virtual ~QmitkUiLoader();

  QStringList availableWidgets () const;
  virtual QWidget* createWidget(const QString & className, QWidget * parent = 0, const QString & name = QString() );

private:

}; // end class

#endif // QmitkUiLoader_h
