/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


#ifndef APPLICATIONDESCRIPTOR_H_
#define APPLICATIONDESCRIPTOR_H_

namespace osgi {
namespace service {
namespace application {

/**
 * An OSGi service that represents an installed application and stores
 * information about it. The application descriptor can be used for instance
 * creation.
 *
 * @version $Revision$
 */

public abstract class ApplicationDescriptor {
  /*
   * NOTE: An implementor may also choose to replace this class in
   * their distribution with a class that directly interfaces with the
   * org.osgi.service.application implementation. This replacement class MUST NOT alter the
   * public/protected signature of this class.
   */

  /**
   * The property key for the localized name of the application.
   */
  public static final String APPLICATION_NAME = "application.name";

  /**
   * The property key for the localized icon of the application.
   */
  public static final String APPLICATION_ICON = "application.icon";

  /**
   * The property key for the unique identifier (PID) of the application.
   */
  public static final String APPLICATION_PID = Constants.SERVICE_PID;

  /**
   * The property key for the version of the application.
   */
  public static final String APPLICATION_VERSION = "application.version";

  /**
   * The property key for the name of the application vendor.
   */
  public static final String APPLICATION_VENDOR = Constants.SERVICE_VENDOR;

  /**
   * The property key for the visibility property of the application.
   */
  public static final String APPLICATION_VISIBLE = "application.visible";

  /**
   * The property key for the launchable property of the application.
   */
  public static final String APPLICATION_LAUNCHABLE = "application.launchable";

  /**
   * The property key for the locked property of the application.
   */
  public static final String APPLICATION_LOCKED = "application.locked";

  /**
   * The property key for the localized description of the application.
   */
  public static final String APPLICATION_DESCRIPTION = "application.description";

  /**
   * The property key for the localized documentation of the application.
   */
  public static final String APPLICATION_DOCUMENTATION = "application.documentation";

  /**
   * The property key for the localized copyright notice of the application.
   */
  public static final String APPLICATION_COPYRIGHT = "application.copyright";

  /**
   * The property key for the localized license of the application.
   */
  public static final String APPLICATION_LICENSE = "application.license";

  /**
   * The property key for the application container of the application.
   */
  public static final String APPLICATION_CONTAINER = "application.container";

  /**
   * The property key for the location of the application.
   */
  public static final String APPLICATION_LOCATION = "application.location";

  private final String pid;

  private final boolean[] locked = {false};

  /**
   * Constructs the <code>ApplicationDescriptor</code>.
   *
   * @param applicationId
   *            The identifier of the application. Its value is also available
   *            as the <code>service.pid</code> service property of this
   *            <code>ApplicationDescriptor</code> service. This parameter must not
   *            be <code>null</code>.
   * @throws NullPointerException if the specified <code>applicationId</code> is null.
   */
  protected ApplicationDescriptor(String applicationId) {
    if (null == applicationId) {
      throw new NullPointerException("Application ID must not be null!");
    }

    this.pid = applicationId;
    locked[0] = isPersistentlyLocked();
  }

  /**
   * Returns the identifier of the represented application.
   *
   * @return the identifier of the represented application
   */
  public final String getApplicationId() {
    return pid;
  }

  /**
   * This method verifies whether the specified <code>pattern</code>
   * matches the Distinguished Names of any of the certificate chains
   * used to authenticate this application.
   * <P>
   * The <code>pattern</code> must adhere to the
   * syntax defined in {@link org.osgi.service.application.ApplicationAdminPermission}
   * for signer attributes.
   * <p>
   * This method is used by {@link ApplicationAdminPermission#implies(java.security.Permission)} method
   * to match target <code>ApplicationDescriptor</code> and filter.
   *
   * @param pattern a pattern for a chain of Distinguished Names. It must not be null.
   * @return <code>true</code> if the specified pattern matches at least
   *   one of the certificate chains used to authenticate this application
   * @throws NullPointerException if the specified <code>pattern</code> is null.
   * @throws IllegalStateException if the application descriptor was
   *   unregistered
   */
  public abstract boolean matchDNChain(String pattern);

  /**
   * Returns the properties of the application descriptor as key-value pairs.
   * The return value contains the locale aware and unaware properties as
   * well. The returned <code>Map</code> will include the service
   * properties of this <code>ApplicationDescriptor</code> as well.
   * <p>
   * This method will call the <code>getPropertiesSpecific</code> method
   * to enable the container implementation to insert application model and/or
   * container implementation specific properties.
   * <P>
   * The returned {@link java.util.Map} will contain the standard OSGi service
   * properties as well
   * (e.g. service.id, service.vendor etc.) and specialized application
   * descriptors may offer further service properties. The returned Map contains
   * a snapshot of the properties. It will not reflect further changes in the
   * property values nor will the update of the Map change the corresponding
   * service property.
   *
   * @param locale
   *            the locale string, it may be null, the value null means the
   *            default locale. If the provided locale is the empty String
   *            (<code>""</code>)then raw (non-localized) values are returned.
   *
   * @return copy of the service properties of this application descriptor service,
   *         according to the specified locale. If locale is null then the
   *         default locale's properties will be returned. (Since service
   *         properties are always exist it cannot return null.)
   *
   * @throws IllegalStateException
   *             if the application descriptor is unregistered
   */
  public final Map getProperties(String locale) {
    Map props = getPropertiesSpecific(locale);
    Boolean containerLocked = (Boolean) props.remove(APPLICATION_LOCKED);
    synchronized (locked) {
      if (containerLocked != null && containerLocked.booleanValue() != locked[0]) {
        if (locked[0])
          lockSpecific();
        else
          unlockSpecific();
      }
    }
    /* replace the container's lock with the application model's lock, that's the correct */
    props.put(APPLICATION_LOCKED, new Boolean(locked[0]));
    return props;
  }

  /**
   * Container implementations can provide application model specific
   * and/or container implementation specific properties via this
   * method.
   *
   * Localizable properties must be returned localized if the provided
   * <code>locale</code> argument is not the empty String. The value
   * <code>null</code> indicates to use the default locale, for other
   * values the specified locale should be used.
   *
   * The returned {@link java.util.Map} must contain the standard OSGi service
   * properties as well
   * (e.g. service.id, service.vendor etc.) and specialized application
   * descriptors may offer further service properties.
   * The returned <code>Map</code>
   * contains a snapshot of the properties. It will not reflect further changes in the
   * property values nor will the update of the Map change the corresponding
   * service property.

   * @param locale the locale to be used for localizing the properties.
   * If <code>null</code> the default locale should be used. If it is
   * the empty String (<code>""</code>) then raw (non-localized) values
   * should be returned.
   *
   * @return the application model specific and/or container implementation
   * specific properties of this application descriptor.
   *
   * @throws IllegalStateException
   *             if the application descriptor is unregistered
   */
  protected abstract Map getPropertiesSpecific(String locale);

  /**
   * Launches a new instance of an application. The <code>args</code> parameter specifies
   * the startup parameters for the instance to be launched, it may be null.
   * <p>
   * The following steps are made:
   * <UL>
   * <LI>Check for the appropriate permission.
   * <LI>Check the locking state of the application. If locked then throw
   *     an {@link ApplicationException} with the reason code
   *     {@link ApplicationException#APPLICATION_LOCKED}.
   * <LI>Calls the <code>launchSpecific()</code> method to create and start an application
   * instance.
   * <LI>Returns the <code>ApplicationHandle</code> returned by the
   * launchSpecific()
   * </UL>
   * The caller has to have ApplicationAdminPermission(applicationPID,
   * "launch") in order to be able to perform this operation.
   * <P>
   * The <code>Map</code> argument of the launch method contains startup
   * arguments for the
   * application. The keys used in the Map must be non-null, non-empty <code>String<code>
   * objects. They can be standard or application
   * specific. OSGi defines the <code>org.osgi.triggeringevent</code>
   * key to be used to
   * pass the triggering event to a scheduled application, however
   * in the future it is possible that other well-known keys will be defined.
   * To avoid unwanted clashes of keys, the following rules should be applied:
   * <ul>
   *   <li>The keys starting with the dash (-) character are application
   *       specific, no well-known meaning should be associated with them.</li>
   *   <li>Well-known keys should follow the reverse domain name based naming.
   *       In particular, the keys standardized in OSGi should start with
   *       <code>org.osgi.</code>.</li>
   * </ul>
   * <P>
   * The method is synchronous, it return only when the application instance was
   * successfully started or the attempt to start it failed.
   * <P>
   * This method never returns <code>null</code>. If launching an application fails,
   * the appropriate exception is thrown.
   *
   * @param arguments
   *            Arguments for the newly launched application, may be null
   *
   * @return the registered ApplicationHandle, which represents the newly
   *         launched application instance. Never returns <code>null</code>.
   *
   * @throws SecurityException
   *             if the caller doesn't have "lifecycle"
   *             ApplicationAdminPermission for the application.
   * @throws ApplicationException
   *             if starting the application failed
   * @throws IllegalStateException
   *             if the application descriptor is unregistered
   * @throws IllegalArgumentException
   *             if the specified <code>Map</code> contains invalid keys
   *             (null objects, empty <code>String</code> or a key that is not
   *              <code>String</code>)
   */
  public final ApplicationHandle launch(Map arguments) throws ApplicationException {
    SecurityManager sm = System.getSecurityManager();
    if (sm != null)
      sm.checkPermission(new ApplicationAdminPermission(this, ApplicationAdminPermission.LIFECYCLE_ACTION));
    synchronized (locked) {
      if (locked[0])
        throw new ApplicationException(ApplicationException.APPLICATION_LOCKED, "Application is locked, can't launch!");
    }
    if (!isLaunchableSpecific())
      throw new ApplicationException(ApplicationException.APPLICATION_NOT_LAUNCHABLE, "Cannot launch the application!");
    checkArgs(arguments, false);
    try {
      return launchSpecific(arguments);
    } catch (IllegalStateException ise) {
      throw ise;
    } catch (SecurityException se) {
      throw se;
    } catch (ApplicationException ae) {
      throw ae;
    } catch (Exception t) {
      throw new ApplicationException(ApplicationException.APPLICATION_INTERNAL_ERROR, t);
    }
  }

  /**
   * Called by launch() to create and start a new instance in an application
   * model specific way. It also creates and registeres the application handle
   * to represent the newly created and started instance and registeres it.
   * The method is synchonous, it return only when the application instance was
   * successfully started or the attempt to start it failed.
   * <P>
   * This method must not return <code>null</code>. If launching the application
   * failed, and exception must be thrown.
   *
   * @param arguments
   *            the startup parameters of the new application instance, may be
   *            null
   *
   * @return the registered application model
   *         specific application handle for the newly created and started
   *         instance.
   *
   * @throws IllegalStateException
   *             if the application descriptor is unregistered
   * @throws Exception
   *             if any problem occures.
   */
  protected abstract ApplicationHandle launchSpecific(Map arguments) throws Exception;

  /**
   * This method is called by launch() to verify that according to the
   * container, the application is launchable.
   *
   * @return true, if the application is launchable according to the
   *  container, false otherwise.
   *
   * @throws IllegalStateException
   *             if the application descriptor is unregistered
   */
  protected abstract boolean isLaunchableSpecific();

  /**
   * Schedules the application at a specified event. Schedule information
   * should not get lost even if the framework or the device restarts so it
   * should be stored in a persistent storage. The method registers a
   * {@link ScheduledApplication} service in Service Registry, representing
   * the created schedule.
   * <p>
   * The <code>Map</code> argument of the  method contains startup
   * arguments for the application. The keys used in the Map must be non-null,
   * non-empty <code>String<code> objects. The argument values must be
   * of primitive types, wrapper classes of primitive types, <code>String</code>
   * or arrays or collections of these.
   * <p>
   * The created schedules have a unique identifier within the scope of this
   * <code>ApplicationDescriptor</code>. This identifier can be specified
   * in the <code>scheduleId</code> argument. If this argument is <code>null</code>,
   * the identifier is automatically generated.
   *
   * @param scheduleId
   *             the identifier of the created schedule. It can be <code>null</code>,
   *             in this case the identifier is automatically generated.
   * @param arguments
   *            the startup arguments for the scheduled application, may be
   *            null
   * @param topic
   *            specifies the topic of the triggering event, it may contain a
   *            trailing asterisk as wildcard, the empty string is treated as
   *            "*", must not be null
   * @param eventFilter
   *            specifies and LDAP filter to filter on the properties of the
   *            triggering event, may be null
   * @param recurring
   *            if the recurring parameter is false then the application will
   *            be launched only once, when the event firstly occurs. If the
   *            parameter is true then scheduling will take place for every
   *            event occurrence; i.e. it is a recurring schedule
   *
   * @return the registered scheduled application service
   *
   * @throws NullPointerException
   *             if the topic is <code>null</code>
   * @throws InvalidSyntaxException
   *         if the specified <code>eventFilter</code> is not syntactically correct
   * @throws ApplicationException
   *              if the schedule couldn't be created. The possible error
   *              codes are
   *              <ul>
   *               <li> {@link ApplicationException#APPLICATION_DUPLICATE_SCHEDULE_ID}
   *                 if the specified <code>scheduleId</code> is already used
   *                 for this <code>ApplicationDescriptor</code>
   *               <li> {@link ApplicationException#APPLICATION_SCHEDULING_FAILED}
   *                 if the scheduling failed due to some internal reason
   *                 (e.g. persistent storage error).
   *               <li> {@link ApplicationException#APPLICATION_INVALID_STARTUP_ARGUMENT}
   *                 if the specified startup argument doesn't satisfy the
   *                 type or value constraints of startup arguments.
   *              </ul>
   * @throws SecurityException
   *             if the caller doesn't have "schedule"
   *             ApplicationAdminPermission for the application.
   * @throws IllegalStateException
   *             if the application descriptor is unregistered
   * @throws IllegalArgumentException
   *             if the specified <code>Map</code> contains invalid keys
   *             (null objects, empty <code>String</code> or a key that is not
   *              <code>String</code>)
   */
  public final ScheduledApplication schedule(String scheduleId, Map arguments, String topic, String eventFilter, boolean recurring) throws InvalidSyntaxException, ApplicationException {
    SecurityManager sm = System.getSecurityManager();
    if (sm != null)
      sm.checkPermission(new ApplicationAdminPermission(this, ApplicationAdminPermission.SCHEDULE_ACTION));
    arguments = checkArgs(arguments, true);
    isLaunchableSpecific(); // checks if the ApplicationDescriptor was already unregistered
    return AppPersistence.addScheduledApp(this, scheduleId, arguments, topic, eventFilter, recurring);
  }

  /**
   * Sets the lock state of the application. If an application is locked then
   * launching a new instance is not possible. It does not affect the already
   * launched instances.
   *
   * @throws SecurityException
   *             if the caller doesn't have "lock" ApplicationAdminPermission
   *             for the application.
   * @throws IllegalStateException
   *             if the application descriptor is unregistered
   */
  public final void lock() {
    SecurityManager sm = System.getSecurityManager();
    if (sm != null)
      sm.checkPermission(new ApplicationAdminPermission(this, ApplicationAdminPermission.LOCK_ACTION));
    synchronized (locked) {
      if (locked[0])
        return;
      locked[0] = true;
      lockSpecific();
      saveLock(true);
    }
  }

  /**
   * This method is used to notify the container implementation that the
   * corresponding application has been locked and it should update the
   * <code>application.locked</code> service property accordingly.
   * @throws IllegalStateException
   *             if the application descriptor is unregistered
   */
  protected abstract void lockSpecific();

  /**
   * Unsets the lock state of the application.
   *
   * @throws SecurityException
   *             if the caller doesn't have "lock" ApplicationAdminPermission
   *             for the application.
   * @throws IllegalStateException
   *             if the application descriptor is unregistered
   */
  public final void unlock() {
    SecurityManager sm = System.getSecurityManager();
    if (sm != null)
      sm.checkPermission(new ApplicationAdminPermission(this, ApplicationAdminPermission.LOCK_ACTION));
    synchronized (locked) {
      if (!locked[0])
        return;
      locked[0] = false;
      unlockSpecific();
      saveLock(false);
    }
  }

  /**
   * This method is used to notify the container implementation that the
   * corresponding application has been unlocked and it should update the
   * <code>application.locked</code> service property accordingly.

   * @throws IllegalStateException
   *             if the application descriptor is unregistered
   */
  protected abstract void unlockSpecific();

  private void saveLock(boolean locked) {
    AppPersistence.saveLock(this, locked);
  }

  private boolean isPersistentlyLocked() {
    return AppPersistence.isLocked(this);
  }

  private static final Collection scalars = Arrays.asList(new Class[] {String.class, Integer.class, Long.class, Float.class, Double.class, Byte.class, Short.class, Character.class, Boolean.class});
  private static final Collection scalarsArrays = Arrays.asList(new Class[] {String[].class, Integer[].class, Long[].class, Float[].class, Double[].class, Byte[].class, Short[].class, Character[].class, Boolean[].class});
  private static final Collection primitiveArrays = Arrays.asList(new Class[] {long[].class, int[].class, short[].class, char[].class, byte[].class, double[].class, float[].class, boolean[].class});

  private static Map checkArgs(Map arguments, boolean validateValues) throws ApplicationException {
    if (arguments == null)
      return arguments;
    Map copy = validateValues ? new HashMap() : null;
    for (Iterator entries = arguments.entrySet().iterator(); entries.hasNext();) {
      Map.Entry entry = (Entry) entries.next();
      if (!(entry.getKey() instanceof String))
        throw new IllegalArgumentException("Invalid key type: " + entry.getKey() == null ? "<null>" : entry.getKey().getClass().getName());
      if ("".equals(entry.getKey())) //$NON-NLS-1$
        throw new IllegalArgumentException("Empty string is an invalid key");
      if (validateValues)
        validateValue(entry, copy);
    }
    return validateValues ? copy : arguments;
  }

  private static void validateValue(Map.Entry entry, Map copy) throws ApplicationException {
    Class clazz = entry.getValue().getClass();

    // Is it in the set of scalar types
    if (scalars.contains(clazz)) {
      copy.put(entry.getKey(), entry.getValue());
      return;
    }

    // Is it an array of primitives or scalars
    if (scalarsArrays.contains(clazz) || primitiveArrays.contains(clazz)) {
      int arrayLength = Array.getLength(entry.getValue());
      Object copyOfArray = Array.newInstance(entry.getValue().getClass().getComponentType(), arrayLength);
      System.arraycopy(entry.getValue(), 0, copyOfArray, 0, arrayLength);
      copy.put(entry.getKey(), copyOfArray);
      return;
    }

    // Is it a Collection of scalars
    if (entry.getValue() instanceof Collection) {
      Collection valueCollection = (Collection) entry.getValue();
      for (Iterator it = valueCollection.iterator(); it.hasNext();) {
        Class containedClazz = it.next().getClass();
        if (!scalars.contains(containedClazz)) {
          throw new ApplicationException(ApplicationException.APPLICATION_INVALID_STARTUP_ARGUMENT, "The value for key \"" + entry.getKey() + "\" is a collection that contains an invalid value of type \"" + containedClazz.getName() + "\""); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
        }
      }
      copy.put(entry.getKey(), new ArrayList((Collection) entry.getValue()));
      return;
    }
    throw new ApplicationException(ApplicationException.APPLICATION_INVALID_STARTUP_ARGUMENT, "The value for key \"" + entry.getKey() + "\" is an invalid type \"" + clazz.getName() + "\""); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
  }
};

}
}
}

#endif /* APPLICATIONDESCRIPTOR_H_ */
