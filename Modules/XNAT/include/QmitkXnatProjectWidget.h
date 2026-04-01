/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatProjectWidget_h
#define QmitkXnatProjectWidget_h

// XNATUI
#include <MitkXNATExports.h>

// Qt
#include <QWidget>
#include <memory>

// CTK XNAT Core
class ctkXnatProject;

namespace Ui { class QmitkXnatProjectWidgetControls; }

/**
 * \brief Widget for displaying or editing XNAT project properties.
 *
 * QmitkXnatProjectWidget provides a form-based user interface for viewing or creating
 * XNAT projects. In INFO mode, the widget shows the project name, description, and
 * principal investigator in read-only fields (the ID field is hidden). In CREATE mode,
 * all fields including the project ID are visible and editable.
 *
 * \sa QmitkXnatCreateObjectDialog, QmitkXnatSubjectWidget, QmitkXnatExperimentWidget, ctkXnatProject
 */
class MITKXNAT_EXPORT QmitkXnatProjectWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Operation mode for the project widget.
   */
  enum Mode
  {
    INFO,   ///< Display project properties in read-only mode.
    CREATE  ///< Allow editing of project properties for creation.
  };

  /**
   * \brief Construct the widget in INFO mode.
   *
   * \param[in] parent Optional parent widget.
   */
  QmitkXnatProjectWidget(QWidget *parent = nullptr);

  /**
   * \brief Construct the widget in the specified mode.
   *
   * \param[in] mode The operation mode (INFO or CREATE).
   * \param[in] parent Optional parent widget.
   */
  QmitkXnatProjectWidget(Mode mode, QWidget *parent = nullptr);

  /**
   * \brief Destructor.
   */
  ~QmitkXnatProjectWidget() override;

  /**
   * \brief Populate the widget fields from the given project object.
   *
   * Sets the name, description, and principal investigator fields from the
   * project's properties.
   *
   * \param[in] project The XNAT project whose properties should be displayed.
   * \pre \p project must not be \c nullptr.
   */
  void SetProject(ctkXnatProject *project);

  /**
   * \brief Retrieve the project object, optionally updated with user-entered values.
   *
   * In CREATE mode, the returned project has its ID, name, and description properties
   * updated from the widget's input fields. In INFO mode, the project is returned unchanged.
   *
   * \return Pointer to the ctkXnatProject with current widget values applied.
   */
  ctkXnatProject *GetProject() const;

protected:
  std::unique_ptr<Ui::QmitkXnatProjectWidgetControls> m_Controls; ///< UI controls generated from the .ui form.

private:
  void Init();
  Mode m_Mode;
  ctkXnatProject *m_Project;
};

#endif
