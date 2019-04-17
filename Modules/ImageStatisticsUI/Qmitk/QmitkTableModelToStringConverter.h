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
\brief Converts the content of a table model to a string

\details
The content of a table model is converted (as-is or transposed) to a string. Each cell of the table
is converted to a string. Default oder: iteration over rows. The line separation delimiter (default:
'\n' and the column separation delimiter (default: ',') can be chosen. It also can be chosen if
the (colum/row) headers should be exported to the string.

By default, the produced string is csv conform
*/

class MITKIMAGESTATISTICSUI_EXPORT QmitkTableModelToStringConverter
{

public:
  QmitkTableModelToStringConverter();

  void SetTableModel(QAbstractItemModel* model);
  /**
  \brief Returns the string
  \exception throws exception if model is nullptr
  */
  QString GetString() const;
  /**
  \brief If the table should be transposed (iterate over columns instead of rows)
  */
  void SetTransposeOutput(bool transposeOutput);
  void SetLineDelimiter(QChar lineDelimiter);
  void SetColumnDelimiter(QChar columnDelimiter);
  /**
  \brief If header data (column/row captions) are exported
  */
  void SetIncludeHeaderData(bool includeHeaderData);

private:
  QString GetStringTransposed() const;
  QString GetStringNonTransposed() const;

  QAbstractItemModel* m_tableModel=nullptr;
  bool m_transposeOutput=false;
  QChar m_lineDelimiter = '\n';
  bool m_includeHeaderData = false;
  QString m_columnDelimiterWithSpace = ", ";
};
#endif // QmitkTableModelToQString_H__INCLUDED
