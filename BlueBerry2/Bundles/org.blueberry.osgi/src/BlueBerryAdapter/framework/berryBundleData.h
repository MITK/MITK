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


#ifndef BERRYBUNDLEDATA_H_
#define BERRYBUNDLEDATA_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include <osgi/framework/Version.h>
#include <osgi/framework/Bundle.h>
#include <osgi/framework/Exceptions.h>
#include <osgi/util/Dictionary.h>

#include <Poco/URI.h>
#include <Poco/File.h>


namespace berry {
namespace osgi {
namespace framework {

using namespace ::osgi::framework;

/**
 * The <code>BundleData</code> represents a single bundle that is persistently
 * stored by a <code>FrameworkAdaptor</code>.  A <code>BundleData</code> creates
 * the ClassLoader for a bundle, finds native libraries installed in the
 * FrameworkAdaptor for the bundle, creates data files for the bundle,
 * used to access bundle entries, manifest information, and getting and saving
 * metadata.
 * <p>
 * Clients may implement this interface.
 * </p>
 * @since 3.1
 */
struct BundleData : public Object {

  osgiObjectMacro(berry::osgi::framework::BundleData)

  enum Type {
  /** The BundleData is for a framework extension bundle */
  TYPE_FRAMEWORK_EXTENSION = 0x00000002,
  /** The BundleData is for a singleton bundle */
  TYPE_SINGLETON = 0x00000008,
  /** The BundleData is for a composite bundle */
  TYPE_COMPOSITEBUNDLE = 0x00000020,
  /** The BundleData is for a composite bundle surrogate */
  TYPE_SURROGATEBUNDLE = 0x00000040
  };

  /**
   * Gets a <code>URL</code> to the bundle entry specified by path.
   * This method must not use the BundleClassLoader to find the
   * bundle entry since the ClassLoader will delegate to find the resource.
   * @see org.osgi.framework.Bundle#getEntry(String)
   * @param path The bundle entry path.
   * @return A URL used to access the entry or null if the entry
   * does not exist.
   */
  virtual Poco::URI GetEntry(const std::string path) = 0;

  /**
   * Gets all of the bundle entries that exist under the specified path.
   * For example: <p>
   * <code>getEntryPaths("/META-INF")</code> <p>
   * This will return all entries from the /META-INF directory of the bundle.
   * @see org.osgi.framework.Bundle#getEntryPaths(String path)
   * @param path The path to a directory in the bundle.
   * @return An Enumeration of the entry paths or null if the specified path
   * does not exist.
   */
  virtual std::vector<Poco::URI> GetEntryPaths(const std::string& path) = 0;

  /**
   * Returns the absolute path name of a native library. The BundleData
   * ClassLoader invokes this method to locate the native libraries that
   * belong to classes loaded from this BundleData. Returns
   * null if the library does not exist in this BundleData.
   * @param libname The name of the library to find the absolute path to.
   * @return The absolute path name of the native library or null if
   * the library does not exist.
   */
  virtual std::string FindLibrary(const std::string& libname) = 0;

  /**
   * Installs the native code paths for this BundleData.  Each
   * element of nativepaths must be installed for lookup when findLibrary
   * is called.
   * @param nativepaths The array of native code paths to install for
   * the bundle.
   * @throws BundleException If any error occurs during install.
   */
  virtual void InstallNativeCode(const std::vector<std::string>& nativepaths) throw(BundleException) = 0;

  /**
   * Return the bundle data directory.
   * Attempt to create the directory if it does not exist.
   *
   * @see org.osgi.framework.BundleContext#getDataFile(String)
   * @return Bundle data directory or null if not supported.
   */

  virtual Poco::File GetDataFile(const std::string& path) = 0;

  /**
   * Return the Dictionary of manifest headers for the BundleData.
   * @return Dictionary that contains the Manifest headers for the BundleData.
   * @throws BundleException if an error occurred while reading the
   * bundle manifest data.
   */
  virtual SmartPointer<Dictionary> GetManifest() throw(BundleException) = 0;

  /**
   * Get the BundleData bundle ID.  This will be used as the bundle
   * ID by the framework.
   * @return The BundleData ID.
   */
  virtual long GetBundleID() = 0;

  /**
   * Get the BundleData Location.  This will be used as the bundle
   * location by the framework.
   * @return the BundleData location.
   */
  virtual std::string GetLocation() = 0;

  /**
   * Get the last time this BundleData was modified.
   * @return the last time this BundleData was modified
   */
  virtual long GetLastModified() = 0;

  /**
   * Close all resources for this BundleData
   * @throws IOException If an error occurs closing.
   */
  virtual void Close() throw(Poco::IOException) = 0;

  /**
   * Open the BundleData. This method will reopen the BundleData if it has been
   * previously closed.
   * @throws IOException If an error occurs opening.
   */
  virtual void Open() throw(Poco::IOException) = 0;

  /**
   * Sets the Bundle object for this BundleData.
   * @param bundle The Bundle Object for this BundleData.
   */
  virtual void SetBundle(SmartPointer<Bundle> bundle) = 0;

  /**
   * Returns the start level metadata for this BundleData.
   * @return the start level metadata for this BundleData.
   */
  virtual int GetStartLevel() = 0;

  /**
   * Returns the status metadata for this BundleData.  A value of 1
   * indicates that this bundle is started persistently.  A value of 0
   * indicates that this bundle is not started persistently.
   * @return the status metadata for this BundleData.
   */
  virtual int GetStatus() = 0;

  /**
   * Sets the start level metatdata for this BundleData.  Metadata must be
   * stored persistently when BundleData.save() is called.
   * @param value the start level metadata
   */
  virtual void SetStartLevel(int value) = 0;

  /**
   * Creates the ClassLoader for the BundleData.  The ClassLoader created
   * must use the <code>ClassLoaderDelegate</code> to delegate class, resource
   * and library loading.  The delegate is responsible for finding any resource
   * or classes imported by the bundle through an imported package or a required
   * bundle. <p>
   * The <code>ProtectionDomain</code> domain must be used by the Classloader when
   * defining a class.
   * @param delegate The <code>ClassLoaderDelegate</code> to delegate to.
   * @param domain The <code>BundleProtectionDomain</code> to use when defining a class.
   * @param bundleclasspath An array of bundle classpaths to use to create this
   * classloader.  This is specified by the Bundle-ClassPath manifest entry.
   * @return The new ClassLoader for the BundleData.
   */
  //BundleClassLoader createClassLoader(ClassLoaderDelegate delegate, BundleProtectionDomain domain, String[] bundleclasspath);


  /**
   * Sets the status metadata for this BundleData.  Metadata must be
   * stored persistently when BundleData.save() is called.
   * @param value the status metadata.
   */
  virtual void SetStatus(int value) = 0;

  /**
   * Persistently stores all the metadata for this BundleData
   * @throws IOException
   */
  virtual void Save() throw(Poco::IOException) = 0;

  /**
   * Returns the Bundle-SymbolicName for this BundleData as specified in the bundle
   * manifest file.
   * @return the Bundle-SymbolicName for this BundleData.
   */
  virtual std::string GetSymbolicName() = 0;

  /**
   * Returns the Bundle-Version for this BundleData as specified in the bundle
   * manifest file.
   * @return the Bundle-Version for this BundleData.
   */
  virtual Version GetVersion() = 0;

  /**
   * Returns the type of bundle this BundleData is for.
   * @return returns the type of bundle this BundleData is for
   */
  virtual Type GetType() = 0;

  /**
   * Returns the Bundle-ClassPath for this BundleData as specified in
   * the bundle manifest file.
   * @return the classpath for this BundleData.
   */
  //virtual String[] getClassPath() throws BundleException;

  /**
   * Returns the Bundle-Activator for this BundleData as specified in
   * the bundle manifest file.
   * @return the Bundle-Activator for this BundleData.
   */
  virtual std::string GetActivator() = 0;

  /**
   * Returns the Bundle-RequiredExecutionEnvironment for this BundleData as
   * specified in the bundle manifest file.
   * @return the Bundle-RequiredExecutionEnvironment for this BundleData.
   */
  //public String getExecutionEnvironment();

  /**
   * Returns the DynamicImport-Package for this BundleData as
   * specified in the bundle manifest file.
   * @return the DynamicImport-Packaget for this BundleData.
   */
  //public String getDynamicImports();
};

}
}
}

#endif /* BERRYBUNDLEDATA_H_ */
