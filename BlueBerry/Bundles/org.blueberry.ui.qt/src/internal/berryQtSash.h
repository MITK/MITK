/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYQTSASH_H_
#define BERRYQTSASH_H_

#include <QWidget>
#include <QRubberBand>

#include <berryGuiTkISelectionListener.h>

namespace berry {

class QtSash : public QWidget
{
    Q_OBJECT

public:
    QtSash(Qt::Orientation o, QWidget* parent = 0, bool smooth = true);
    ~QtSash();

    //void setOrientation(Qt::Orientation o);
    Qt::Orientation GetOrientation() const;
    bool SmoothResize() const;

    void AddSelectionListener(GuiTk::ISelectionListener::Pointer listener);
    void RemoveSelectionListener(GuiTk::ISelectionListener::Pointer listener);

protected:
    //void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    bool event(QEvent *);

    void drawRubberBand(int x, int y, int width, int height);

    // needed for stylesheet support
    void paintEvent(QPaintEvent*);

    //void moveSplitter(int p);
    //int closestLegalPosition(int p);

private:

  bool smooth;
  Qt::Orientation orientation;

  QRubberBand* rubberBand;

  QRect startRect;
  int lastX, lastY;
  bool dragging;

  GuiTk::ISelectionListener::Events selectionEvents;
};

}


#endif /* BERRYQTSASH_H_ */
