/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDicomLocalStorageWidget_h
#define QmitkDicomLocalStorageWidget_h

#include <MitkDICOMUIExports.h>
#include <QWidget>

class ctkDICOMDatabase;
class ctkDICOMIndexer;

namespace Ui
{
  class QmitkDicomLocalStorageWidget;
}

/**
 * \brief Wrapper widget for a ctkDICOMTableManager and a few extra buttons for
 *        managing the local storage DICOM database.
 */
class MITKDICOMUI_EXPORT QmitkDicomLocalStorageWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkDicomLocalStorageWidget(QWidget* parent = nullptr);
  ~QmitkDicomLocalStorageWidget() override;

  /**
   * \brief Set the directory for the local storage DICOM database.
   *
   * It is necessary to set the directory for the local storage
   * DICOM database to fully initialize this widget.
   *
   * The returned database pointer should be passed to the
   * ctkDICOMQueryRetrieveWidget as retrieve database.
   *
   * \param databaseDirectory Path to a directory used for storing
   *        the local storage DICOM database.
   *
   * \return A shared pointer to the DICOM database of the wrapped
   *         ctkDICOMTableManager.
   */
  QSharedPointer<ctkDICOMDatabase> SetDatabaseDirectory(const QString& databaseDirectory);

signals:
  /**
   * \brief Emitted when the indexing of newly added DICOM data is complete.
   */
  void IndexingComplete();

  /**
   * \brief Emitted when the View button is clicked.
   *
   * \param series A vector of pairs containing the first file of each series
                   and optionally its modality (DICOM tag (0008,0060)).
   */
  void ViewSeries(const std::vector<std::pair<std::string, std::optional<std::string>>>& series);

public slots:
  /**
   * \brief Starts a thread adding the given DICOM files.
   */
  void OnImport(const QStringList &files);

private slots:
  void OnViewButtonClicked();
  void OnDeleteButtonClicked();
  void OnSeriesSelectionChanged(const QStringList&);

private:
  bool DeletePatients();
  bool DeleteStudies();
  bool DeleteSeries();

  QSharedPointer<ctkDICOMDatabase> m_LocalDatabase;
  std::unique_ptr<ctkDICOMIndexer> m_LocalIndexer;
  Ui::QmitkDicomLocalStorageWidget *m_Ui;
};

#endif
