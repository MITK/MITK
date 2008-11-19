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


#ifndef CHERRYQTSASH_H_
#define CHERRYQTSASH_H_

#include <QWidget>
#include <QRubberBand>

#include <guitk/cherryGuiTkISelectionListener.h>

namespace cherry {

class QtSash : public QWidget
{
    Q_OBJECT

public:
    QtSash(Qt::Orientation o, QWidget* parent = 0, bool opaque = true);
    ~QtSash();

    //void setOrientation(Qt::Orientation o);
    Qt::Orientation GetOrientation() const;
    bool OpaqueResize() const;

    void AddSelectionListener(GuiTk::ISelectionListener::Pointer listener);
    void RemoveSelectionListener(GuiTk::ISelectionListener::Pointer listener);

protected:
    //void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    bool event(QEvent *);

    void drawRubberBand(int x, int y, int width, int height);

    //void moveSplitter(int p);
    //int closestLegalPosition(int p);

private:

  bool opaque;
  Qt::Orientation orientation;

  QRubberBand* rubberBand;

  QRect startRect;
  int lastX, lastY;
  bool dragging;

  GuiTk::ISelectionListener::Events selectionEvents;
};

}


#endif /* CHERRYQTSASH_H_ */
