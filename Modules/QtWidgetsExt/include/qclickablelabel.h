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


#ifndef qclickablelabelhincluded
#define qclickablelabelhincluded

#include <QLabel>
#include "MitkQtWidgetsExtExports.h"

#include <vector>
#include <map>

#include "mitkCommon.h"

/**
  \brief A QLabel with multiple hotspots, that can be clicked

  Specially useful in connection with a pixmap.
  Stretched images should be avoided, because the hotspots will not be adjusted in any way.
*/
class MITKQTWIDGETSEXT_EXPORT QClickableLabel : public QLabel
{
  Q_OBJECT

  public:
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QClickableLabel( QWidget* parent, Qt::WFlags f = 0 );
    QClickableLabel( const QString& text, QWidget* parent, Qt::WFlags f = 0 );
#else
    QClickableLabel( QWidget* parent, Qt::WindowFlags f = nullptr );
    QClickableLabel( const QString& text, QWidget* parent, Qt::WindowFlags f = nullptr );
#endif
    virtual ~QClickableLabel();

    void AddHotspot( const QString& name, const QRect position );

    void RemoveHotspot( const QString& name );
    void RemoveHotspot( unsigned int hotspotIndex );
    void RemoveAllHotspots();

  signals:

    void mousePressed( const QString& hotspotName );
    void mousePressed( unsigned int hotspotIndex );
    void mouseReleased( const QString& hotspotName );
    void mouseReleased( unsigned int hotspotIndex );

  protected:

    virtual void mousePressEvent ( QMouseEvent* e ) override;
    virtual void mouseReleaseEvent ( QMouseEvent* e ) override;

    /// returns index == m_Hotspots.size() if nothing is hit
    unsigned int matchingRect( const QPoint& p );

    typedef std::vector< QRect > RectVectorType;
    RectVectorType m_Hotspots;

    typedef std::map< QString, unsigned int > NameToIndexMapType;
    typedef std::map< unsigned int, QString > IndexToNameMapType;
    NameToIndexMapType m_HotspotIndexForName;
    IndexToNameMapType m_HotspotNameForIndex;

};

#endif

