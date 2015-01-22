Reader and Writer {#ReaderWriterPage}
=================

This page is work in progress.

## Mime Types

The MimeType class provides meta-data about a specific data format.
Every mitk::IFileReader and mitk::IFileWriter instance must be associated
with exactly one mime-type via a service property.

Mime-type are used for categorizing data formats and creating filter
strings in file open and save dialogs. Hence they effectively control
the accessible set of data formats from a graphical user interface.

## The IFileReader interface

This is the main interface for reading data from files or streams.
However, instances of this interface are rarely used directly.
See the mitk::IOUtil section below.

## The IFileWriter interface

This is the main interface for writing data to files or streams.
However, instances of this interface are rarely used directly.
See the mitk::IOUtil section below.

## Ranking strategies

Todo.

## Convenience classes

Developers usually do not interact with the service registry directly
to retrieve and select a matching mitk::IFileReader or mitk::IFileWriter instance.

### IOUtil

This class provides convenience methods for loading data into a data
storage or just returning BaseData objects. The mitk::IOUtil::Save()
and mitk::IOUtil::Load() methods cover thy typical use cases and
automatically select the best matching mitk::IFileReader and mitk::IFileWriter
instance.

### %QmitkIOUtil

The QmitkIOUtil class is a wrapper around mitk::IOUtil, providing
file open and save dialogs for selecting a file name from a within
a graphical user interface.

### FileReaderRegistry and FileWriterRegistry

Access to mitk::IFileReader and mitk::IFileWriter objects and their service references.

## Integrating external I/O mechanisms

The MITK I/O system integrates with several external I/O systems.

### ITK Reader and Writer

The I/O classes from ITK are automatically added to the service registry
of MITK. They can be transparently used via the mitk::IFileReader and mitk::IFileWriter
interfaces or the mitk::IOUtil methods.

### VTK Reader and Writer

VTK does not provide a mechanism to enumerate all available I/O classes.
Hence MITK provides manual integration of a specific set of VTK readers
and writers.

## Error handling and recovery

Todo.
