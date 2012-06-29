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


#ifndef BERRYACTION_H
#define BERRYACTION_H

#include <QAction>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

class ActionPrivate;

class BERRY_UI_QT Action : public QAction
{

public:

  /**
   * Creates a new action with no text and no image.
   * <p>
   * Configure the action later using the set methods.
   * </p>
   */
  Action();

  /**
   * Creates a new action with the given text and no image.
   *
   * @param text
   *            the string used as the text for the action, or
   *            <code>null</code> if there is no text
   */
  Action(const QString& text);

  /**
   * Creates a new action with the given text and image.
   *
   * @param text
   *            the action's text, or <code>null</code> if there is no text
   * @param image
   *            the action's image, or <code>null</code> if there is no
   *            image
   */
  Action(const QString& text, const QIcon& image);

  QString GetId() const;
  void SetId(const QString& id);

private:

  QScopedPointer<ActionPrivate> d;

  Q_DISABLE_COPY(Action)

};

}

#endif // BERRYACTION_H
