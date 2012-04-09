/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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

