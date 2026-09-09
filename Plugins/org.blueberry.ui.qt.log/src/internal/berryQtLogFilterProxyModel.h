/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQTLOGFILTERPROXYMODEL_H_
#define BERRYQTLOGFILTERPROXYMODEL_H_

#include <mitkLogLevel.h>

#include <QSortFilterProxyModel>

namespace berry {

/** \brief Filters log messages by severity and by a text pattern.
 *
 * The text search is done here rather than through filterKeyColumn(), which
 * always iterates all columns of the source model and so would match hidden
 * detail columns like the source file path.
 */
class QtLogFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  explicit QtLogFilterProxyModel(QObject* parent = nullptr);
  ~QtLogFilterProxyModel() override;

  /** \brief Hide messages less severe than the given level. */
  void SetMinimumLevel(mitk::LogLevel level);

  /** \brief Include the detail columns in the text search. */
  void SetShowDetails(bool showDetails);

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
  mitk::LogLevel m_MinimumLevel;
  bool m_ShowDetails;
};

}

#endif /*BERRYQTLOGFILTERPROXYMODEL_H_*/
