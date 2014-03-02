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

#ifndef QDiffusionImagingSplashScreen_h_included
#define QDiffusionImagingSplashScreen_h_included

#include <QWidget>
#include <QImage>

class QDiffusionImagingSplashScreen : public QWidget
{
  Q_OBJECT

  public:

    QDiffusionImagingSplashScreen(const QImage& image, QWidget* parent = 0);
    ~QDiffusionImagingSplashScreen();
    virtual QSize sizeHint() const;

  protected:

    virtual void paintEvent(QPaintEvent* paintEvent);
    virtual void resizeEvent(QResizeEvent* resizeEvent);
    virtual void mouseReleaseEvent(QMouseEvent* mouseEvent);

    QRegion createMaskRegion( const QImage & image, bool posMask = true );

    QImage backgroundImage;
};

#endif

