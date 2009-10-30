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
 ** This is a derived work of QxtFlowView (http://pictureflow.googlecode.com)
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

#include "qxtflowview_p.h"




// ------------- QxtFlowViewState ---------------------------------------

QxtFlowViewState::QxtFlowViewState():
        backgroundColor(0), slideWidth(150), slideHeight(200),
        reflectionEffect(QxtFlowView::BlurredReflection), centerIndex(0)
{
}

QxtFlowViewState::~QxtFlowViewState()
{
    for (int i = 0; i < (int)slideImages.count(); i++)
        delete slideImages[i];
}

// readjust the settings, call this when slide dimension is changed
void QxtFlowViewState::reposition()
{
    angle = 70 * IANGLE_MAX / 360;  // approx. 70 degrees tilted

    offsetX = slideWidth / 2 * (PFREAL_ONE - fcos(angle));
    offsetY = slideWidth / 2 * fsin(angle);
    offsetX += slideWidth * PFREAL_ONE;
    offsetY += slideWidth * PFREAL_ONE / 4;
    spacing = 40;
}

// adjust slides so that they are in "steady state" position
void QxtFlowViewState::reset()
{
    centerSlide.angle = 0;
    centerSlide.cx = 0;
    centerSlide.cy = 0;
    centerSlide.slideIndex = centerIndex;
    centerSlide.blend = 256;

    leftSlides.resize(6);
    for (int i = 0; i < (int)leftSlides.count(); i++)
    {
        SlideInfo& si = leftSlides[i];
        si.angle = angle;
        si.cx = -(offsetX + spacing * i * PFREAL_ONE);
        si.cy = offsetY;
        si.slideIndex = centerIndex - 1 - i;
        si.blend = 256;
        if (i == (int)leftSlides.count() - 2)
            si.blend = 128;
        if (i == (int)leftSlides.count() - 1)
            si.blend = 0;
    }

    rightSlides.resize(6);
    for (int i = 0; i < (int)rightSlides.count(); i++)
    {
        SlideInfo& si = rightSlides[i];
        si.angle = -angle;
        si.cx = offsetX + spacing * i * PFREAL_ONE;
        si.cy = offsetY;
        si.slideIndex = centerIndex + 1 + i;
        si.blend = 256;
        if (i == (int)rightSlides.count() - 2)
            si.blend = 128;
        if (i == (int)rightSlides.count() - 1)
            si.blend = 0;
    }
}

// ------------- QxtFlowViewAnimator  ---------------------------------------

QxtFlowViewAnimator::QxtFlowViewAnimator():
        state(0), target(0), step(0), frame(0)
{
}

void QxtFlowViewAnimator::start(int slide)
{
    target = slide;
    if (!animateTimer.isActive() && state)
    {
        step = (target < state->centerSlide.slideIndex) ? -1 : 1;
        animateTimer.start(30);
    }
}

void QxtFlowViewAnimator::stop(int slide)
{
    step = 0;
    target = slide;
    frame = slide << 16;
    animateTimer.stop();
}

void QxtFlowViewAnimator::update()
{
    if (!animateTimer.isActive())
        return;
    if (step == 0)
        return;
    if (!state)
        return;

    int speed = 16384 / 4;

#if 1
    // deaccelerate when approaching the target
    const int max = 2 * 65536;

    int fi = frame;
    fi -= (target << 16);
    if (fi < 0)
        fi = -fi;
    fi = qMin(fi, max);

    int ia = IANGLE_MAX * (fi - max / 2) / (max * 2);
    speed = 512 + 16384 * (PFREAL_ONE + fsin(ia)) / PFREAL_ONE;
#endif

    frame += speed * step;

    int index = frame >> 16;
    int pos = frame & 0xffff;
    int neg = 65536 - pos;
    int tick = (step < 0) ? neg : pos;
    PFreal ftick = (tick * PFREAL_ONE) >> 16;

    if (step < 0)
        index++;

    if (state->centerIndex != index)
    {
        state->centerIndex = index;
        frame = index << 16;
        state->centerSlide.slideIndex = state->centerIndex;
        for (int i = 0; i < (int)state->leftSlides.count(); i++)
            state->leftSlides[i].slideIndex = state->centerIndex - 1 - i;
        for (int i = 0; i < (int)state->rightSlides.count(); i++)
            state->rightSlides[i].slideIndex = state->centerIndex + 1 + i;
    }

    state->centerSlide.angle = (step * tick * state->angle) >> 16;
    state->centerSlide.cx = -step * fmul(state->offsetX, ftick);
    state->centerSlide.cy = fmul(state->offsetY, ftick);

    if (state->centerIndex == target)
    {
        stop(target);
        state->reset();
        return;
    }

    for (int i = 0; i < (int)state->leftSlides.count(); i++)
    {
        SlideInfo& si = state->leftSlides[i];
        si.angle = state->angle;
        si.cx = -(state->offsetX + state->spacing * i * PFREAL_ONE + step * state->spacing * ftick);
        si.cy = state->offsetY;
    }

    for (int i = 0; i < (int)state->rightSlides.count(); i++)
    {
        SlideInfo& si = state->rightSlides[i];
        si.angle = -state->angle;
        si.cx = state->offsetX + state->spacing * i * PFREAL_ONE - step * state->spacing * ftick;
        si.cy = state->offsetY;
    }

    if (step > 0)
    {
        PFreal ftick = (neg * PFREAL_ONE) >> 16;
        state->rightSlides[0].angle = -(neg * state->angle) >> 16;
        state->rightSlides[0].cx = fmul(state->offsetX, ftick);
        state->rightSlides[0].cy = fmul(state->offsetY, ftick);
    }
    else
    {
        PFreal ftick = (pos * PFREAL_ONE) >> 16;
        state->leftSlides[0].angle = (pos * state->angle) >> 16;
        state->leftSlides[0].cx = -fmul(state->offsetX, ftick);
        state->leftSlides[0].cy = fmul(state->offsetY, ftick);
    }

    // must change direction ?
    if (target < index) if (step > 0)
            step = -1;
    if (target > index) if (step < 0)
            step = 1;

    // the first and last slide must fade in/fade out
    int nleft = state->leftSlides.count();
    int nright = state->rightSlides.count();
    int fade = pos / 256;

    for (int index = 0; index < nleft; index++)
    {
        int blend = 256;
        if (index == nleft - 1)
            blend = (step > 0) ? 0 : 128 - fade / 2;
        if (index == nleft - 2)
            blend = (step > 0) ? 128 - fade / 2 : 256 - fade / 2;
        if (index == nleft - 3)
            blend = (step > 0) ? 256 - fade / 2 : 256;
        state->leftSlides[index].blend = blend;
    }
    for (int index = 0; index < nright; index++)
    {
        int blend = (index < nright - 2) ? 256 : 128;
        if (index == nright - 1)
            blend = (step > 0) ? fade / 2 : 0;
        if (index == nright - 2)
            blend = (step > 0) ? 128 + fade / 2 : fade / 2;
        if (index == nright - 3)
            blend = (step > 0) ? 256 : 128 + fade / 2;
        state->rightSlides[index].blend = blend;
    }

}

// ------------- QxtFlowViewSoftwareRenderer ---------------------------------------

QxtFlowViewSoftwareRenderer::QxtFlowViewSoftwareRenderer():
        QxtFlowViewAbstractRenderer(), size(0, 0), bgcolor(0), effect(-1), blankSurface(0)
{
}

QxtFlowViewSoftwareRenderer::~QxtFlowViewSoftwareRenderer()
{
    surfaceCache.clear();
    buffer = QImage();
    delete blankSurface;
}

void QxtFlowViewSoftwareRenderer::paint()
{
    if (!widget)
        return;

    if (widget->size() != size)
        init();

    if (state->backgroundColor != bgcolor)
    {
        bgcolor = state->backgroundColor;
        surfaceCache.clear();
    }

    if ((int)(state->reflectionEffect) != effect)
    {
        effect = (int)state->reflectionEffect;
        surfaceCache.clear();
    }

    if (dirty)
        render();

    QPainter painter(widget);
    painter.drawImage(QPoint(0, 0), buffer);
}

void QxtFlowViewSoftwareRenderer::init()
{
    if (!widget)
        return;

    surfaceCache.clear();
    blankSurface = 0;

    size = widget->size();
    int ww = size.width();
    int wh = size.height();
    int w = (ww + 1) / 2;
    int h = (wh + 1) / 2;

#ifdef PICTUREFLOW_QT4
    buffer = QImage(ww, wh, QImage::Format_RGB32);
#endif
#if defined(PICTUREFLOW_QT3) || defined(PICTUREFLOW_QT2)
    buffer.create(ww, wh, 32);
#endif
    buffer.fill(bgcolor);

    rays.resize(w*2);
    for (int i = 0; i < w; i++)
    {
        PFreal gg = ((PFREAL_ONE >> 1) + i * PFREAL_ONE) / (2 * h);
        rays[w-i-1] = -gg;
        rays[w+i] = gg;
    }

    dirty = true;
}

// TODO: optimize this with lookup tables
static QRgb blendColor(QRgb c1, QRgb c2, int blend)
{
    int r = qRed(c1) * blend / 256 + qRed(c2) * (256 - blend) / 256;
    int g = qGreen(c1) * blend / 256 + qGreen(c2) * (256 - blend) / 256;
    int b = qBlue(c1) * blend / 256 + qBlue(c2) * (256 - blend) / 256;
    return qRgb(r, g, b);
}


static QImage* prepareSurface(const QImage* slideImage, int w, int h, QRgb bgcolor,
                              QxtFlowView::ReflectionEffect reflectionEffect)
{
#ifdef PICTUREFLOW_QT4
    Qt::TransformationMode mode = Qt::SmoothTransformation;
    QImage img = slideImage->scaled(w, h, Qt::IgnoreAspectRatio, mode);
#endif
#if defined(PICTUREFLOW_QT3) || defined(PICTUREFLOW_QT2)
    QImage img = slideImage->smoothScale(w, h);
#endif

    // slightly larger, to accommodate for the reflection
    int hs = h * 2;
    int hofs = h / 3;

    // offscreen buffer: black is sweet
#ifdef PICTUREFLOW_QT4
    QImage* result = new QImage(hs, w, QImage::Format_RGB32);
#endif
#if defined(PICTUREFLOW_QT3) || defined(PICTUREFLOW_QT2)
    QImage* result = new QImage;
    result->create(hs, w, 32);
#endif
    result->fill(bgcolor);

    // transpose the image, this is to speed-up the rendering
    // because we process one column at a time
    // (and much better and faster to work row-wise, i.e in one scanline)
    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++)
            result->setPixel(hofs + y, x, img.pixel(x, y));

    if (reflectionEffect != QxtFlowView::NoReflection)
    {
        // create the reflection
        int ht = hs - h - hofs;
        int hte = ht;
        for (int x = 0; x < w; x++)
            for (int y = 0; y < ht; y++)
            {
                QRgb color = img.pixel(x, img.height() - y - 1);
                result->setPixel(h + hofs + y, x, blendColor(color, bgcolor, 128*(hte - y) / hte));
            }

        if (reflectionEffect == QxtFlowView::BlurredReflection)
        {
            // blur the reflection everything first
            // Based on exponential blur algorithm by Jani Huhtanen
            QRect rect(hs / 2, 0, hs / 2, w);
            rect &= result->rect();

            int r1 = rect.top();
            int r2 = rect.bottom();
            int c1 = rect.left();
            int c2 = rect.right();

            int bpl = result->bytesPerLine();
            int rgba[4];
            unsigned char* p;

            // how many times blur is applied?
            // for low-end system, limit this to only 1 loop
            for (int loop = 0; loop < 2; loop++)
            {
                for (int col = c1; col <= c2; col++)
                {
                    p = result->scanLine(r1) + col * 4;
                    for (int i = 0; i < 3; i++)
                        rgba[i] = p[i] << 4;

                    p += bpl;
                    for (int j = r1; j < r2; j++, p += bpl)
                        for (int i = 0; i < 3; i++)
                            p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
                }

                for (int row = r1; row <= r2; row++)
                {
                    p = result->scanLine(row) + c1 * 4;
                    for (int i = 0; i < 3; i++)
                        rgba[i] = p[i] << 4;

                    p += 4;
                    for (int j = c1; j < c2; j++, p += 4)
                        for (int i = 0; i < 3; i++)
                            p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
                }

                for (int col = c1; col <= c2; col++)
                {
                    p = result->scanLine(r2) + col * 4;
                    for (int i = 0; i < 3; i++)
                        rgba[i] = p[i] << 4;

                    p -= bpl;
                    for (int j = r1; j < r2; j++, p -= bpl)
                        for (int i = 0; i < 3; i++)
                            p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
                }

                for (int row = r1; row <= r2; row++)
                {
                    p = result->scanLine(row) + c2 * 4;
                    for (int i = 0; i < 3; i++)
                        rgba[i] = p[i] << 4;

                    p -= 4;
                    for (int j = c1; j < c2; j++, p -= 4)
                        for (int i = 0; i < 3; i++)
                            p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
                }
            }

            // overdraw to leave only the reflection blurred (but not the actual image)
            for (int x = 0; x < w; x++)
                for (int y = 0; y < h; y++)
                    result->setPixel(hofs + y, x, img.pixel(x, y));
        }
    }

    return result;
}

QImage* QxtFlowViewSoftwareRenderer::surface(int slideIndex)
{
    if (!state)
        return 0;
    if (slideIndex < 0)
        return 0;
    if (slideIndex >= (int)state->slideImages.count())
        return 0;

#ifdef PICTUREFLOW_QT4
    int key = slideIndex;
#endif
#if defined(PICTUREFLOW_QT3) || defined(PICTUREFLOW_QT2)
    QString key = QString::number(slideIndex);
#endif

    QImage* img = state->slideImages.at(slideIndex);
    bool empty = img ? img->isNull() : true;
    if (empty)
    {
        surfaceCache.remove(key);
        imageHash.remove(slideIndex);
        if (!blankSurface)
        {
            int sw = state->slideWidth;
            int sh = state->slideHeight;

#ifdef PICTUREFLOW_QT4
            QImage img = QImage(sw, sh, QImage::Format_RGB32);

            QPainter painter(&img);
            QPoint p1(sw*4 / 10, 0);
            QPoint p2(sw*6 / 10, sh);
            QLinearGradient linearGrad(p1, p2);
            linearGrad.setColorAt(0, Qt::black);
            linearGrad.setColorAt(1, Qt::white);
            painter.setBrush(linearGrad);
            painter.fillRect(0, 0, sw, sh, QBrush(linearGrad));

            painter.setPen(QPen(QColor(64, 64, 64), 4));
            painter.setBrush(QBrush());
            painter.drawRect(2, 2, sw - 3, sh - 3);
            painter.end();
#endif
#if defined(PICTUREFLOW_QT3) || defined(PICTUREFLOW_QT2)
            QPixmap pixmap(sw, sh, 32);
            QPainter painter(&pixmap);
            painter.fillRect(pixmap.rect(), QColor(192, 192, 192));
            painter.fillRect(5, 5, sw - 10, sh - 10, QColor(64, 64, 64));
            painter.end();
            QImage img = pixmap.convertToImage();
#endif

            blankSurface = prepareSurface(&img, sw, sh, bgcolor, state->reflectionEffect);
        }
        return blankSurface;
    }

#ifdef PICTUREFLOW_QT4
    bool exist = imageHash.contains(slideIndex);
    if (exist)
        if (img == imageHash.find(slideIndex).value())
#endif
#ifdef PICTUREFLOW_QT3
            bool exist = imageHash.find(slideIndex) != imageHash.end();
    if (exist)
        if (img == imageHash.find(slideIndex).data())
#endif
#ifdef PICTUREFLOW_QT2
            if (img == imageHash[slideIndex])
#endif
                if (surfaceCache.contains(key))
                    return surfaceCache[key];

    QImage* sr = prepareSurface(img, state->slideWidth, state->slideHeight, bgcolor, state->reflectionEffect);
    surfaceCache.insert(key, sr);
    imageHash.insert(slideIndex, img);

    return sr;
}

// Renders a slide to offscreen buffer. Returns a rect of the rendered area.
// col1 and col2 limit the column for rendering.
QRect QxtFlowViewSoftwareRenderer::renderSlide(const SlideInfo &slide, int col1, int col2)
{
    int blend = slide.blend;
    if (!blend)
        return QRect();

    QImage* src = surface(slide.slideIndex);
    if (!src)
        return QRect();

    QRect rect(0, 0, 0, 0);

    int sw = src->height();
    int sh = src->width();
    int h = buffer.height();
    int w = buffer.width();

    if (col1 > col2)
    {
        int c = col2;
        col2 = col1;
        col1 = c;
    }

    col1 = (col1 >= 0) ? col1 : 0;
    col2 = (col2 >= 0) ? col2 : w - 1;
    col1 = qMin(col1, w - 1);
    col2 = qMin(col2, w - 1);

    int zoom = 100;
    int distance = h * 100 / zoom;
    PFreal sdx = fcos(slide.angle);
    PFreal sdy = fsin(slide.angle);
    PFreal xs = slide.cx - state->slideWidth * sdx / 2;
    PFreal ys = slide.cy - state->slideWidth * sdy / 2;
    PFreal dist = distance * PFREAL_ONE;

    int xi = qMax((PFreal)0, ((w * PFREAL_ONE / 2) + fdiv(xs * h, dist + ys)) >> PFREAL_SHIFT);
    if (xi >= w)
        return rect;

    bool flag = false;
    rect.setLeft(xi);
    for (int x = qMax(xi, col1); x <= col2; x++)
    {
        PFreal hity = 0;
        PFreal fk = rays[x];
        if (sdy)
        {
            fk = fk - fdiv(sdx, sdy);
            hity = -fdiv((rays[x] * distance - slide.cx + slide.cy * sdx / sdy), fk);
        }

        dist = distance * PFREAL_ONE + hity;
        if (dist < 0)
            continue;

        PFreal hitx = fmul(dist, rays[x]);
        PFreal hitdist = fdiv(hitx - slide.cx, sdx);

        int column = sw / 2 + (hitdist >> PFREAL_SHIFT);
        if (column >= sw)
            break;
        if (column < 0)
            continue;

        rect.setRight(x);
        if (!flag)
            rect.setLeft(x);
        flag = true;

        int y1 = h / 2;
        int y2 = y1 + 1;
        QRgb* pixel1 = (QRgb*)(buffer.scanLine(y1)) + x;
        QRgb* pixel2 = (QRgb*)(buffer.scanLine(y2)) + x;
        QRgb pixelstep = pixel2 - pixel1;

        int center = (sh / 2);
        int dy = dist / h;
        int p1 = center * PFREAL_ONE - dy / 2;
        int p2 = center * PFREAL_ONE + dy / 2;

        const QRgb *ptr = (const QRgb*)(src->scanLine(column));
        if (blend == 256)
            while ((y1 >= 0) && (y2 < h) && (p1 >= 0))
            {
                *pixel1 = ptr[p1 >> PFREAL_SHIFT];
                *pixel2 = ptr[p2 >> PFREAL_SHIFT];
                p1 -= dy;
                p2 += dy;
                y1--;
                y2++;
                pixel1 -= pixelstep;
                pixel2 += pixelstep;
            }
        else
            while ((y1 >= 0) && (y2 < h) && (p1 >= 0))
            {
                QRgb c1 = ptr[p1 >> PFREAL_SHIFT];
                QRgb c2 = ptr[p2 >> PFREAL_SHIFT];
                *pixel1 = blendColor(c1, bgcolor, blend);
                *pixel2 = blendColor(c2, bgcolor, blend);
                p1 -= dy;
                p2 += dy;
                y1--;
                y2++;
                pixel1 -= pixelstep;
                pixel2 += pixelstep;
            }
    }

    rect.setTop(0);
    rect.setBottom(h - 1);
    return rect;
}

void QxtFlowViewSoftwareRenderer::renderSlides()
{
    int nleft = state->leftSlides.count();
    int nright = state->rightSlides.count();

    QRect r = renderSlide(state->centerSlide);
    int c1 = r.left();
    int c2 = r.right();

    for (int index = 0; index < nleft; index++)
    {
        QRect rs = renderSlide(state->leftSlides[index], 0, c1 - 1);
        if (!rs.isEmpty())
            c1 = rs.left();
    }
    for (int index = 0; index < nright; index++)
    {
        QRect rs = renderSlide(state->rightSlides[index], c2 + 1, buffer.width());
        if (!rs.isEmpty())
            c2 = rs.right();
    }
}

// Render the slides. Updates only the offscreen buffer.
void QxtFlowViewSoftwareRenderer::render()
{
    buffer.fill(state->backgroundColor);
    renderSlides();
    dirty = false;
}








