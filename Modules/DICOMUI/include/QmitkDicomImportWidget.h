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
 * \brief Wrapper widget for a ctkDICOMTableManager and a few extra buttons for
 *        importing DICOM data from the hard drive.
 */
class MITKDICOMUI_EXPORT QmitkDicomImportWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkDicomImportWidget(QWidget* parent = nullptr);
  ~QmitkDicomImportWidget() override;

signals:

  /**
   * \brief Emitted when a request is made to import DICOM data into local storage.
   *
   * \param files The list of files associated with the current selection.
   */
  void Import(const QStringList& files);

  /**
   * \brief Emitted when the View button is clicked.
   *
   * \param series A vector of pairs containing the first file of each series
                   and optionally its modality (DICOM tag (0008,0060)).
   */
  void ViewSeries(const std::vector<std::pair<std::string, std::optional<std::string>>>& series);

private slots:
  void OnAddToLocalStorageButtonClicked();
  void OnImport(const QString& directory);
  void OnIndexingComplete(int, int, int, int);
  void OnProgress(int value);
  void OnProgressDetail(const QString& detail);
  void OnProgressStep(const QString& step);
  void OnSeriesSelectionChanged(const QStringList& selection);
  void OnViewButtonClicked();

private:
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

  Ui::QmitkDicomImportWidget* m_Ui;
};

#endif
