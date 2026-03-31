/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDicomImportWidget_h
#define QmitkDicomImportWidget_h

#include <MitkDICOMUIExports.h>

#include <QWidget>
#include <memory>

class ctkFileDialog;
class ctkDICOMDatabase;
class ctkDICOMIndexer;

class QProgressDialog;
class QTemporaryFile;

namespace Ui
{
  class QmitkDicomImportWidget;
}

/**
 * \class QmitkDicomImportWidget
 * \brief Widget for browsing and importing DICOM data from the filesystem.
 *
 * This widget wraps a ctkDICOMTableManager for displaying DICOM patients, studies,
 * and series. It provides a directory browser dialog for scanning DICOM directories,
 * a button to import selected data into the local storage, and a button to view
 * selected series. DICOM files are indexed into a temporary SQLite database.
 *
 * \sa QmitkDicomLocalStorageWidget
 * \ingroup MitkDICOMUIModule
 */
class MITKDICOMUI_EXPORT QmitkDicomImportWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructor.
   * \param[in] parent Optional parent widget.
   */
  explicit QmitkDicomImportWidget(QWidget* parent = nullptr);

  /** \brief Destructor. Closes the temporary database. */
  ~QmitkDicomImportWidget() override;

signals:

  /**
   * \brief Emitted when a request is made to import DICOM data into local storage.
   * \param[in] files The list of file paths for all series in the current selection.
   */
  void Import(const QStringList& files);

  /**
   * \brief Emitted when the View button is clicked to load selected series.
   * \param[in] series A vector of pairs, each containing the first file path of a series
   *             and optionally its DICOM Modality string (tag 0008,0060).
   */
  void ViewSeries(const std::vector<std::pair<std::string, std::optional<std::string>>>& series);

private slots:
  void OnAddToLocalStorageButtonClicked();
  void OnImport(const QString& directory);
  void OnIndexingComplete(int, int, int, int);
  void OnProgress(int value);
  void OnProgressDetail(const QString& detail);
  void OnProgressStep(const QString& step);
  void OnAnySelectionChanged();
  void OnViewButtonClicked();

private:
  void showEvent(QShowEvent* event) override;

  QStringList GetFileNamesFromSelection();
  void SetupProgressDialog();

  bool OpenDatabase();
  void CloseDatabase();

  QTemporaryFile* m_DatabaseFile;
  ctkDICOMDatabase* m_Database;
  ctkDICOMIndexer* m_Indexer;
  ctkFileDialog* m_ImportDialog;
  QProgressDialog* m_ProgressDialog;
  QString m_ProgressStep;

  std::unique_ptr<Ui::QmitkDicomImportWidget> m_Ui;
};

#endif
