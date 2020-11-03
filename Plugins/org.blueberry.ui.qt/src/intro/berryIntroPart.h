/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYINTROPART_H_
#define BERRYINTROPART_H_

#include "berryIIntroPart.h"

#include "berryIIntroSite.h"

#include <berryIConfigurationElement.h>
#include <berryIExecutableExtension.h>
#include <berryIPropertyChangeListener.h>

#include <QIcon>

namespace berry
{

/**
 * Abstract base implementation of an intro part.
 * <p>
 * Subclasses must implement the following methods:
 * <ul>
 * <li><code>CreatePartControl</code>- to create the intro part's controls
 * </li>
 * <li><code>SetFocus</code>- to accept focus</li>
 * <li><code>StandbyStateChanged</code>- to change the standby mode</li>
 * </ul>
 * </p>
 * <p>
 * Subclasses may extend or reimplement the following methods as required:
 * <ul>
 * <li><code>SetInitializationData</code>- extend to provide additional
 * initialization when the intro extension is instantiated</li>
 * <li><code>Init(IIntroSite::Pointer, IMemento::Pointer)</code>- extend to provide additional
 * initialization when intro is assigned its site</li>
 * <li><code>GetAdapter</code>- reimplement to make their intro adaptable
 * </li>
 * </ul>
 * </p>
 */
class BERRY_UI_QT IntroPart: public QObject, public IIntroPart, public IExecutableExtension
{

  Q_OBJECT
  Q_INTERFACES(berry::IIntroPart berry::IExecutableExtension)

private:

  IConfigurationElement::Pointer configElement;

  QIcon imageDescriptor;

  IIntroSite::Pointer partSite;

  QString titleLabel;

  IPropertyChangeListener::Events propChangeEvents;

  /**
   * Return the default title string.
   *
   * @return the default title string
   */
  QString GetDefaultTitle() const;

protected:

  /**
   * Fires a property changed event.
   *
   * @param propertyId
   *            the id of the property that changed
   */
  void FirePropertyChange(int propertyId);

  /**
   * Returns the configuration element for this part. The configuration
   * element comes from the plug-in registry entry for the extension defining
   * this part.
   *
   * @return the configuration element for this part
   */
  IConfigurationElement::Pointer GetConfigurationElement();

  /**
   * Returns the default title image.
   *
   * @return the default image
   */
  QIcon GetDefaultImage() const;

  /**
   * Sets the part site.
   * <p>
   * Subclasses must invoke this method from {@link org.eclipse.ui.intro.IIntroPart#init(IIntroSite, IMemento)}.
   * </p>
   *
   * @param site the intro part site
   */
  void SetSite(IIntroSite::Pointer site);

  /**
   * Sets or clears the title image of this part.
   *
   * @param titleImage
   *            the title image, or <code>null</code> to clear
   */
  void SetTitleImage(const QIcon& titleImage);

  /**
   * Set the title string for this part.
   *
   * @param titleLabel the title string.  Must not be <code>null</code>.
   * @since 3.2
   */
  void SetTitle(const QString& titleLabel);

public:

  /* (non-Javadoc)
   * @see org.eclipse.ui.intro.IIntroPart#addPropertyListener(org.eclipse.ui.IPropertyListener)
   */
  void AddPropertyListener(IPropertyChangeListener* l) override;

  /**
   * The <code>IntroPart</code> implementation of this
   * <code>IIntroPart</code> method disposes the title image loaded by
   * <code>setInitializationData</code>. Subclasses may extend.
   */
  ~IntroPart() override;

  /**
   * This implementation of the method declared by <code>IAdaptable</code>
   * passes the request along to the platform's adapter manager; roughly
   * <code>Platform.getAdapterManager().getAdapter(this, adapter)</code>.
   * Subclasses may override this method (however, if they do so, they should
   * invoke the method on their superclass to ensure that the Platform's
   * adapter manager is consulted).
   */
  //    Object getAdapter(Class adapter) {
  //        return Platform.getAdapterManager().getAdapter(this, adapter);
  //    }


  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.intro.IIntroPart#getIntroSite()
   */
  IIntroSite::Pointer GetIntroSite() const override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.intro.IIntroPart#getTitleImage()
   */
  QIcon GetTitleImage() const override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.intro.IIntroPart#getTitle()
   */
  QString GetPartName() const override;

  /**
   * The base implementation of this {@link org.eclipse.ui.intro.IIntroPart}method ignores the
   * memento and initializes the part in a fresh state. Subclasses may extend
   * to perform any state restoration, but must call the super method.
   *
   * @param site
   *            the intro site
   * @param memento
   *            the intro part state or <code>null</code> if there is no
   *            previous saved state
   * @exception PartInitException
   *                if this part was not initialized successfully
   */
  void Init(IIntroSite::Pointer site, IMemento::Pointer memento) override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.intro.IIntroPart#removePropertyListener(org.eclipse.ui.IPropertyListener)
   */
  void RemovePropertyListener(IPropertyChangeListener* l) override;

  /**
   * The base implementation of this {@link org.eclipse.ui.intro.IIntroPart} method does nothing.
   * Subclasses may override.
   *
   * @param memento
   *            a memento to receive the object state
   */
  void SaveState(IMemento::Pointer memento) override;

  /**
   * The <code>IntroPart</code> implementation of this
   * <code>IExecutableExtension</code> records the configuration element in
   * and internal state variable (accessible via <code>getConfigElement</code>).
   * It also loads the title image, if one is specified in the configuration
   * element. Subclasses may extend.
   *
   * Should not be called by clients. It is called by the core plugin when
   * creating this executable extension.
   */
  void SetInitializationData(const IConfigurationElement::Pointer& cfig,
                             const QString& propertyName, const Object::Pointer& data) override;

};

}

#endif /* BERRYINTROPART_H_ */
