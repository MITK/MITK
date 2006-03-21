package dialog;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import javax.swing.JTextField;

import dom.DOMGetInstance;
import dom.ReadActionAndEventDOMTree;

public class NewActionDialog extends JDialog {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private javax.swing.JPanel jContentPane = null;
	private JButton oKButton = null;
	private JButton cancelButton = null;
	private JLabel actionCommentLabel = null;
	private JTextField actionCommentTextField = null;
	private JLabel actionNameLabel = null;
	private JTextField actionNameTextField = null;
	private JLabel actionIdLabel = null;
	private IntegerTextField actionIdTextField = null;
	private boolean isCanceled = false;
	
	/**
	 * This is the default constructor
	 * @param dialog 
	 */
	public NewActionDialog(ActionDialog dialog) {
		super(dialog);
		initialize();
	}

	/**
	 * This method initializes this
	 * 
	 * @return void
	 */
	private void initialize() {
		this.setSize(680, 200);
		this.setName("NewActionDialog");
		this.setTitle("New action");
		this.setLocationRelativeTo(null);
		this.setResizable(false);
		this.setModal(true);
		this.setAlwaysOnTop(true);
		this.setComponentOrientation(java.awt.ComponentOrientation.LEFT_TO_RIGHT);
		this.setContentPane(getJContentPane());
	}

	/**
	 * This method initializes jContentPane
	 * 
	 * @return javax.swing.JPanel
	 */
	private javax.swing.JPanel getJContentPane() {
		if(jContentPane == null) {
			actionCommentLabel = new JLabel();
			actionCommentLabel.setBounds(new java.awt.Rectangle(30,70,120,20));
			actionCommentLabel.setText("Action comment:");
			actionNameLabel = new JLabel();
			actionNameLabel.setBounds(new java.awt.Rectangle(30,10,120,20));
			actionNameLabel.setText("Action name:");
			actionIdLabel = new JLabel();
			actionIdLabel.setBounds(new java.awt.Rectangle(30,40,120,20));
			actionIdLabel.setText("Action id:");
			jContentPane = new javax.swing.JPanel();
			jContentPane.setLayout(null);
			jContentPane.setComponentOrientation(java.awt.ComponentOrientation.LEFT_TO_RIGHT);
			jContentPane.add(actionCommentLabel, null);
			jContentPane.add(getActionCommentTextField(), null);
			jContentPane.add(actionNameLabel, null);
			jContentPane.add(getActionNameTextField(), null);
			jContentPane.add(actionIdLabel, null);
			jContentPane.add(getActionIdTextField(), null);
			jContentPane.add(getOKButton(), null);
			jContentPane.add(getCancelButton(), null);
		}
		return jContentPane;
	}

	/**
	 * This method initializes jTextField	
	 * 	
	 * @return javax.swing.JTextField
	 */
	private JTextField getActionCommentTextField() {
		if (actionCommentTextField == null) {
			actionCommentTextField = new JTextField();
			actionCommentTextField.setBounds(new java.awt.Rectangle(160,70,490,20));
			actionCommentTextField.setText("");			
		}
		return actionCommentTextField;
	}

	/**
	 * This method initializes jTextField	
	 * 	
	 * @return javax.swing.JTextField
	 */
	private JTextField getActionNameTextField() {
		if (actionNameTextField == null) {
			actionNameTextField = new JTextField();
			actionNameTextField.setBounds(new java.awt.Rectangle(160,10,490,20));
			actionNameTextField.setText("");
		}
		return actionNameTextField;
	}

	/**
	 * This method initializes jTextField	
	 * 	
	 * @return javax.swing.JTextField
	 */
	private IntegerTextField getActionIdTextField() {
		if (actionIdTextField == null) {
			actionIdTextField = new IntegerTextField();
			actionIdTextField.setBounds(new java.awt.Rectangle(160,40,490,20));
			actionIdTextField.setText("");
		}
		return actionIdTextField;
	}
	
	/**
	 * This method initializes jButton1	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getOKButton() {
		if (oKButton == null) {
			oKButton = new JButton();
			oKButton.setBounds(new java.awt.Rectangle(480,100,80,30));
			oKButton.setText("OK");
			oKButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					ReadActionAndEventDOMTree actionTree = DOMGetInstance.getActionAndEventInstance();
					if (actionNameTextField.getText().equals("")) {
						JOptionPane.showMessageDialog(NewActionDialog.this,
								"You have to enter an action name!", "Error Message",
								JOptionPane.ERROR_MESSAGE);
						return;
					}
					else if (actionTree.containsActionName(actionNameTextField.getText())) {
						JOptionPane.showMessageDialog(NewActionDialog.this,
								"Action name already exists, please choose another one!", "Error Message",
								JOptionPane.ERROR_MESSAGE);
						return;
					}
					else if (actionIdTextField.getText().equals("")) {
						JOptionPane.showMessageDialog(NewActionDialog.this,
								"You have to enter an ID!", "Error Message",
								JOptionPane.ERROR_MESSAGE);
						return;
					}
					else if (actionTree.containsActionId(actionIdTextField.getText())) {
						JOptionPane.showMessageDialog(NewActionDialog.this,
								"ID already exists, please choose another one!", "Error Message",
								JOptionPane.ERROR_MESSAGE);
						return;
					}
					dispose();
				}
			});
		}
		return oKButton;
	}
	
	/**
	 * This method initializes jButton2	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getCancelButton() {
		if (cancelButton == null) {
			cancelButton = new JButton();
			cancelButton.setBounds(new java.awt.Rectangle(570,100,80,30));
			cancelButton.setText("Cancel");
			cancelButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					isCanceled = true;
					dispose();
				}
			});
		}
		return cancelButton;
	}
	
	/**
	 * @return the comment on the new action
	 */
	public String getActionComment() {
		return actionCommentTextField.getText();
	}
	
	/**
	 * @return the name of the new action
	 */
	public String getActionName() {
		return actionNameTextField.getText();
	}
	
	/**
	 * @return the ID of the new action
	 */
	public String getActionId() {
		return actionIdTextField.getText();
	}
	
	/**
	 * @return returns whether this dialog is canceled
	 */
	public boolean isCanceled() {
		return isCanceled;
	}
}