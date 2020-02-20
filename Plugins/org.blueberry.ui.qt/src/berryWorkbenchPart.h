/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __BERRY_WORKBENCH_PART_H__
#define __BERRY_WORKBENCH_PART_H__

#include "berryIWorkbenchPart.h"
#include "berryIWorkbenchPartSite.h"

#include <berryIConfigurationElement.h>
#include <berryIExecutableExtension.h>

#include <QIcon>

namespace berry {


/**
 * \ingroup org_blueberry_ui_qt
 *
 * Abstract base implementation of all workbench parts.
 * <p>
 * This class is not intended to be subclassed by clients outside this
 * package; clients should instead subclass <code>ViewPart</code> or
 * <code>EditorPart</code>.
 * </p>
 *
 * @see org.blueberry.ui.part.ViewPart
 * @see org.blueberry.ui.part.EditorPart
 * @noextend This class is not intended to be subclassed by clients.
 */
class BERRY_UI_QT WorkbenchPart : public QObject,
    public virtual IWorkbenchPart, public IExecutableExtension
{
  Q_OBJECT
  Q_INTERFACES(berry::IExecutableExtension);

public:
  berryObjectMacro(WorkbenchPart, QObject, IWorkbenchPart, IExecutableExtension);

  ~WorkbenchPart() override;

private:
  QString m_Title;
  QIcon m_TitleImage;
  QString m_ToolTip;

  IConfigurationElement::Pointer m_ConfigElement;
  IWorkbenchPartSite::Pointer m_PartSite;
  QString m_PartName;
  QString m_ContentDescription;

  QHash<QString, QString> partProperties;
  IPropertyChangeListener::Events partChangeEvents;

  void InternalSetContentDescription(const QString& description);
  void InternalSetPartName(const QString& partName);

protected:
  WorkbenchPart();

  /**
    * Returns the configuration element for this part. The configuration element
    * comes from the plug-in registry entry for the extension defining this part.
    *
    * @return the configuration element for this part
    */
  IConfigurationElement::Pointer GetConfigurationElement() const
  {
    return m_ConfigElement;
  }

  /**
    * Returns the default title image.
    *
    * @return the default image
    */
//  protected Image getDefaultImage() {
//    return PlatformUI.getWorkbench().getSharedImages().getImage(
//                ISharedImages.IMG_DEF_VIEW);
//  }

  /**
   * Sets the part site.
   * <p>
   * Subclasses must invoke this method from <code>IEditorPart.init</code>
   * and <code>IViewPart.init</code>.
   *
   * @param site the workbench part site
   */
  void SetSite(IWorkbenchPartSite::Pointer site);

  /**
   * Checks that the given site is valid for this type of part.
   * The default implementation does nothing.
   *
   * @param site the site to check
   */
  virtual void CheckSite(IWorkbenchPartSite::Pointer site);

  /**
   * Sets or clears the title image of this part.
   *
   * @param titleImage the title image, or <code>null</code> to clear
   */
  virtual void SetTitleImage(const QIcon& titleImage);

  /**
   * Sets or clears the title tool tip text of this part. Clients should
   * call this method instead of overriding <code>getTitleToolTip</code>
   *
   * @param toolTip the new tool tip text, or <code>null</code> to clear
   */
  virtual void SetTitleToolTip(const QString& toolTip);

  /**
   * Sets the name of this part. The name will be shown in the tab area for
   * the part. Clients should call this method instead of overriding getPartName.
   * Setting this to the empty string will cause a default part name to be used.
   *
   * @param partName the part name, as it should be displayed in tabs.
   */
  virtual void SetPartName(const QString& partName);

  /**
   * Sets the content description for this part. The content description is typically
   * a short string describing the current contents of the part. Setting this to the
   * empty string will cause a default content description to be used. Clients should
   * call this method instead of overriding getContentDescription(). For views, the
   * content description is shown (by default) in a line near the top of the view. For
   * editors, the content description is shown beside the part name when showing a
   * list of editors. If the editor is open on a file, this typically contains the path
   * to the input file, without the filename or trailing slash.
   *
   * @param description the content description
   */
  virtual void SetContentDescription(const QString& description);

  void FirePropertyChanged(const QString& key,
      const QString& oldValue, const QString& newValue);

  void FirePropertyChange(int propertyId);

public:

  /* (non-Javadoc)
   * Method declared on IWorkbenchPart.
   */
  void AddPropertyListener(IPropertyChangeListener* l) override;

  void RemovePropertyListener(IPropertyChangeListener* l) override;

  void SetPartProperty(const QString& key, const QString& value) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IWorkbenchPart3#getPartProperty(java.lang.String)
   */
  QString GetPartProperty(const QString& key) const override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IWorkbenchPart3#getPartProperties()
   */
  const QHash<QString, QString>& GetPartProperties() const override;

  /**
   * {@inheritDoc}
   * The <code>WorkbenchPart</code> implementation of this
   * <code>IExecutableExtension</code> records the configuration element in
   * and internal state variable (accessible via <code>getConfigElement</code>).
   * It also loads the title image, if one is specified in the configuration element.
   * Subclasses may extend.
   *
   * Should not be called by clients. It is called by the core plugin when creating
   * this executable extension.
   */
  void SetInitializationData(const IConfigurationElement::Pointer& cfig,
                             const QString& propertyName, const Object::Pointer& data) override;

  /*
   * Creates the controls for this workbench part.
   * <p>
   * Subclasses must implement this method.  For a detailed description of the
   * requirements see <code>IWorkbenchPart</code>
   * </p>
   *
   * @param parent the parent control
   * @see IWorkbenchPart
   */
  void CreatePartControl(QWidget* parent) override = 0;

  /* (non-Javadoc)
     * Asks this part to take focus within the workbench.
     * <p>
     * Subclasses must implement this method.  For a detailed description of the
     * requirements see <code>IWorkbenchPart</code>
     * </p>
     *
     * @see IWorkbenchPart
     */
  void SetFocus() override = 0;

  /*
   * Method declared on IWorkbenchPart.
   */
  IWorkbenchPartSite::Pointer GetSite() const override;

  /**
   * {@inheritDoc}
   * <p>
   * It is considered bad practise to overload or extend this method.
   * Parts should call setPartName to change their part name.
   * </p>
   */
  QString GetPartName() const override;

  /**
   * {@inheritDoc}
   * <p>
   * It is considered bad practise to overload or extend this method.
   * Parts should call setContentDescription to change their content description.
   * </p>
   */
  QString GetContentDescription() const override;


  /* (non-Javadoc)
   * Method declared on IWorkbenchPart.
   */
  QIcon GetTitleImage() const override;

  /* (non-Javadoc)
   * Gets the title tool tip text of this part.
   *
   * @return the tool tip text
   */
  QString GetTitleToolTip() const override;

};

} // namespace berry

#endif // __BERRY_WORKBENCH_PART_H__

