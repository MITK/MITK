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

#ifndef __qmlMitkSliderLevelWindowItem_h
#define __qmlMitkSliderLevelWindowItem_h

#include <MitkQmlItemsExports.h>

#include <mitkLevelWindowManager.h>
#include <mitkDataStorage.h>

#include <QQuickPaintedItem>
#include <QQmlEngine>

class MITKQMLITEMS_EXPORT QmlMitkSliderLevelWindowItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(QColor fontColor READ fontColor WRITE setFontColor)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int level READ getLevel WRITE setLevel NOTIFY levelChanged)
    Q_PROPERTY(int window READ getWindow WRITE setWindow NOTIFY windowChanged)

private:
    static mitk::DataStorage::Pointer storage;

    /// change notifications from the mitkLevelWindowManager
    void OnPropertyModified(const itk::EventObject& e);

    QColor m_Color;
    QColor m_BorderColor;
    QColor m_FontColor;
    QFont  m_Font;

    bool m_Enabled;

    int m_Level;
    int m_Window;

    mitk::LevelWindow m_LevelWindow;
    mitk::LevelWindowManager::Pointer m_Manager;

public:
    static QmlMitkSliderLevelWindowItem* instance;

    QmlMitkSliderLevelWindowItem(QQuickPaintedItem * parent = nullptr);
    ~QmlMitkSliderLevelWindowItem();

    void setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager);
    void setDataStorage(mitk::DataStorage* ds);

    mitk::LevelWindowManager* GetManager();

    QColor color() const;
    void setColor(const QColor &color);

    QColor fontColor() const;
    void setFontColor(const QColor &color);

    QColor borderColor() const;
    void setBorderColor(const QColor &color);

    int getLevel();
    void setLevel(int level);

    int getWindow();
    void setWindow(int window);

    int fontSize() const;
    void setFontSize(const int &fontSize);

    bool isEnabled();
    void setEnabled(bool enable);

    static void create(QQmlEngine &engine, mitk::DataStorage::Pointer storage);

protected:
    QRect m_Rect;
    QPoint m_StartPos;

    QRect m_LowerBound;
    QRect m_UpperBound;

    bool m_Resize;
    bool m_Bottom;
    bool m_MouseDown;
    bool m_Leftbutton;
    bool m_CtrlPressed;
    bool m_ScaleVisible;

    int m_FontSize;
    int m_MoveHeight;

    unsigned long m_ObserverTag;
    bool m_IsObserverTagSet;

    void paint(QPainter* painter) override;

    void mouseMoveEvent(QMouseEvent* mouseEvent ) override;
    void mousePressEvent(QMouseEvent* mouseEvent ) override;
    void mouseReleaseEvent(QMouseEvent* mouseEvent ) override;

    void hoverMoveEvent(QHoverEvent* event) override;
    void geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry) override;

    virtual void update( );

protected slots:
    void hideScale();
    void showScale();

signals:
    void enabledChanged();
    void levelChanged();
    void windowChanged();
    void sync();
};

#endif
