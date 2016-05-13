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

#include <QmlMitkSliderLevelWindowItem.h>

#include <QCursor>
#include <QPainter>
#include <QToolTip>
#include <QMouseEvent>
#include <QDebug>

#include <itkCommand.h>
#include <mitkRenderingManager.h>

#include <math.h>

QmlMitkSliderLevelWindowItem* QmlMitkSliderLevelWindowItem::instance = nullptr;
mitk::DataStorage::Pointer QmlMitkSliderLevelWindowItem::storage = nullptr;

QmlMitkSliderLevelWindowItem::QmlMitkSliderLevelWindowItem( QQuickPaintedItem * parent )
: QQuickPaintedItem(parent)
{
    m_Manager = mitk::LevelWindowManager::New();
    m_Manager->SetDataStorage(QmlMitkSliderLevelWindowItem::storage);

    itk::ReceptorMemberCommand<QmlMitkSliderLevelWindowItem>::Pointer command = itk::ReceptorMemberCommand<QmlMitkSliderLevelWindowItem>::New();
    command->SetCallbackFunction(this, &QmlMitkSliderLevelWindowItem::OnPropertyModified);
    m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
    m_IsObserverTagSet = true;

    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setAntialiasing(true);

    m_Resize = false;
    m_Bottom = false;
    m_CtrlPressed = false;
    m_MouseDown = false;
    m_ScaleVisible = true;

    this->setEnabled(false);

    update();
}

QmlMitkSliderLevelWindowItem::~QmlMitkSliderLevelWindowItem()
{
    if ( m_IsObserverTagSet)
    {
        m_Manager->RemoveObserver(m_ObserverTag);
        m_IsObserverTagSet = false;
    }
}

void QmlMitkSliderLevelWindowItem::setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager)
{
    if ( m_IsObserverTagSet)
    {
        m_Manager->RemoveObserver(m_ObserverTag);
        m_IsObserverTagSet = false;
    }
    m_Manager = levelWindowManager;
    if ( m_Manager.IsNotNull() )
    {
        itk::ReceptorMemberCommand<QmlMitkSliderLevelWindowItem>::Pointer command = itk::ReceptorMemberCommand<QmlMitkSliderLevelWindowItem>::New();
        command->SetCallbackFunction(this, &QmlMitkSliderLevelWindowItem::OnPropertyModified);
        m_ObserverTag = m_Manager->AddObserver(itk::ModifiedEvent(), command);
        m_IsObserverTagSet = true;
    }
}

void QmlMitkSliderLevelWindowItem::OnPropertyModified(const itk::EventObject& )
{
    try
    {
        m_LevelWindow = m_Manager->GetLevelWindow();

        this->m_Level = (int)m_LevelWindow.GetLevel();
        this->m_Window = (int)m_LevelWindow.GetWindow();
        this->setEnabled(true);

        emit this->sync();
        update();
    }
    catch(...)
    {
        this->setEnabled(false);
    }

    QQuickPaintedItem::update();
}

bool QmlMitkSliderLevelWindowItem::isEnabled()
{
    return this->m_Enabled;
}

void QmlMitkSliderLevelWindowItem::setEnabled(bool enable)
{
    this->m_Enabled = enable;
    emit this->enabledChanged();
}

int QmlMitkSliderLevelWindowItem::fontSize() const
{
    return this->m_FontSize;
}

void QmlMitkSliderLevelWindowItem::setFontSize(const int &fontSize)
{
    this->m_FontSize = fontSize;
    this->m_Font.setPointSize(fontSize);
}

QColor QmlMitkSliderLevelWindowItem::fontColor() const
{
    return this->m_FontColor;
}

void QmlMitkSliderLevelWindowItem::setFontColor(const QColor &color)
{
    this->m_FontColor = color;
}

QColor QmlMitkSliderLevelWindowItem::color() const
{
    return this->m_Color;
}

void QmlMitkSliderLevelWindowItem::setColor(const QColor &color)
{
    this->m_Color = color;
}

QColor QmlMitkSliderLevelWindowItem::borderColor() const
{
    return this->m_BorderColor;
}

void QmlMitkSliderLevelWindowItem::setBorderColor(const QColor &color)
{
    this->m_BorderColor = color;
}

void QmlMitkSliderLevelWindowItem::setLevel(int level)
{

    if(level != m_LevelWindow.GetLevel())
    {
        m_LevelWindow.SetLevelWindow(level, m_LevelWindow.GetWindow());
        m_Manager->SetLevelWindow(m_LevelWindow);

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    this->m_Level = level;
}

int QmlMitkSliderLevelWindowItem::getLevel()
{
    return this->m_Level;
}

void QmlMitkSliderLevelWindowItem::setWindow(int window)
{

    if(window != m_LevelWindow.GetWindow())
    {
        m_LevelWindow.SetLevelWindow(m_LevelWindow.GetLevel(), window);
        m_Manager->SetLevelWindow(m_LevelWindow);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    this->m_Window = window;
}

int QmlMitkSliderLevelWindowItem::getWindow()
{
    return this->m_Window;
}

void QmlMitkSliderLevelWindowItem::paint( QPainter* painter )
{
    if(!this->m_Enabled)
        return;

    m_MoveHeight = boundingRect().height() - 55;

    painter->setFont( m_Font );
    painter->setPen(this->m_BorderColor);

    painter->setBrush(this->m_Color);
    painter->drawRoundedRect(m_Rect, 3, 3);

    painter->setPen(this->m_FontColor);

    float mr = m_LevelWindow.GetRange();

    if ( mr < 1 )
        mr = 1;

    float fact = (float) m_MoveHeight / mr;

    //begin draw scale
    if (m_ScaleVisible)
    {
        int minRange = (int)m_LevelWindow.GetRangeMin();
        int maxRange = (int)m_LevelWindow.GetRangeMax();
        int yValue = m_MoveHeight + (int)(minRange*fact);
        QString s = " 0";
        if (minRange <= 0 && maxRange >= 0)
        {
            painter->drawLine( 5, yValue , 15, yValue);
            painter->drawText( 21, yValue + 3, s );
        }

        int count = 1;
        int k = 5;
        bool enoughSpace = false;
        bool enoughSpace2 = false;

        double dStepSize = pow(10,floor(log10(mr/100))+1);

        for(int i = m_MoveHeight + (int)(minRange*fact); i < m_MoveHeight;)//negative
        {
            if (-count*dStepSize < minRange)
                break;
            yValue = m_MoveHeight + (int)((minRange + count*dStepSize)*fact);

            s = QString::number(-count*dStepSize);
            if (count % k && ((dStepSize*fact) > 2.5))
            {
                painter->drawLine( 8, yValue, 12, yValue);
                enoughSpace = true;
            }
            else if (!(count % k))
            {
                if ((k*dStepSize*fact) > 7)
                {
                    painter->drawLine( 5, yValue, 15, yValue);
                    painter->drawText( 21, yValue + 3, s );
                    enoughSpace2 = true;
                }
                else
                {
                    k += 5;
                }
            }
            if (enoughSpace)
            {
                i=yValue;
                count++;
            }
            else if (enoughSpace2)
            {
                i=yValue;
                count += k;
            }
            else
            {
                i=yValue;
                count = k;
            }
        }
        count = 1;
        k = 5;
        enoughSpace = false;
        enoughSpace2 = false;

        for(int i = m_MoveHeight + (int)(minRange*fact); i >= 0;)
        {
            if (count*dStepSize > maxRange)
                break;
            yValue = m_MoveHeight + (int)((minRange - count*dStepSize)*fact);

            s = QString::number(count*dStepSize);
            if(count % k && ((dStepSize*fact) > 2.5))
            {
                if (!(minRange > 0 && (count*dStepSize) < minRange))
                    painter->drawLine( 8, yValue, 12, yValue);
                enoughSpace = true;
            }
            else if (!(count % k))
            {
                if ((k*dStepSize*fact) > 7)
                {
                    if (!(minRange > 0 && (count*dStepSize) < minRange))
                    {
                        painter->drawLine( 5, yValue, 15, yValue);
                        painter->drawText( 21, yValue + 3, s );
                    }
                    enoughSpace2 = true;
                }
                else
                {
                    k += 5;
                }
            }
            if (enoughSpace)
            {
                i=yValue;
                count++;
            }
            else if (enoughSpace2)
            {
                i=yValue;
                count += k;
            }
            else
            {
                i=yValue;
                count = k;
            }
        }
    }
}

void QmlMitkSliderLevelWindowItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    update();
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
}

void QmlMitkSliderLevelWindowItem::hoverMoveEvent(QHoverEvent *mouseEvent)
{
    if ( mouseEvent->pos().y() >= 0
        && mouseEvent->pos().y() <= (m_Rect.topLeft().y() + 3) )
    {
        setCursor(Qt::SizeVerCursor);
        m_UpperBound.setRect(m_Rect.topLeft().x(), m_Rect.topLeft().y() - 3, 17, 7);
        //this->setToolTip("Ctrl + left click to change only upper bound");
        m_Resize = true;
    }
    else if ( mouseEvent->pos().y() >= (m_Rect.bottomLeft().y() - 3) )
    {
        setCursor(Qt::SizeVerCursor);
        m_LowerBound.setRect(m_Rect.bottomLeft().x(), m_Rect.bottomLeft().y() - 3, 17, 7);
        //this->setToolTip("Ctrl + left click to change only lower bound");
        m_Resize = true;
        m_Bottom = true;
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        //this->setToolTip("Left click and mouse move to adjust the slider");
        m_Resize = false;
        m_Bottom = false;
    }
}
/**
 *
 */
void QmlMitkSliderLevelWindowItem::mouseMoveEvent( QMouseEvent* mouseEvent )
{
    if(!mouseEvent && !m_MouseDown)
        return;
    if ( m_LevelWindow.IsFixed() )
        return;


    float fact = (float) m_MoveHeight / m_LevelWindow.GetRange();

    if ( m_Leftbutton )
    {
        if (m_Resize && !m_CtrlPressed)
        {
            double diff = (mouseEvent->pos().y()) / fact;
            diff -= (m_StartPos.y()) / fact;
            m_StartPos = mouseEvent->pos();

            if (diff == 0) return;
            float value;
            if (m_Bottom)
                value = m_LevelWindow.GetWindow() + ( ( 2 * diff ) );
            else
                value = m_LevelWindow.GetWindow() - ( ( 2 * diff ) );

            if ( value < 0 )
                value = 0;

            m_LevelWindow.SetLevelWindow( m_LevelWindow.GetLevel(), value );
        }
        else if(m_Resize && m_CtrlPressed)
        {
            if (!m_Bottom)
            {
                double diff = (mouseEvent->pos().y()) / fact;
                diff -= (m_StartPos.y()) / fact;
                m_StartPos = mouseEvent->pos();

                if (diff == 0) return;
                float value;

                value = m_LevelWindow.GetWindow() - ( ( diff ) );

                if ( value < 0 )
                    value = 0;
                float oldWindow;
                float oldLevel;
                float newLevel;
                oldWindow = m_LevelWindow.GetWindow();
                oldLevel = m_LevelWindow.GetLevel();
                newLevel = oldLevel + (value - oldWindow)/2;
                if (!((newLevel + value/2) > m_LevelWindow.GetRangeMax()))
                    m_LevelWindow.SetLevelWindow( newLevel, value );
            }
            else
            {
                double diff = (mouseEvent->pos().y()) / fact;
                diff -= (m_StartPos.y()) / fact;
                m_StartPos = mouseEvent->pos();

                if (diff == 0) return;
                float value;

                value = m_LevelWindow.GetWindow() + ( ( diff ) );

                if ( value < 0 )
                    value = 0;
                float oldWindow;
                float oldLevel;
                float newLevel;
                oldWindow = m_LevelWindow.GetWindow();
                oldLevel = m_LevelWindow.GetLevel();
                newLevel = oldLevel - (value - oldWindow)/2;
                if (!((newLevel - value/2) < m_LevelWindow.GetRangeMin()))
                    m_LevelWindow.SetLevelWindow( newLevel, value );
            }
        }
        else
        {
            const float minv = m_LevelWindow.GetRangeMin();

            const float level = (m_MoveHeight - mouseEvent->pos().y()) / fact + minv;

            double diff = (mouseEvent->pos().x()) / fact;
            diff -= (m_StartPos.x()) / fact;
            m_StartPos = mouseEvent->pos();

            float window;
            if (m_Bottom)
                window = m_LevelWindow.GetWindow() + ( ( 2 * diff ) );
            else
                window = m_LevelWindow.GetWindow() - ( ( 2 * diff ) );

            if ( window < 0 )
                window = 0;

            m_LevelWindow.SetLevelWindow( level, window );
        }
        m_Manager->SetLevelWindow(m_LevelWindow);
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

}

/**
 *
 */
void QmlMitkSliderLevelWindowItem::mousePressEvent( QMouseEvent* mouseEvent ) {
    if ( m_LevelWindow.IsFixed() )
        return;
    m_MouseDown = true;
    m_StartPos = mouseEvent->pos();

    if ( mouseEvent->button() == Qt::LeftButton )
    {
        if (mouseEvent->modifiers() == Qt::ControlModifier || mouseEvent->modifiers() == Qt::ShiftModifier)
        {
            m_CtrlPressed = true;
        }
        else
        {
            m_CtrlPressed = false;
        }
        m_Leftbutton = true;
    }
    else
        m_Leftbutton = false;

    mouseMoveEvent( mouseEvent );
}

/**
 *
 */
void QmlMitkSliderLevelWindowItem::mouseReleaseEvent( QMouseEvent* )
{
    if ( m_LevelWindow.IsFixed() )
        return;
    m_MouseDown = false;
}

/**
 *
 */
void QmlMitkSliderLevelWindowItem::update()
{

    int rectWidth;
    if(m_ScaleVisible)
    {
        rectWidth = 16;
    }
    else
    {
        rectWidth = 26;
    }

    float mr = m_LevelWindow.GetRange();

    if ( mr < 1 )
        mr = 1;

    float fact = (float) m_MoveHeight / mr;

    float rectHeight = m_LevelWindow.GetWindow() * fact;

    if ( rectHeight < 15 )
        rectHeight = 15;

    if ( m_LevelWindow.GetLowerWindowBound() < 0 )
        m_Rect.setRect( 2, (int) (m_MoveHeight - (m_LevelWindow.GetUpperWindowBound() - m_LevelWindow.GetRangeMin()) * fact) , rectWidth, (int) rectHeight );
    else
        m_Rect.setRect( 2, (int) (m_MoveHeight - (m_LevelWindow.GetUpperWindowBound() - m_LevelWindow.GetRangeMin()) * fact), rectWidth, (int) rectHeight );

    QQuickPaintedItem::update();
}


void QmlMitkSliderLevelWindowItem::hideScale()
{
    m_ScaleVisible = false;
    update();
}

void QmlMitkSliderLevelWindowItem::showScale()
{
    m_ScaleVisible = true;
    update();
}

void QmlMitkSliderLevelWindowItem::setDataStorage(mitk::DataStorage* ds)
{
    m_Manager->SetDataStorage(ds);
}

mitk::LevelWindowManager* QmlMitkSliderLevelWindowItem::GetManager()
{
    return m_Manager.GetPointer();
}

void QmlMitkSliderLevelWindowItem::create(QQmlEngine &engine, mitk::DataStorage::Pointer storage)
{
    qmlRegisterType<QmlMitkSliderLevelWindowItem>("Mitk.Views", 1, 0, "LevelWindow");

    QmlMitkSliderLevelWindowItem::storage = storage;
}
