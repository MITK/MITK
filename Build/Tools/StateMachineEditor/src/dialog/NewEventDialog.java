package dialog;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import javax.swing.JTextField;

import dom.DOMGetInstance;
import dom.ReadActionAndEventDOMTree;

public class NewEventDialog extends JDialog {

	private static final long serialVersionUID = 1L;
	private javax.swing.JPanel jContentPane = null;
	private JButton oKButton = null;
	private JButton cancelButton = null;
	private JLabel eventCommentLabel = null;
	private JTextField eventCommentTextField = null;
	private JLabel eventNameLabel = null;
	private JTextField eventNameTextField = null;
	private JLabel eventIdLabel = null;
	private IntegerTextField eventIdTextField = null;
	private boolean isCanceled = false;
	
	/**
	 * This is the default constructor
	 * @param dialog 
	 */
	public NewEventDialog(EventDialog dialog) {
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
		this.setName("NewEventDialog");
		this.setTitle("New event");
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
			eventCommentLabel = new JLabel();
			eventCommentLabel.setBounds(new java.awt.Rectangle(30,70,120,20));
			eventCommentLabel.setText("Event comment:");
			eventNameLabel = new JLabel();
			eventNameLabel.setBounds(new java.awt.Rectangle(30,10,120,20));
			eventNameLabel.setText("Event name:");
			eventIdLabel = new JLabel();
			eventIdLabel.setBounds(new java.awt.Rectangle(30,40,120,20));
			eventIdLabel.setText("Event id:");
			jContentPane = new javax.swing.JPanel();
			jContentPane.setLayout(null);
			jContentPane.setComponentOrientation(java.awt.ComponentOrientation.LEFT_TO_RIGHT);
			jContentPane.add(eventCommentLabel, null);
			jContentPane.add(getEventCommentTextField(), null);
			jContentPane.add(eventNameLabel, null);
			jContentPane.add(getEventNameTextField(), null);
			jContentPane.add(eventIdLabel, null);
			jContentPane.add(getEventIdTextField(), null);
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
	private JTextField getEventCommentTextField() {
		if (eventCommentTextField == null) {
			eventCommentTextField = new JTextField();
			eventCommentTextField.setBounds(new java.awt.Rectangle(160,70,490,20));
			eventCommentTextField.setText("");			
		}
		return eventCommentTextField;
	}

	/**
	 * This method initializes jTextField	
	 * 	
	 * @return javax.swing.JTextField
	 */
	private JTextField getEventNameTextField() {
		if (eventNameTextField == null) {
			eventNameTextField = new JTextField();
			eventNameTextField.setBounds(new java.awt.Rectangle(160,10,490,20));
			eventNameTextField.setText("");
		}
		return eventNameTextField;
	}

	/**
	 * This method initializes jTextField	
	 * 	
	 * @return javax.swing.JTextField
	 */
	private IntegerTextField getEventIdTextField() {
		if (eventIdTextField == null) {
			eventIdTextField = new IntegerTextField();
			eventIdTextField.setBounds(new java.awt.Rectangle(160,40,490,20));
			eventIdTextField.setText("");
		}
		return eventIdTextField;
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
					ReadActionAndEventDOMTree eventTree = DOMGetInstance.getActionAndEventInstance();
					if (eventNameTextField.getText().equals("")) {
						JOptionPane.showMessageDialog(NewEventDialog.this,
								"You have to enter an event name!", "Error Message",
								JOptionPane.ERROR_MESSAGE);
						return;
					}
					else if (eventTree.containsEventName(eventNameTextField.getText())) {
						JOptionPane.showMessageDialog(NewEventDialog.this,
								"Event name already exists, please choose another one!", "Error Message",
								JOptionPane.ERROR_MESSAGE);
						return;
					}
					else if (eventIdTextField.getText().equals("")) {
						JOptionPane.showMessageDialog(NewEventDialog.this,
								"You have to enter an ID!", "Error Message",
								JOptionPane.ERROR_MESSAGE);
						return;
					}
					else if (eventTree.containsEventId(eventIdTextField.getText())) {
						JOptionPane.showMessageDialog(NewEventDialog.this,
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
	 * @return the comment on this new event
	 */
	public String getEventComment() {
		return eventCommentTextField.getText();
	}
	
	/**
	 * @return the name of this new event
	 */
	public String getEventName() {
		return eventNameTextField.getText();
	}
	
	/**
	 * @return the ID of this new event
	 */
	public String getEventId() {
		return eventIdTextField.getText();
	}
	
	
	/**
	 * @return returns whether this dialog is canceled
	 */
	public boolean isCanceled() {
		return isCanceled;
	}
}