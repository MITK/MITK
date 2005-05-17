import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;

import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.border.Border;


/**
 * The tool autmaticaly creates a FilterFactoryItem from a template file 
 * and inserts lines in several files, where the FilterFactoryItems ar listed.
 * This class creates a new window with input fields for text, like directory paths
 * and then starts start.class.
 * 
 */

public class FilterFactoryItemGenerator
extends JFrame
implements ActionListener
{
  JFrame frameO;
  JPanel inputPanel, input1, input2, input2a, input3, input4, buttonPanel;
  JTextField textf1, textf2, textf2a, textf3, textf3a, textf4, textf4a; 
  JLabel label1, label2, label2a, label3, label4, labelOut;
  JButton startButton, exitButton, explorerButton, textB2, textB2a;
  JFileChooser fChooser2, fChooser2a;
  
  public FilterFactoryItemGenerator()
  {
    frameO = new JFrame("FilterFactoryItemGenerator");
    String nameOS = new String(System.getProperty("os.name"));
    String userHome = System.getProperty("user.home");
    if (nameOS.equals("Windows XP")) {
      userHome = "C:" + File.separator + "home"
        + File.separator + System.getProperty("user.name");
    }
    Icon folder = new javax.swing.plaf.metal.MetalIconFactory.TreeFolderIcon();
    Border emptyBorder = new javax.swing.border.EmptyBorder(0,0,0,0);
    
    // Inputfeld
    inputPanel = new JPanel();
    inputPanel.setLayout(new GridLayout(5,1));    
    
    input1 = new JPanel();
    input1.setLayout(new FlowLayout(FlowLayout.RIGHT));
    label1 = new JLabel("Filtername:");
    input1.add(label1);
    textf1 = new JTextField("NewFilter", 35);
    input1.add(textf1);
    inputPanel.add(input1);
    
    input2 = new JPanel();
    input2.setLayout(new FlowLayout(FlowLayout.RIGHT));
    label2 = new JLabel("Mitk Pfad:");
    input2.add(label2);
    fChooser2 = new JFileChooser();
    fChooser2.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
    textB2 = new JButton(folder);
    textB2.setBorder(emptyBorder);
    textB2.addActionListener(this);
    input2.add(textB2);
    textf2 = new JTextField(userHome
        + File.separator
        + "ip++",
        35);
    input2.add(textf2);
    inputPanel.add(input2);
    
    input2a = new JPanel();
    input2a.setLayout(new FlowLayout(FlowLayout.RIGHT));
    label2a = new JLabel("Alle.xml Pfad:");
    input2a.add(label2a);
    fChooser2a = new JFileChooser();
    fChooser2a.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
    textB2a = new JButton(folder);
    textB2a.setBorder(emptyBorder);
    textB2a.addActionListener(this);
    input2a.add(textB2a);
    textf2a = new JTextField(userHome
        + File.separator
        + "ip++"
        + File.separator
        + "AppModules"
        + File.separator
        + "QmitkSegmentationToolBox",
        35);
    input2a.add(textf2a);
    inputPanel.add(input2a);
    
    input3 = new JPanel();
    input3.setLayout(new FlowLayout(FlowLayout.RIGHT));
    label3 = new JLabel("Inputdatentyp, ~dimension");
    input3.add(label3);
    textf3 = new JTextField("float", 27);
    textf3a = new JTextField("3", 3);
    input3.add(textf3);
    input3.add(textf3a);
    inputPanel.add(input3);
    
    input4 = new JPanel();
    input4.setLayout(new FlowLayout(FlowLayout.RIGHT));
    label4 = new JLabel("Inputdatentyp, ~dimension");
    input4.add(label4);
    textf4 = new JTextField("float", 27);
    textf4a = new JTextField("3", 3);
    input4.add(textf4);
    input4.add(textf4a);
    inputPanel.add(input4);
        
    frameO.getContentPane().add("North", inputPanel);
    
    // Buttons
    buttonPanel = new JPanel();
    buttonPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
    
    startButton = new JButton("Start");
    startButton.addActionListener(this);
    buttonPanel.add(startButton);

    explorerButton = new JButton("Explorer");
    explorerButton.addActionListener(this);
    buttonPanel.add(explorerButton);
    
    exitButton = new JButton("Exit");
    exitButton.addActionListener(this);
    buttonPanel.add(exitButton);
    
    labelOut = new JLabel("Programm bereit. (" + nameOS + ")");
    buttonPanel.add(labelOut);
    
    frameO.getContentPane().add("South", buttonPanel);
    
    // [x]-Button
    frameO.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            
    //show
    frameO.setLocation(100,100);
    frameO.pack();
    frameO.setVisible(true);
  }
    
  public void actionPerformed(ActionEvent event)   
  {
    Object cmd = event.getSource();
    String nameOS = new String(System.getProperty("os.name"));
    String explorer = "konqueror ";
    if (nameOS.equals("Windows XP")) {
      explorer = "explorer ";
    }
    
    if (cmd.equals(startButton)) {
      // look for mitkFilterTemplateFFI.h
      File mitkDir = new File(textf2.getText());
      File algDir = new File(mitkDir, "Algorithms");
      File FFIDir = new File(algDir, "mitkFilterFactoryItem");
      File templ = new File(FFIDir, "mitkFilterTemplateFFI.h");
      if (!templ.exists()) {
        labelOut.setText("mitkFilterTemplateFFI.h nicht gefunden!");
      } else {
        // check, if FilterFactoryItem already exits
        File newFFI = new File(FFIDir, "mitk" + textf1.getText() + "FFI.h");
        if (newFFI.exists()) {
          labelOut.setText("Datei \"" + "mitk" + textf1.getText() 
              + "FFI.h" + "\" existiert bereits!");  
        } else {
          labelOut.setText("Programm gestartet...");      
          start prog = new start(textf1.getText(),
                      textf2.getText(),
                      textf2a.getText(),
                      textf3.getText(),
                      textf3a.getText(),
                      textf4.getText(),
                      textf4a.getText());
          try {
            prog.run();
            labelOut.setText("Programm ausgeführt.");
          } catch (IOException e) {
            System.out.println("Fehler beim Ausführen des Programms!");
          } 
        }   
      }
    }  else if (cmd.equals(exitButton)) {
      setVisible(false);
      dispose(); 
      System.exit(0);
    }  else if (cmd.equals(explorerButton)) {
      File mitkDir = new File(textf2.getText());
      File algDir = new File(mitkDir, "Algorithms");
      File FFIDir = new File(algDir, "mitkFilterFactoryItem");
      if (FFIDir.exists()) {
        if (FFIDir.isDirectory()) {
          try {
            Runtime.getRuntime().exec(explorer
                        + FFIDir.getAbsolutePath());
          } catch (Exception e) {
            labelOut.setText("Fehler, kann " + explorer 
                    + "nicht öffnen.");  
          }        
        } else {
          try {
            Runtime.getRuntime().exec(explorer
                + textf2.getText());
          } catch (Exception e) {
            labelOut.setText("Fehler, kann " + explorer 
                + "nicht öffnen.");
          }
        }
      } else {
        try {
          Runtime.getRuntime().exec(explorer
              + textf2.getText());
        } catch (Exception e) {
          labelOut.setText("Fehler, kann " + explorer 
              + "nicht öffnen.");
        }
      }
    }  else if (cmd.equals(textB2)) {
      File curDir = new File(textf2.getText());
      if (curDir.exists()) {
        fChooser2.setCurrentDirectory(curDir);
      }
      int retV = fChooser2.showOpenDialog(FilterFactoryItemGenerator.this);
            if (retV == JFileChooser.APPROVE_OPTION) {
                File file = fChooser2.getSelectedFile();
                textf2.setText(file.getAbsolutePath());
            }
    }   else if (cmd.equals(textB2a)) {
      File curDir = new File(textf2a.getText());
      if (curDir.exists()) {
        fChooser2a.setCurrentDirectory(curDir);
      }
      int retV = fChooser2a.showOpenDialog(FilterFactoryItemGenerator.this);
            if (retV == JFileChooser.APPROVE_OPTION) {
                File file = fChooser2a.getSelectedFile();
                textf2a.setText(file.getAbsolutePath());
            }
    }
  }


  public static void main(String[] args)
  throws IOException{
    FilterFactoryItemGenerator frame = new FilterFactoryItemGenerator();
  }
}
