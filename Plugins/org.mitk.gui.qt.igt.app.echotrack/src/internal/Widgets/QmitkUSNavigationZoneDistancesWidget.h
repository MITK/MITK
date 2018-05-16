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

#ifndef QMITKUSNAVIGATIONZONEDISTANCESWIDGET_H
#define QMITKUSNAVIGATIONZONEDISTANCESWIDGET_H

#include <QWidget>

#include "mitkNumericTypes.h"

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
  class NavigationData;
  class DataNode;
}

class QmitkZoneProgressBar;

/**
 * \brief Widget for showing distances to given zones using instances of QmitkZoneProgressBar.
 * The zones can be added by AddZone() and removed by ClearZones(). To update
 * the progress bars, UpdateDistancesToNeedlePosition() has to be called with
 * a navigation data.
 */
class QmitkUSNavigationZoneDistancesWidget : public QWidget
{
  Q_OBJECT

signals:
  /** \brief Emmited whenever a the distance to a zone falls below zero. */
  void SignalZoneViolated(const mitk::DataNode*, mitk::Point3D);

public:
  explicit QmitkUSNavigationZoneDistancesWidget(QWidget *parent = 0);
  ~QmitkUSNavigationZoneDistancesWidget();

  /**
   * \brief Set the key for the data node float property holding the radius of a zone.
   * The default value of this attribute is "zone.size".
   */
  void SetSizePropertyKey(const std::string& sizePropertyKey);

  /** \brief Get the key for the data node float property holding the radius of a zone. */
  const std::string& GetSizePropertyKey() const;

  /** \brief Adds a zone to the widget. */
  void AddZone(itk::SmartPointer<mitk::DataNode> zoneNode);

  /** \brief Removes all zones from the widget. */
  void ClearZones();

  /** \brief Updates color and label of each zone distance bar according to the given needle position. */
  void UpdateDistancesToNeedlePosition(itk::SmartPointer<mitk::NavigationData> needle);

protected:
  std::string                                     m_SizePropertyKey;
  QVector<itk::SmartPointer<mitk::DataNode> >     m_ZoneNodes;
  QVector<QmitkZoneProgressBar*>                  m_ZoneProgressBars;
};

#endif // QMITKUSNAVIGATIONZONEDISTANCESWIDGET_H
