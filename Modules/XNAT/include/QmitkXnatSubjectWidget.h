/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatSubjectWidget_h
#define QmitkXnatSubjectWidget_h

#include <MitkXNATExports.h>

// Qt
#include <QWidget>
#include <memory>

// CTK XNAT Core
class ctkXnatSubject;

namespace Ui { class QmitkXnatSubjectWidgetControls; }

/**
 * \brief Widget for displaying or editing XNAT subject properties.
 *
 * QmitkXnatSubjectWidget provides a form-based user interface for viewing or creating
 * XNAT subjects. In INFO mode, the widget shows the subject label, date of birth, gender,
 * handedness, height, and weight as read-only fields, along with a breadcrumb showing the
 * parent project. In CREATE mode, all fields are editable.
 *
 * \sa QmitkXnatCreateObjectDialog, QmitkXnatProjectWidget, QmitkXnatExperimentWidget, ctkXnatSubject
 */
class MITKXNAT_EXPORT QmitkXnatSubjectWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Operation mode for the subject widget.
   */
  enum Mode
  {
    INFO,   ///< Display subject properties in read-only mode.
    CREATE  ///< Allow editing of subject properties for creation.
  };

  /**
   * \brief Construct the widget in INFO mode.
   *
   * \param[in] parent Optional parent widget.
   */
  QmitkXnatSubjectWidget(QWidget *parent = nullptr);

  /**
   * \brief Construct the widget in the specified mode.
   *
   * \param[in] mode The operation mode (INFO or CREATE).
   * \param[in] parent Optional parent widget.
   */
  QmitkXnatSubjectWidget(Mode mode, QWidget *parent = nullptr);

  /**
   * \brief Destructor.
   */
  ~QmitkXnatSubjectWidget() override;

  /**
   * \brief Populate the widget fields from the given subject object.
   *
   * Sets the breadcrumb, label, date of birth, gender, handedness, height, and weight
   * fields from the subject's properties.
   *
   * \param[in] subject The XNAT subject whose properties should be displayed.
   * \pre \p subject must not be \c nullptr.
   */
  void SetSubject(ctkXnatSubject *subject);

  /**
   * \brief Retrieve the subject object, optionally updated with user-entered values.
   *
   * In CREATE mode, the returned subject has its label, dob, gender, handedness, height,
   * and weight properties updated from the widget's input fields. In INFO mode, the
   * subject is returned unchanged.
   *
   * \return Pointer to the ctkXnatSubject with current widget values applied.
   */
  ctkXnatSubject *GetSubject() const;

protected:
  std::unique_ptr<Ui::QmitkXnatSubjectWidgetControls> m_Controls; ///< UI controls generated from the .ui form.

private:
  void Init();
  Mode m_Mode;
  ctkXnatSubject *m_Subject;
};

#endif
