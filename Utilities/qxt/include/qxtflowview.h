/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 **
 ** This is a derived work of PictureFlow (http://pictureflow.googlecode.com)
 ** The original code was distributed under the following terms:
 **
 ** Copyright (C) 2008 Ariya Hidayat (ariya@kde.org)
 ** Copyright (C) 2007 Ariya Hidayat (ariya@kde.org)
 **
 ** Permission is hereby granted, free of charge, to any person obtaining a copy
 ** of this software and associated documentation files (the "Software"), to deal
 ** in the Software without restriction, including without limitation the rights
 ** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 ** copies of the Software, and to permit persons to whom the Software is
 ** furnished to do so, subject to the following conditions:
 **
 ** The above copyright notice and this permission notice shall be included in
 ** all copies or substantial portions of the Software.
 **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 ** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 ** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 ** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 ** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 ** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 ** THE SOFTWARE.
 ****************************************************************************/

#ifndef QXT_FLOWVIEW_H
#define QXT_FLOWVIEW_H

#include <QWidget>
#include <QAbstractItemModel>
#include "qxtglobal.h"


class QxtFlowViewPrivate;
class QXT_GUI_EXPORT QxtFlowView : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QSize slideSize READ slideSize WRITE setSlideSize)
    Q_PROPERTY(QModelIndex currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(int pictureRole READ pictureRole WRITE setPictureRole)
    Q_PROPERTY(int pictureColumn READ pictureColumn WRITE setPictureColumn)
    Q_PROPERTY(QModelIndex rootIndex READ rootIndex WRITE setRootIndex)

#if 0
    Q_PROPERTY(int textRole READ textRole WRITE setTextRole)
    Q_PROPERTY(int textColumn READ textColumn WRITE setTextColumn)
#endif


public:

    enum ReflectionEffect
    {
        NoReflection,
        PlainReflection,
        BlurredReflection
    };

    /*!Creates a new PictureFlow widget.*/
    QxtFlowView(QWidget* parent = 0);

    /*!Destroys the widget.*/
    ~QxtFlowView();

    /*!
    Sets the model for the view to present.
    <br>
    Note: The view does not take ownership of the model unless it is the model's parent object because it may be shared between many different views.
    */
    void setModel(QAbstractItemModel * model);

    /*!Returns the model that this view is presenting.*/
    QAbstractItemModel * model();

    /*!Returns the background color.*/
    QColor backgroundColor() const;

    /*!Sets the background color. By default it is black.*/
    void setBackgroundColor(const QColor& c);

    /*!Returns the dimension of each slide (in pixels).*/
    QSize slideSize() const;

    /*!Sets the dimension of each slide (in pixels).*/
    void setSlideSize(QSize size);

    /*!Returns the index of slide currently shown in the middle of the viewport.*/
    QModelIndex currentIndex() const;


    //!Returns the model index of the model's root item. The root item is the parent item to the view's toplevel items. The root can be invalid. \n \sa setRootIndex();
    QModelIndex rootIndex() const;


    //!Sets the root item to the item at the given index.\n \sa rootIndex();
    void setRootIndex(QModelIndex index);


    /*!Returns the effect applied to the reflection.*/
    ReflectionEffect reflectionEffect() const;

    /*!Sets the effect applied to the reflection. The default is PlainReflection.*/
    void setReflectionEffect(ReflectionEffect effect);


    /*!Returns the role currently used for the image.*/
    int pictureRole();
    /*!Sets the role to use for the image. the default is Qt::DecorationRole*/
    void setPictureRole(int);

    /*!Returns the column currently used for the image.*/
    int pictureColumn();
    /*!Sets the column to use for the image. the default is 0*/
    void setPictureColumn(int);

#if 0
    int textRole();
    void setTextRole(int);
    int textColumn();
    void setTextColumn(int);
#endif

public Q_SLOTS:

    /*! Sets slide to be shown in the middle of the viewport. No animation
        effect will be produced, unlike using showSlide. */
    void setCurrentIndex(QModelIndex index);

    /*! Shows previous slide using animation effect. */
    void showPrevious();

    /*! Shows next slide using animation effect. */
    void showNext();

    /*! Go to specified slide using animation effect. */
    void showSlide(QModelIndex index);

    /*! Rerender the widget. Normally this function will be automatically invoked whenever necessary, e.g. during the transition animation. */
    void render();

    /*! Schedules a rendering update. Unlike render(), this function does not cause immediate rendering.*/
    void triggerRender();

Q_SIGNALS:
    void currentIndexChanged(QModelIndex index);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void wheelEvent(QWheelEvent * event);
private Q_SLOTS:
    void updateAnimation();

private:
    QxtFlowViewPrivate* d;
};

#endif // PICTUREFLOW_H

