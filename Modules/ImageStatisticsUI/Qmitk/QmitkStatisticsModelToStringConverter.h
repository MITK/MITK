/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkTableModelToQString_H__INCLUDED
#define QmitkTableModelToQString_H__INCLUDED

#include <MitkImageStatisticsUIExports.h>
#include <QmitkImageStatisticsTreeModel.h>

/**
\brief Converts the content of the statistics model to a string

\details
The content of a QmitkImageStatisticsTreeModel instance is converted to a string. Each cell is converted to a string. Default
order: iteration over rows. The row separation delimiter (default: '\n') and the column separation
delimiter (default: ',') can be chosen. It also can be chosen if the headers should
be exported to the string.
*/

class MITKIMAGESTATISTICSUI_EXPORT QmitkStatisticsModelToStringConverter
{
public:
  QmitkStatisticsModelToStringConverter();

  void SetModel(QmitkImageStatisticsTreeModel *model);
  void SetRootIndex(QModelIndex rootIndex);
  /**
  \brief Returns the string
  \exception throws exception if model is nullptr
  */
  QString GetString() const;
  void SetRowDelimiter(QChar rowDelimiter);
  void SetColumnDelimiter(QChar columnDelimiter);
  /**
  \brief If header data (column/row captions) are exported
  */
  void SetIncludeHeaderData(bool includeHeaderData);

private:

  QString Iterate(const QModelIndex &index,
                  const QmitkImageStatisticsTreeModel *model,
                  QString label = "") const;

  QmitkImageStatisticsTreeModel *m_statisticsModel = nullptr;
  QModelIndex m_rootIndex;

  QChar m_rowDelimiter = '\n';
  QChar m_columnDelimiter = ',';

  bool m_includeHeaderData = false;

};

#endif // QmitkTableModelToQString_H__INCLUDED
