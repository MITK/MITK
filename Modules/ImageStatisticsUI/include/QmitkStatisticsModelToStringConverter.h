/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkStatisticsModelToStringConverter_h
#define QmitkStatisticsModelToStringConverter_h

#include <MitkImageStatisticsUIExports.h>
#include <QmitkImageStatisticsTreeModel.h>

/**
 * \brief Converts the content of a QmitkImageStatisticsTreeModel to a delimited string.
 *
 * Iterates over all cells of a QmitkImageStatisticsTreeModel and produces a formatted string
 * suitable for clipboard export or file output. The iteration traverses the tree hierarchy
 * recursively, prepending parent labels with " >> " separators to leaf rows.
 * Row and column delimiters are configurable (defaults: newline and comma).
 * Header data (column captions) can optionally be included.
 *
 * \sa QmitkImageStatisticsTreeModel
 * \sa QmitkImageStatisticsWidget
 */

class MITKIMAGESTATISTICSUI_EXPORT QmitkStatisticsModelToStringConverter
{
public:
  /** \brief Default constructor. Initializes delimiters to newline and comma. */
  QmitkStatisticsModelToStringConverter();

  /**
   * \brief Sets the statistics tree model to convert.
   * \param[in] model Pointer to the tree model. Must not be nullptr when GetString() is called.
   */
  void SetModel(QmitkImageStatisticsTreeModel *model);

  /**
   * \brief Sets the root index from which to start the iteration.
   * \param[in] rootIndex The root model index. Use an invalid QModelIndex for the model root.
   */
  void SetRootIndex(QModelIndex rootIndex);

  /**
   * \brief Converts the model content to a formatted string.
   * \return The string representation of all statistics data.
   * \throw mitk::Exception if the model is nullptr.
   */
  QString GetString() const;

  /**
   * \brief Sets the delimiter used to separate rows.
   * \param[in] rowDelimiter The row delimiter character (default: '\\n').
   */
  void SetRowDelimiter(QChar rowDelimiter);

  /**
   * \brief Sets the delimiter used to separate columns.
   * \param[in] columnDelimiter The column delimiter character (default: ',').
   */
  void SetColumnDelimiter(QChar columnDelimiter);

  /**
   * \brief Sets whether column header data should be included in the output.
   * \param[in] includeHeaderData True to include headers; false to omit them.
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

#endif
