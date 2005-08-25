
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;


/**
 * This Class first opens and reads the template file at the sprecified path.
 * It them replaces all names by the title entered in the window.
 */
public class CopyTemplate {

  String Title, mitkPath;
  FileReader fReader;
  BufferedWriter bWriter;
  StringWriter sWriter;
  StringReader sReader;
  FileWriter fWriter;
  String fileContent, newFileContent;
  int c;

  public CopyTemplate(String Title, String mitkPath) {
    this.Title = Title;
    this.mitkPath = mitkPath;
  }

public void run()
  throws IOException
  {
    File mitkDir = new File(mitkPath);
    File algDir = new File(mitkDir, "Algorithms");
    File FFIDir = new File(algDir, "mitkFilterFactoryItem");
    File newFFI = new File(FFIDir, "mitk" + Title + "FFI.h");
    File templ = new File(FFIDir, "mitkFilterTemplateFFI.h");
    // open file
    try {
      fReader = new FileReader(templ);
    } catch (IOException e) {
      System.out.println("Fehler bei FileReader("
                + templ.getName()
                + ") @ "
                + FFIDir.getAbsolutePath()
                + "! "
                + e.getMessage());
    }

    // read file and replace title
    try {
      sWriter = new StringWriter();
      while ((c=fReader.read())!= -1) {
        sWriter.write(((char)c));
      }
      fileContent = sWriter.toString();
      newFileContent = fileContent.replaceAll(
          "FilterTemplate", Title);
      newFileContent = newFileContent.replaceAll(
          "FILTERTEMPLATE", Title.toUpperCase());
    } catch (IOException e) {
      System.out.println("Fehler bei StringWriter("
          + templ.getName()
          + ") @ "
          + FFIDir.getAbsolutePath()
          + "! "
          + e.getMessage());
    }

    // save new file
    try {
      sReader = new StringReader(newFileContent);
      fWriter = new FileWriter(newFFI);
      bWriter = new BufferedWriter(fWriter);
      while ((c=sReader.read())!= -1) {
        bWriter.write(((char)c));
      }
      bWriter.close();
      fReader.close();
    } catch (IOException e) {
      System.out.println("Fehler bei FileWriter("
          + newFFI.getName()
          + ") @ "
          + FFIDir.getAbsolutePath()
          + "! "
          + e.getMessage());
    }
  }
}
