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

#include "QDiffusionImagingSplashScreen.h"
#include <QtGui>
#include <iostream>

QDiffusionImagingSplashScreen::QDiffusionImagingSplashScreen(const QImage& image, QWidget* parent)
:QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SplashScreen )
,backgroundImage(image)
{
  setAttribute(Qt::WA_TranslucentBackground);
  resize(image.size());
  resizeEvent(NULL);
}

QDiffusionImagingSplashScreen::~QDiffusionImagingSplashScreen()
{
}

QSize QDiffusionImagingSplashScreen::sizeHint() const
{
  return backgroundImage.size();
}

void QDiffusionImagingSplashScreen::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  painter.drawImage( rect(), backgroundImage );
}

void QDiffusionImagingSplashScreen::resizeEvent(QResizeEvent*)
{
  setMask( createMaskRegion( backgroundImage) );
}

QRegion QDiffusionImagingSplashScreen::createMaskRegion( const QImage & image, bool posMask )
{
  if ( image.isNull() ) return QRegion();

  if (image.depth() != 32)
  {
    QImage img32 = image.convertToFormat(QImage::Format_RGB32);
    return createMaskRegion( img32, posMask );
  }

  int width = image.width();
  int height = image.height();
  QRegion retVal;

  for( int y = 0; y < height; ++y )
  {
    // guarented to be 32 bit by the check above
    QRgb *currLine = (QRgb *)image.scanLine( y );
    int xstart = -1;
    int xcurr = -1;
    QRgb * currVal = currLine;
    for( int x = 0; x < width; ++x, ++currVal )
    {
      int alpha = qAlpha( *currVal );
      if ( ( posMask && alpha != 0 ) || ( !posMask && alpha == 0 ) )
      {
        // on non-alpha pixel
        if ( xstart == -1 )
        {
          // remember start of non-alpha line-segment (if necessary)
          xstart = x;
        }
        xcurr = x;
      }
      else // alpha-pixel
      if ( xcurr != -1 ) // if this alpha pixel is following a non-alpha line-segment
      {
        retVal += QRegion( xstart, y, xcurr - xstart + 1, 1 );
        xstart = -1;
        xcurr = -1;
      }
    }
    if ( xcurr != -1 )
    {
      retVal += QRegion( xstart, y, xcurr - xstart + 1, 1 );
    }
  }

  return retVal;
}

void QDiffusionImagingSplashScreen::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
  QWidget::mouseReleaseEvent(mouseEvent);
  hide();
}

