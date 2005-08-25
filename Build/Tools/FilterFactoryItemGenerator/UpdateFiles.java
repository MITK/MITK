import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;

/**
 * This class opens the files
 * mitkFilterFactoryItem.cpp
 * mitkFilterFactoryItem.cmake
 * Alle.xml
 * and inserts lines there, that add the new FilterFactoryItem to the list.
 */

public class UpdateFiles {

  String Title, mitkPath, xmlPath, inputClass, inputDim, outputClass, outputDim;
  FileReader fReader;
  BufferedWriter bWriter;
  StringWriter sWriter;
  StringReader sReader;
  FileWriter fWriter;
  StringBuffer fileContent, newFileContent;
  int c;

  public UpdateFiles(String Title, String mitkPath, String xmlPath,
      String inputClass, String inputDim, String outputClass,
      String outputDim) {
    this.Title = Title;
    this.mitkPath = mitkPath;
    this.xmlPath = xmlPath;
    this.inputClass = inputClass;
    this.inputDim = inputDim;
    this.outputClass = outputClass;
    this.outputDim = outputDim;
  }

  public void run() {
    File mitkDir = new File(mitkPath);
    File algDir = new File(mitkDir, "Algorithms");
    File FFIDir = new File(algDir, "mitkFilterFactoryItem");

    // update mitkFilterFactoryItem.cpp
    File cpp = new File(FFIDir, "mitkFilterFactoryItem.cpp");
    try {
      fReader = new FileReader(cpp);
    } catch (IOException e) {
      System.out.println("Fehler bei FileReader("
                + cpp.getName()
                + ") @ "
                + FFIDir.getAbsolutePath()
                + "! "
                + e.getMessage());
    }

    try {
      sWriter = new StringWriter();
      while ((c=fReader.read())!= -1) {
        sWriter.write(((char)c));
      }
      StringBuffer fileContent = sWriter.getBuffer();

      c = fileContent.indexOf("// Automatic FilterFactoryItemGenerator inserts line \"#define MITK_...\" here, please keep comment");
      newFileContent = fileContent.insert(c,
        "#define MITK_" + Title.toUpperCase() + "_INCLUDE"
        + System.getProperty("line.separator"));

      c = newFileContent.indexOf("// Automatic FilterFactoryItemGenerator inserts lines \"#ifdef MITK_...\" here, please keep comment");
      newFileContent = newFileContent.insert(c,
        "#ifdef MITK_" + Title.toUpperCase() + "_INCLUDE"
        + System.getProperty("line.separator")
        + "#include <mitk" + Title + "FFI.h>"
        + System.getProperty("line.separator")
        + "mitk::" + Title + "FFI< itk::Image<"
        + inputClass + ", " + inputDim + ">, itk::Image<"
        + outputClass + ", " + outputDim + "> > "
        + Title + "FFI_" + inputClass + outputClass
        + "(\"" + Title + "FFI_" + inputClass + "Input_"
        + outputClass + "Output\");"
        + System.getProperty("line.separator")
        + "#endif"
        + System.getProperty("line.separator")
        + System.getProperty("line.separator"));

    } catch (IOException e) {
      System.out.println("Fehler bei StringWriter("
          + cpp.getName()
          +") @ "
          + FFIDir.getAbsolutePath()
          + "! "
          + e.getMessage());
    }

    try {
      sReader = new StringReader(newFileContent.toString());
      fWriter = new FileWriter(cpp);
      bWriter = new BufferedWriter(fWriter);
      while ((c=sReader.read())!= -1) {
        bWriter.write(((char)c));
      }
      bWriter.close();
      sReader.close();
    } catch (IOException e) {
      System.out.println("Fehler bei FileWriter("
          + cpp.getName()
          +") @ "
          + FFIDir.getAbsolutePath()
          + "! "
          + e.getMessage());
    }

    // update mitkFilterFactoryItem.cmake
    File cmake = new File(FFIDir, "mitkFilterFactoryItem.cmake");
    try {
      fReader = new FileReader(cmake);
    } catch (IOException e) {
      System.out.println("Fehler bei FileReader("
                + cmake.getName()
                + ") @ "
                + FFIDir.getAbsolutePath()
                + "! "
                + e.getMessage());
    }

    try {
      sWriter = new StringWriter();
      while ((c=fReader.read())!= -1) {
        sWriter.write(((char)c));
      }
      StringBuffer fileContent = sWriter.getBuffer();

      c = fileContent.indexOf("# Automatic FilterFactoryItemGenerator inserts line \"#SET ( APPMOD_H ${APPMOD_H} ...\" here, please keep comment");
      newFileContent = fileContent.insert(c,
        "SET ( APPMOD_H ${APPMOD_H} ../../Algorithms/mitkFilterFactoryItem/mitk"
        + Title + "FFI.h )"
        + System.getProperty("line.separator"));

    } catch (IOException e) {
      System.out.println("Fehler bei StringWriter("
          + cmake.getName()
          +") @ "
          + FFIDir.getAbsolutePath()
          + "! "
          + e.getMessage());
    }

    try {
      sReader = new StringReader(newFileContent.toString());
      fWriter = new FileWriter(cmake);
      bWriter = new BufferedWriter(fWriter);
      while ((c=sReader.read())!= -1) {
        bWriter.write(((char)c));
      }
      bWriter.close();
      sReader.close();
    } catch (IOException e) {
      System.out.println("Fehler bei FileWriter("
          + cmake.getName()
          +") @ "
          + FFIDir.getAbsolutePath()
          + "! "
          + e.getMessage());
    }

    // update Alle.xml
    File xmlDir = new File(xmlPath);
    File xml = new File(xmlDir, "Alle.xml");
    try {
      fReader = new FileReader(xml);
    } catch (IOException e) {
      System.out.println("Fehler bei FileReader("
                + xml.getName()
                + ") @ "
                + FFIDir.getAbsolutePath()
                + "! "
                + e.getMessage());
    }

    try {
      sWriter = new StringWriter();
      while ((c=fReader.read())!= -1) {
        sWriter.write(((char)c));
      }
      StringBuffer fileContent = sWriter.getBuffer();

      c = fileContent.indexOf("<!-- <newFilterDescription /> -->");
      newFileContent = fileContent.insert(c+35,
        System.getProperty("line.separator")
        + System.getProperty("line.separator")
        + "\t"
        + "<pipe NAME=\""
        + Title + "FFI_" + inputClass + "Input_"
        + outputClass + "Output\" CAPTION=\"XXX\" DESCRIPTION=\"XXX\" INPUTIMAGETYPE=" + inputClass + ">"
        + System.getProperty("line.separator")
        + "\t" + "\t"
        + "<filterDescriptor NAME=\""
        + Title + "FFI_" + inputClass + "Input_"
        + outputClass + "Output\" CAPTION=\"XXX\" DESCRIPTION=\"XXX\">"
        + System.getProperty("line.separator")
        + "\t" + "\t" + "\t"
        + "<inputInteger ID=\"0\" CAPTION=\"XXX\" MIN=\"0\" MAX=\"0\" VALUE=\"0\" DESCRIPTION=\"XXX\" ENABLED=\"TRUE\" VISIBLE=\"TRUE\" />   <!-- please replace values -->"
        + System.getProperty("line.separator")
        + "\t" + "\t"
        + "</filterDescriptor>"
        + System.getProperty("line.separator")
        + "\t"
        + "</pipe>"
        + System.getProperty("line.separator"));


    } catch (IOException e) {
      System.out.println("Fehler bei StringWriter("
          + xml.getName()
          +") @ "
          + FFIDir.getAbsolutePath()
          + "! "
          + e.getMessage());
    }

    try {
      sReader = new StringReader(newFileContent.toString());
      fWriter = new FileWriter(xml);
      bWriter = new BufferedWriter(fWriter);
      while ((c=sReader.read())!= -1) {
        bWriter.write(((char)c));
      }
      bWriter.close();
      sReader.close();
    } catch (IOException e) {
      System.out.println("Fehler bei FileWriter("
          + xml.getName()
          +") @ "
          + FFIDir.getAbsolutePath()
          + "! "
          + e.getMessage());
    }
  }
}
