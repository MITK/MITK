/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef qclickablelabelhincluded
#define qclickablelabelhincluded

#include "MitkQtWidgetsExtExports.h"
#include <QLabel>

#include <map>
#include <vector>

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
  QClickableLabel(QWidget *parent, Qt::WindowFlags f = nullptr);
  QClickableLabel(const QString &text, QWidget *parent, Qt::WindowFlags f = nullptr);
  ~QClickableLabel() override;

  void AddHotspot(const QString &name, const QRect position);

  void RemoveHotspot(const QString &name);
  void RemoveHotspot(unsigned int hotspotIndex);
  void RemoveAllHotspots();

signals:

  void mousePressed(const QString &hotspotName);
  void mousePressed(unsigned int hotspotIndex);
  void mouseReleased(const QString &hotspotName);
  void mouseReleased(unsigned int hotspotIndex);

protected:
  void mousePressEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

  /// returns index == m_Hotspots.size() if nothing is hit
  unsigned int matchingRect(const QPoint &p);

  typedef std::vector<QRect> RectVectorType;
  RectVectorType m_Hotspots;

  typedef std::map<QString, unsigned int> NameToIndexMapType;
  typedef std::map<unsigned int, QString> IndexToNameMapType;
  NameToIndexMapType m_HotspotIndexForName;
  IndexToNameMapType m_HotspotNameForIndex;
};

#endif
