/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDicomExternalDataWidget_h
#define QmitkDicomExternalDataWidget_h

#include "ui_QmitkDicomExternalDataWidgetControls.h"
#include <MitkDICOMUIExports.h>

// include QT
#include <QHash>
#include <QLabel>
#include <QProgressDialog>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QWidget>

class ctkFileDialog;
class ctkDICOMDatabase;
class ctkDICOMIndexer;

/**
* \brief QmitkDicomExternalDataWidget is a QWidget providing functionality for dicom import.
*
* \ingroup Functionalities
*/
class MITKDICOMUI_EXPORT QmitkDicomExternalDataWidget : public QWidget
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string Widget_ID;

  /**
  * \brief QmitkDicomExternalDataWidget(QWidget *parent) constructor.
  *
  * \param parent is a pointer to the parent widget
  */
  QmitkDicomExternalDataWidget(QWidget *parent);

  /**
  * \brief QmitkDicomExternalDataWidget destructor.
  */
  ~QmitkDicomExternalDataWidget() override;

  /**
  * \brief CreateQtPartControl(QWidget *parent) sets the view objects from ui_QmitkDicomExternalDataWidgetControls.h.
  *
  * \param parent is a pointer to the parent widget
  */
  virtual void CreateQtPartControl(QWidget *parent);

  /**
  * \brief Initializes the widget. This method has to be called before widget can start.
  */
  void Initialize();

signals:

  /// @brief emitted when import into database is finished.
  void SignalStartDicomImport(const QStringList &);

  /// @brief emitted when view button is clicked.
  void SignalDicomToDataManager(QHash<QString, QVariant>);

public slots:

  /// @brief Called when download button was clicked.
  void OnDownloadButtonClicked();

  /// @brief Called when view button was clicked.
  void OnViewButtonClicked();

  /// @brief   Called when adding a dicom directory. Starts a thread adding the directory.
  void OnStartDicomImport(const QString &);

  void OnSeriesSelectionChanged(const QStringList &s);

protected slots:
  void OnProgressStep(const QString&);

  void OnProgressDetail(const QString&);

  void OnProgress(int value);

  void OnIndexingComplete(int, int, int, int);

protected:
  /// \brief Get the list of filepath from current selected index in TreeView. All file paths referring to the index
  /// will be returned.
  QStringList GetFileNamesFromIndex();

  /// \brief SetupImportDialog Sets up import dialog.
  void SetupImportDialog();

  void SetupProgressDialog();

  ctkDICOMDatabase *m_ExternalDatabase;
  ctkDICOMIndexer *m_ExternalIndexer;
  ctkFileDialog *m_ImportDialog;

  QProgressDialog *m_ProgressDialog;
  QString m_LastImportDirectory;
  QString m_ProgressStep;

  Ui::QmitkDicomExternalDataWidgetControls *m_Controls;
};

#endif
