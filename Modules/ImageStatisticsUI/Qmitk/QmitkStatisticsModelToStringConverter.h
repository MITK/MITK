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
#ifndef QmitkTableModelToQString_H__INCLUDED
#define QmitkTableModelToQString_H__INCLUDED

#include <MitkImageStatisticsUIExports.h>
#include <QAbstractItemModel>

/**
\brief Converts the content of the statistics model to a string

\details
The content of a table model is converted to a string. Each cell is converted to a string. Default 
oder: iteration over rows. The line separation delimiter (default: '\n') and the column separation 
delimiter (default: ',') can be chosen. It also can be chosen if the headers should 
be exported to the string.

By default, the produced string is csv conform
*/

class MITKIMAGESTATISTICSUI_EXPORT QmitkStatisticsModelToStringConverter
{
public:
  QmitkStatisticsModelToStringConverter();

  void SetTableModel(QAbstractItemModel *model);
  void SetRootIndex(QModelIndex rootIndex);
  /**
  \brief Returns the string
  \exception throws exception if model is nullptr
  */
  QString GetString() const;
  void SetRowDelimiter(QChar lineDelimiter);
  void SetColumnDelimiter(QChar columnDelimiter);
  /**
  \brief If header data (column/row captions) are exported
  */
  void SetIncludeHeaderData(bool includeHeaderData);

private:
  /**
\brief Iterates recursively over all cells and returns their content
\details based on https://stackoverflow.com/questions/39153835/how-to-loop-over-qabstractitemview-indexes
*/
  QString Iterate(const QModelIndex &index,
                  const QAbstractItemModel *model,
                  int depth = 0) const;

  QAbstractItemModel *m_statisticsModel = nullptr;
  QModelIndex m_rootIndex;
  QChar m_lineDelimiter = '\n';
  bool m_includeHeaderData = false;
  QString m_columnDelimiterWithSpace = ", ";
};
#endif // QmitkTableModelToQString_H__INCLUDED
