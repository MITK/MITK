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
#include <memory>

class ctkDICOMDatabase;
class ctkDICOMIndexer;

namespace Ui
{
  class QmitkDicomLocalStorageWidget;
}

/**
 * \class QmitkDicomLocalStorageWidget
 * \brief Widget for managing a persistent local DICOM storage database.
 *
 * This widget wraps a ctkDICOMTableManager for browsing patients, studies, and series
 * stored in a local SQLite-based DICOM database. It provides buttons to view selected
 * series and to delete selected patients, studies, or series from the database.
 * Files can be imported into the local database via the OnImport() slot.
 *
 * \sa QmitkDicomImportWidget
 * \ingroup MitkDICOMUIModule
 */
class MITKDICOMUI_EXPORT QmitkDicomLocalStorageWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructor.
   * \param[in] parent Optional parent widget.
   */
  explicit QmitkDicomLocalStorageWidget(QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkDicomLocalStorageWidget() override;

  /**
   * \brief Sets the directory for the local storage DICOM database.
   *
   * This method must be called to fully initialize the widget. It creates or opens
   * the SQLite database in the specified directory and connects it to the internal
   * ctkDICOMTableManager and ctkDICOMIndexer.
   *
   * The returned database pointer can be passed to a ctkDICOMQueryRetrieveWidget
   * as the retrieve database.
   *
   * \param[in] databaseDirectory Path to a directory used for storing
   *            the local DICOM database. Created if it does not exist.
   * \return A shared pointer to the ctkDICOMDatabase.
   */
  QSharedPointer<ctkDICOMDatabase> SetDatabaseDirectory(const QString& databaseDirectory);

signals:
  /**
   * \brief Emitted when the indexing of newly imported DICOM data is complete.
   */
  void IndexingComplete();

  /**
   * \brief Emitted when the View button is clicked to load selected series.
   * \param[out] series A vector of pairs, each containing the first file path of a series
   *             and optionally its DICOM Modality string (tag 0008,0060).
   */
  void ViewSeries(const std::vector<std::pair<std::string, std::optional<std::string>>>& series);

public slots:
  /**
   * \brief Imports the given DICOM files into the local database.
   *
   * The files are indexed asynchronously. The IndexingComplete() signal is
   * emitted when the indexing finishes.
   *
   * \param[in] files List of DICOM file paths to import.
   * \pre The local database must be open (SetDatabaseDirectory() must have been called).
   */
  void OnImport(const QStringList &files);

private slots:
  void OnViewButtonClicked();
  void OnDeleteButtonClicked();
  void OnAnySelectionChanged();

private:
  void showEvent(QShowEvent* event) override;

  bool DeletePatients();
  bool DeleteStudies();
  bool DeleteSeries();

  QSharedPointer<ctkDICOMDatabase> m_LocalDatabase;
  std::unique_ptr<ctkDICOMIndexer> m_LocalIndexer;
  std::unique_ptr<Ui::QmitkDicomLocalStorageWidget> m_Ui;
};

#endif
