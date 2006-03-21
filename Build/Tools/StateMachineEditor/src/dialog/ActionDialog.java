package dialog;

import java.awt.Component;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import javax.swing.DefaultCellEditor;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JTextField;
import javax.swing.JComboBox;
import javax.swing.JButton;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.plaf.basic.BasicComboBoxRenderer;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableColumn;

import org.jdom.Comment;
import org.jdom.Element;

import dom.DOMGetInstance;
import dom.ReadActionAndEventDOMTree;

import model.Action;

public class ActionDialog extends JDialog {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private javax.swing.JPanel jContentPane = null;
	private JLabel actionCommentLabel = null;
	private JTextField actionCommentTextField = null;
	private JLabel actionCategoryLabel = null;
	private JComboBox actionCategoryComboBox = null;
	private JButton newCategoryButton = null;
	private JTable jTable = null;
	private JButton oKButton = null;
	private JButton cancelButton = null;
	private ActionAndEventInfo info;
	private JButton addParameterButton = null;
	private JButton removeParameterButton = null;
	private JScrollPane jScrollPane = null;
	private DefaultTableModel model;
	private JLabel actionNameLabel = null;
	private JComboBox actionNameComboBox = null;
	private JButton newActionButton = null;
	private Action action = null;
	private ReadActionAndEventDOMTree actionTree = null;
	private String category;
	private List tooltips = new ArrayList();
	
	/**
	 * This is the default constructor for add action
	 */
	public ActionDialog() {
		super();
		initialize();
	}
	
	/**
	 * constructor for change action with action to fill dialog with values
	 * @param action the action according to this dialog
	 */
	public ActionDialog(Action action) {
		super();
		this.action = action;
		initialize();
	}

	/**
	 * This method initializes this
	 * 
	 * @return void
	 */
	private void initialize() {
		actionTree = DOMGetInstance.getActionAndEventInstance();
		this.setSize(680, 360);
		// add action
		if (action == null) {
			this.setName("ActionDialog");
			this.setTitle("Add action");
		}
		// change action
		else {
			this.setName("ChangeActionDialog");
			this.setTitle("Change action");
		}
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
			actionNameLabel = new JLabel();
			actionNameLabel.setBounds(new java.awt.Rectangle(30,40,120,20));
			actionNameLabel.setText("Action name:");
			actionCategoryLabel = new JLabel();
			actionCategoryLabel.setBounds(new java.awt.Rectangle(30,10,120,20));
			actionCategoryLabel.setText("Action category:");
			actionCommentLabel = new JLabel();
			actionCommentLabel.setText("Comment on action:");
			actionCommentLabel.setBounds(new java.awt.Rectangle(30,70,120,20));
			jContentPane = new javax.swing.JPanel();
			jContentPane.setLayout(null);
			jContentPane.setComponentOrientation(java.awt.ComponentOrientation.LEFT_TO_RIGHT);
			jContentPane.add(actionCommentLabel, null);
			jContentPane.add(getActionCommentTextField(), null);
			jContentPane.add(actionCategoryLabel, null);
			jContentPane.add(getActionCategoryComboBox(), null);
			jContentPane.add(getNewCategoryButton(), null);
			jContentPane.add(getNewActionButton(), null);
			jContentPane.add(getOKButton(), null);
			jContentPane.add(getCancelButton(), null);
			jContentPane.add(getAddParameterButton(), null);
			jContentPane.add(getRemoveParameterButton(), null);
			jContentPane.add(getJScrollPane(), null);
			jContentPane.add(actionNameLabel, null);
			jContentPane.add(getActionNameComboBox(), null);
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
			actionCommentTextField.setBounds(new java.awt.Rectangle(160,70,360,20));
			actionCommentTextField.setText("");
			if (!(action == null)) {
				actionCommentTextField.setText(action.getActionComment());
			}
		}
		return actionCommentTextField;
	}

	/**
	 * This method initializes jComboBox	
	 * 	
	 * @return javax.swing.JComboBox	
	 */
	private JComboBox getActionCategoryComboBox() {
		if (actionCategoryComboBox == null) {
			actionCategoryComboBox = new JComboBox();
			actionCategoryComboBox.addItemListener(new java.awt.event.ItemListener() {
				public void itemStateChanged(java.awt.event.ItemEvent e) {
					category = actionCategoryComboBox.getSelectedItem().toString();
					if (!(actionNameComboBox == null)) {
						actionNameComboBox.removeAllItems();
						List allActions = actionTree.getActions(category);
						tooltips.clear();
						for (int i = 0; i < allActions.size(); i++) {
							Element act = (Element) allActions.get(i);
							String action = act.getAttributeValue("NAME");
							actionNameComboBox.addItem(action);
							List commentList = act.getContent();
							// set comments for each category to the tooltip
							String toolTip = "";
							for (int j = 0; j < commentList.size(); j++) {
								Object comEle = commentList.get(j);
								if (comEle instanceof Comment) {
									Comment comment = (Comment) comEle;
									toolTip = comment.getText();
									break;
								}
							}
							if (toolTip.equals("")) {
								toolTip = "No comment available";
							}
							tooltips.add(toolTip);
						}
						actionNameComboBox.setRenderer(new MyComboBoxRenderer());
					}
				}
			});
			actionCategoryComboBox.setMaximumRowCount(5);
			List allCategories = actionTree.getActionCategories();
			for (int i = 0; i < allCategories.size(); i++) {
				actionCategoryComboBox.addItem(allCategories.get(i).toString());
			}
			actionCategoryComboBox.setBounds(new java.awt.Rectangle(160,10,360,20));
			// for change action set correct combobox entry
			if (!(action == null)) {
				String id = action.getActionId();
				if (!(actionTree.getActionCategoryName(actionTree.getActionName(id)) == null)) {
					actionCategoryComboBox.setSelectedItem(actionTree.getActionCategoryName(actionTree.getActionName(id)));
				}
			}
		}
		return actionCategoryComboBox;
	}

	/**
	 * This method initializes jButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getNewCategoryButton() {
		if (newCategoryButton == null) {
			newCategoryButton = new JButton();
			newCategoryButton.setBounds(new java.awt.Rectangle(530,10,120,20));
			newCategoryButton.setText("New category");
			newCategoryButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					while (true) {
						String inputValue = JOptionPane.showInputDialog(ActionDialog.this, "New category name:");
						if (inputValue == null) {
							//cancel
							break;
						}
						else if (!(inputValue == null) && !inputValue.equals("") && !actionTree.containsActionCategory(inputValue)) {
						Object[] options = { "OK", "CANCEL" };
						int option = JOptionPane.showOptionDialog(ActionDialog.this, "Do you really want to add this category?\nAdded categories can not be removed!", "Click OK to continue",
						            JOptionPane.DEFAULT_OPTION, JOptionPane.WARNING_MESSAGE,
						            null, options, options[0]);
							if (option == 0) {
								actionTree.addActionCategory(inputValue);
								actionCategoryComboBox.addItem(inputValue);
								actionCategoryComboBox.setSelectedItem(inputValue);
							}
							break;
						}
						else if (actionTree.containsActionCategory(inputValue)) {
							JOptionPane.showMessageDialog(ActionDialog.this,
									"Action category already exists, please choose another one!", "Error Message",
									JOptionPane.ERROR_MESSAGE);
						}
						else {
							JOptionPane.showMessageDialog(ActionDialog.this,
									"You have to enter a category name!", "Error Message",
									JOptionPane.ERROR_MESSAGE);
						}
					}
				}
			});
		}
		return newCategoryButton;
	}

	/**
	 * This method initializes jTable	
	 * 	
	 * @return javax.swing.JTable	
	 */
	private JTable getJTable() {
		String[] columnNames = {"Parametername", "Parametertype", "Parametervalue", "Parametercomment"};
		if (jTable == null) {
			model = new DefaultTableModel(columnNames, 0);
			jTable = new JTable(model);
		}
		jTable.setAutoResizeMode(javax.swing.JTable.AUTO_RESIZE_ALL_COLUMNS);
		jTable.setColumnSelectionAllowed(false);
		jTable.setCellSelectionEnabled(true);
		jTable.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
		jTable.setRowSelectionAllowed(true);
		TableColumn typeColumn = jTable.getColumnModel().getColumn(1);
		JComboBox comboBox = new JComboBox();
		comboBox.addItem("intParameter");
		comboBox.addItem("stringParameter");
		comboBox.addItem("floatParameter");
		comboBox.addItem("boolParameter");
		typeColumn.setCellEditor(new DefaultCellEditor(comboBox));
		// if change action: set parameters
		if (!(action == null)) {
			List allParas = action.getParameter();
			for (int i = 0; i < allParas.size(); i++) {
				Vector paraVec = (Vector) allParas.get(i);
				model.addRow(paraVec);
			}
		}
		return jTable;
	}

	/**
	 * This method initializes jButton1	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getOKButton() {
		if (oKButton == null) {
			oKButton = new JButton();
			oKButton.setBounds(new java.awt.Rectangle(480,280,80,30));
			oKButton.setText("OK");
			oKButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					if (!(actionCategoryComboBox.getSelectedItem() == null)) {
						Vector paraVector = model.getDataVector();
						TableCellEditor cellEditor = jTable.getCellEditor();
						if (jTable.getSelectedRowCount() > 0) {
							if (!(jTable.getSelectedColumn() == 1)) {
								if (!(jTable.getEditingRow() == -1)) {
									cellEditor.stopCellEditing();
								}
							}
							else {
								jTable.clearSelection();
							}
						}
						for (int i = 0; i < paraVector.size(); i++) {
							Vector para = (Vector) paraVector.get(i);
							if ((para.get(0) == null) || (para.get(0).toString().equals(""))) {
								JOptionPane.showMessageDialog(ActionDialog.this,
										"One or more parameters have no name! Please enter a name", "Error Message",
										JOptionPane.ERROR_MESSAGE);
								return;
							}
							else if ((para.get(2) == null) || (para.get(2).toString().equals(""))) {
								JOptionPane.showMessageDialog(ActionDialog.this,
										"One or more parameters have no value! Please enter a value", "Error Message",
										JOptionPane.ERROR_MESSAGE);
								return;
							}
							else if ((para.get(1) == null) || (para.get(1).toString().equals(""))) {
								JOptionPane.showMessageDialog(ActionDialog.this,
										"One or more parameters have no type! Please select a type", "Error Message",
										JOptionPane.ERROR_MESSAGE);
								return;
							}
						}
						
						if (actionNameComboBox.getSelectedItem() == null) {
							JOptionPane.showMessageDialog(ActionDialog.this,
									"You have to select an action name!", "Error Message",
									JOptionPane.ERROR_MESSAGE);
							return;
						}
						String id = actionTree.getActionId(actionNameComboBox.getSelectedItem().toString());
						info = new ActionAndEventInfo(actionCommentTextField.getText(), id, model.getDataVector());
					}
					cancel();
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
			cancelButton.setBounds(new java.awt.Rectangle(570,280,80,30));
			cancelButton.setText("Cancel");
			cancelButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					cancel();
				}
			});
		}
		return cancelButton;
	}
	
	/**
	 * disposes this dialog
	 */
	void cancel() {
		dispose();
	}
	
	/**
	 * @return the info object with comment, name and parameter
	 */
	public ActionAndEventInfo getInfo() {
		return info;
	}

	/**
	 * This method initializes jButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getAddParameterButton() {
		if (addParameterButton == null) {
			addParameterButton = new JButton();
			addParameterButton.setBounds(new java.awt.Rectangle(30,280,150,30));
			addParameterButton.setText("Add Parameter");
			addParameterButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					Vector newElement = new Vector(4);
					newElement.add(0,"");
					newElement.add(1,"");
					newElement.add(2,"");
					newElement.add(3,"");
					model.addRow(newElement);
				}
			});
		}
		return addParameterButton;
	}

	/**
	 * This method initializes jButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getRemoveParameterButton() {
		if (removeParameterButton == null) {
			removeParameterButton = new JButton();
			removeParameterButton.setBounds(new java.awt.Rectangle(190,280,150,30));
			removeParameterButton.setText("Remove Parameter");
			removeParameterButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					if (!(jTable.getSelectedRow() == -1)) {
						model.removeRow(jTable.getSelectedRow());
					}
				}
			});
		}
		return removeParameterButton;
	}

	/**
	 * This method initializes jScrollPane	
	 * 	
	 * @return javax.swing.JScrollPane	
	 */
	private JScrollPane getJScrollPane() {
		if (jScrollPane == null) {
			jScrollPane = new JScrollPane();
			jScrollPane.setBounds(new java.awt.Rectangle(30,110,620,150));
			jScrollPane.setViewportView(getJTable());
		}
		return jScrollPane;
	}

	/**
	 * This method initializes jComboBox	
	 * 	
	 * @return javax.swing.JComboBox	
	 */
	private JComboBox getActionNameComboBox() {
		boolean contains = false;
		if (actionNameComboBox == null) {
			actionNameComboBox = new JComboBox();
			actionNameComboBox.setMaximumRowCount(5);
			List allActions = actionTree.getActions(category);
			for (int i = 0; i < allActions.size(); i++) {
				Element act = (Element) allActions.get(i);
				String action = act.getAttributeValue("NAME");
				actionNameComboBox.addItem(action);
				List commentList = act.getContent();
				// set comments for each action to the tooltip
				String toolTip = "";
				for (int j = 0; j < commentList.size(); j++) {
					Object comEle = commentList.get(j);
					if (comEle instanceof Comment) {
						Comment comment = (Comment) comEle;
						toolTip = comment.getText();
						break;
					}
				}
				if (toolTip.equals("")) {
					toolTip = "No comment available";
				}
				tooltips.add(toolTip);
			}
			// if change action: set combobox to the correct entry
			if (!(action == null)) {
				String id = action.getActionId();
				for (int i = 0; i < actionNameComboBox.getItemCount(); i++) {
					String actionName = (String) actionNameComboBox.getItemAt(i);
					if (actionName.equals(actionTree.getActionName(id))) {
						contains = true;
						break;
					}
				}
				if (contains == false) {
					actionNameComboBox.addItem(actionTree.getActionName(id));
					tooltips.add("No comment available");
				}
				actionNameComboBox.setSelectedItem(actionTree.getActionName(id));
			}
			actionNameComboBox.setRenderer(new MyComboBoxRenderer());
			actionNameComboBox.setBounds(new java.awt.Rectangle(160,40,360,20));
		}
		return actionNameComboBox;
	}
	
	/**
	 * @author Daniel
	 * adds a tooltip to each combobox entry
	 */
	class MyComboBoxRenderer extends BasicComboBoxRenderer {
		private static final long serialVersionUID = 1L;
		
		public Component getListCellRendererComponent( JList list, 
				Object value, int index, boolean isSelected, boolean cellHasFocus) {
			if (isSelected) {
				setBackground(list.getSelectionBackground());
				setForeground(list.getSelectionForeground());      
				if (-1 < index) {
					list.setToolTipText(tooltips.get(index).toString());
				}
			} else {
				setBackground(list.getBackground());
				setForeground(list.getForeground());
			} 
			setFont(list.getFont());
			setText((value == null) ? "" : value.toString());     
			return this;
		}  
	}
	
	/**
	 * This method initializes jButton	
	 * 	
	 * @return javax.swing.JButton	
	 */
	private JButton getNewActionButton() {
		if (newActionButton == null) {
			newActionButton = new JButton();
			newActionButton.setBounds(new java.awt.Rectangle(530,40,120,20));
			newActionButton.setText("New action");
			newActionButton.setActionCommand("New action");
			newActionButton.addActionListener(new java.awt.event.ActionListener() {
				public void actionPerformed(java.awt.event.ActionEvent e) {
					NewActionDialog acDlg = new NewActionDialog(ActionDialog.this);
					acDlg.setVisible(true);
					String newComment = acDlg.getActionComment();
					String newName = acDlg.getActionName();
					String newId = acDlg.getActionId();
					if (!(acDlg.isCanceled())) {
					if (!(newName == null || newId == null) && (!(newName.equals("") || newId.equals("")))) {
						Object[] options = { "OK", "CANCEL" };
						int option = JOptionPane.showOptionDialog(ActionDialog.this, "Do you really want to add this action?\nAdded actions can not be removed!", "Click OK to continue",
									JOptionPane.DEFAULT_OPTION, JOptionPane.WARNING_MESSAGE,
									null, options, options[0]);
						if (option == 0) {
							actionTree.addAction(actionCategoryComboBox.getSelectedItem().toString(), newComment, newName, newId);
							actionNameComboBox.addItem(newName);
							if (!newComment.equals("")) {
								tooltips.add(newComment);
							}
							else {
								tooltips.add("No comment available");
							}
							actionNameComboBox.setSelectedItem(newName);
						}
					}
					}
				}
			});
		}
		return newActionButton;
	}

}  //  @jve:decl-index=0:visual-constraint="10,10"
