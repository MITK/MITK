/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
    QtSash(Qt::Orientation o, QWidget* parent = nullptr, bool smooth = true);
    ~QtSash() override;

    //void setOrientation(Qt::Orientation o);
    Qt::Orientation GetOrientation() const;
    bool SmoothResize() const;

    void AddSelectionListener(GuiTk::ISelectionListener::Pointer listener);
    void RemoveSelectionListener(GuiTk::ISelectionListener::Pointer listener);

protected:
    //void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    bool event(QEvent *) override;

    void drawRubberBand(int x, int y, int width, int height);

    // needed for stylesheet support
    void paintEvent(QPaintEvent*) override;

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
