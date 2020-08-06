/****************************************************************************
			        XSB Installation
File name: 			LinuxFrame.java
Author(s): 			Dongli Zhang
Brief description: 	This is the Frame under Linux.
****************************************************************************/


import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Scanner;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.JPasswordField;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;

public class LinuxFrame extends JFrame {

    private static final long serialVersionUID = 1L;
	
    private String osType;
    private int[] features;
    private String homePath="";
    private String readline="";
    private String password;
    private int installSuccess;
    private int needJavaHome=0;
    private String currentDir="";
    private String installerShell="";
    private String installerLog="";
	
    private JPanel infoContentPane = null; //Panel for general information
    private JPanel featureContentPane = null; //Panel for user to select feature
    private JPanel jdkPathContentPane = null; //Panel for user to specify where is JDK
    private JPanel installContentPane = null; //Panel to display the installation of packages
    private JPanel compileContentPane = null; //Panel to display the compilation of XSB
    private JPanel finishContentPane = null; //Panel to display the final page
	
    private JButton infoPrevButton = null; //Previous Button on Info Panel
    private JButton infoNextButton = null; //Next Button on Info Panel
    private JButton featurePrevButton = null; //Previous Button on Feature Panel
    private JButton featureNextButton = null; //Next Button on Feature Panel
    private JButton jdkPathPrevButton = null; //Previous Button on JDK Path Selection Panel
    private JButton jdkPathNextButton = null; //Next Button on JDK Path Selection Panel
    private JButton installPrevButton = null; //Previous Button on Package Installation Panel
    private JButton installNextButton = null; //Next Button on Package Installation Panel
    private JButton compilePrevButton = null; //Previous Button on Compilation Panel
    private JButton compileNextButton = null; //Next Button on Compilation Panel
    private JButton finishPrevButton = null; //Previous Button on Last Finish Panel
    private JButton finishNextButton = null; //Next Button on Last Finish Panel
	
    //The five checkboxes below are checkboxes for features on feature selection panel
    private JCheckBox dbCheckBox = null;
    private JCheckBox httpCheckBox = null;
    private JCheckBox xmlCheckBox = null;
    private JCheckBox regCheckBox = null;
    private JCheckBox javaCheckBox = null;
	
    private JLabel infoLabel = null;
    private JLabel featureLabel = null;
    private JLabel jdkPathLabel = null;
    private JLabel installLabel = null;
    private JLabel compileLabel = null;
    private JLabel finishSuccessLabel =null;
    private JLabel finishFailLabel = null;
	
    private JScrollPane installScrollPane = null;
    private JTextArea installTextArea = null;
	
    private JScrollPane compileScrollPane = null;
    private JTextArea compileTextArea = null;
	
    private JTextField jdkPathTextField = null;
    private JButton jdkPathChooseButton = null;

    public LinuxFrame() {
	super();
	initialize();
    }
	
    public LinuxFrame(String osType) {
	super();
		
	this.osType=osType;
	//features: xml http db interptolog and regular expression
	this.features=new int[5];
		
	initialize();
    }

    private void initialize() {
	Toolkit kit = Toolkit.getDefaultToolkit();
	Dimension screenSize = kit.getScreenSize();	
	int screenHeight = screenSize.height;
	int screenWidth = screenSize.width;
		
	this.setSize(screenWidth/4*3, screenHeight/4*3);
	this.setLocation(screenWidth/8, screenHeight/8);
	this.setContentPane(getInfoContentPane());
	this.setTitle("XSB Installation");
	this.setResizable(false);
	this.setDefaultCloseOperation(EXIT_ON_CLOSE);
	this.setVisible(true);
	try {
	    // problem: Linux File manager hides the current dir of the jar file
	    // so, we get it in a round-about way
	    // we need to pass currentDir to the shell
	    currentDir = MainRun.class.getProtectionDomain().getCodeSource().getLocation().toURI().getPath();
	    currentDir=currentDir.substring(0,currentDir.lastIndexOf('/')+1);
	    installerShell =
		currentDir + "installer/unixinstall.sh " + currentDir + " ";
	    installerLog = currentDir+"Installer.log";
	} catch (java.net.URISyntaxException e) {
	    e.printStackTrace();
	}
    }
	
    private int getFrameHeight() {
	return this.getHeight();
    }
	
    private int getFrameWidth() {
	return this.getWidth();
    }

    //The first penel to show general information to user.
    private JPanel getInfoContentPane() {
	if (infoContentPane == null) {
	    infoContentPane = new JPanel();
	    infoContentPane.setLayout(null);
	    infoContentPane.add(getInfoLabel());
	    infoContentPane.add(getInfoNextButton());
	    infoContentPane.add(getInfoPrevButton());
	}
	return infoContentPane;
    }
	
    private JButton getInfoPrevButton() {
	if(infoPrevButton == null) {
	    infoPrevButton = new JButton();
	    infoPrevButton.setText("Previous");
	    infoPrevButton.setEnabled(false);
	    infoPrevButton.setBounds(200, getFrameHeight()-100, 100, 30);
	}
	return infoPrevButton;
    }
	
    private JButton getInfoNextButton() {
	if(infoNextButton == null) {
	    infoNextButton = new JButton();
	    infoNextButton.setText("Next");
	    infoNextButton.setBounds(getFrameWidth()-350, getFrameHeight()-100, 100, 30);
		
	    infoNextButton.addActionListener(new InfoNextListener());
	}
	return infoNextButton;
    }
	
    private JLabel getInfoLabel() {
	if(infoLabel == null) {
	    infoLabel =new JLabel();
	    String message=
		"<html><body>"
		+"<h1>XSB Installation</h1><br/><br/>"
		+"<h2>Selecting features:</h2>"
		+"<p>On the next page you will be offered several options for XSB installation. Some of these options may require installing additional packages for your system. In turn, this requires that your username is in the <i>sudoer list</i>. To confirm this, open the file <u>/etc/sudoers</u> and verify that the line:</p><"
		+"<h3>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<i>your-user-name</i>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ALL=(ALL)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ALL</h3>"
		+"<p>is there. If not, you will have to login as <u>root</u> and add the above line to that file.</p><br/>"
		+"<h2>No root access:</h2>"
		+"<p>If the above steps cannot be performed because you are not a sudoer and have no root access, XSB will be installed with whatever features are possible given your configuration. Note: all the features on the next page are <i>optional</i>, so you will be getting a functioning copy of XSB in any case.</p><br/>"
		+"</body></html>";
	    infoLabel.setText(message);
	    infoLabel.setBounds(30,20,650,400);
	}
	return infoLabel;
    }
	
    //The panel to let the use choose the required feature
    private JPanel getFeatureContentPane() {
	if (featureContentPane == null) {
	    featureContentPane = new JPanel();
	    featureContentPane.setLayout(null);
	    featureContentPane.add(getDbCheckBox(),null);
	    featureContentPane.add(getHttpCheckBox(),null);
	    featureContentPane.add(getXmlCheckBox(),null);
	    featureContentPane.add(getRegCheckBox(),null);
	    featureContentPane.add(getJavaCheckBox(),null);
	    featureContentPane.add(getFeaturePrevButton(),null);
	    featureContentPane.add(getFeatureNextButton(),null);
	    featureContentPane.add(getFeatureLabel(),null);
	}
	return featureContentPane;
    }
	
    private JButton getFeaturePrevButton() {
	if(featurePrevButton == null) {
	    featurePrevButton = new JButton();
	    featurePrevButton.setText("Previous");
	    featurePrevButton.setBounds(200, getFrameHeight()-100, 100, 30);
			
	    featurePrevButton.addActionListener(new FeaturePrevListener());
	}
	return featurePrevButton;
    }
	
    private JButton getFeatureNextButton() {
	if(featureNextButton == null) {
	    featureNextButton = new JButton();
	    featureNextButton.setText("Next");
	    featureNextButton.setBounds(getFrameWidth()-350, getFrameHeight()-100, 100, 30);
		
	    featureNextButton.addActionListener(new FeatureNextListener());
	}
	return featureNextButton;
    }
	
    private JLabel getFeatureLabel() {
	if(featureLabel == null) {
	    featureLabel =new JLabel();
	    String message=	"<html><body>"
		+"<h1>XSB Installation</h1><br/><br/>"
		+"<h3>Please choose the features you would like to install XSB with.</h3>"
		+"<h3>If you need faster XSB-Java interface, please install JDK and set the JAVA_HOME environment variable.</h3>"
		+"</body></html>";
	    featureLabel.setText(message);
	    featureLabel.setBounds(30,20,600,150);
	}
	return featureLabel;
    }
	
    private JCheckBox getDbCheckBox() {
	if(dbCheckBox == null) {
	    dbCheckBox = new JCheckBox();
	    dbCheckBox.setText("Database drivers");
	    dbCheckBox.setBounds(30, 200, 190, 30);
	    if(features[0]==1)
		{
		    dbCheckBox.setSelected(true);
		}
	}
	return dbCheckBox;
    }
	
    private JCheckBox getHttpCheckBox() {
	if(httpCheckBox == null) {
	    httpCheckBox = new JCheckBox();
	    httpCheckBox.setText("HTTP access");
	    httpCheckBox.setBounds(30, 250, 190, 30);
	    if(features[1]==1)
		{
		    httpCheckBox.setSelected(true);
		}
	}
	return httpCheckBox;
    }
	
    private JCheckBox getXmlCheckBox() {
	if(xmlCheckBox == null) {
	    xmlCheckBox = new JCheckBox();
	    xmlCheckBox.setText("XML parsing");
	    xmlCheckBox.setBounds(30, 300, 190, 30);
	    if(features[2]==1)
		{
		    xmlCheckBox.setSelected(true);
		}
	}
	return xmlCheckBox;
    }
	
    private JCheckBox getRegCheckBox() {
	if(regCheckBox == null) {
	    regCheckBox = new JCheckBox();
	    regCheckBox.setText("Regular expressions");
	    regCheckBox.setBounds(250, 200, 190, 30);
	    if(features[3]==1)
		{
		    regCheckBox.setSelected(true);
		}
	}
	return regCheckBox;
    }
	
    private JCheckBox getJavaCheckBox() {
	if(javaCheckBox == null) {
	    javaCheckBox = new JCheckBox();
	    javaCheckBox.setText("Faster Java interface");
	    javaCheckBox.setBounds(250, 250, 190, 30);
	    if(features[4]==1)
		{
		    javaCheckBox.setSelected(true);
		}
	}
	return javaCheckBox;
    }
	
    //The panel for user to specify the location of JDK
    private JPanel getJdkPathContentPane() {
	if (jdkPathContentPane == null) {
	    jdkPathContentPane = new JPanel();
	    jdkPathContentPane.setLayout(null);
	    jdkPathContentPane.add(getJdkPathPrevButton(),null);
	    jdkPathContentPane.add(getJdkPathNextButton(),null);
	    jdkPathContentPane.add(getJdkPathLabel(),null);
	    jdkPathContentPane.add(getJdkPathTextField(),null);
	    jdkPathContentPane.add(getJdkPathChooseButton(),null);
	}
	return jdkPathContentPane;
    }
	
    private JButton getJdkPathPrevButton() {
	if(jdkPathPrevButton == null) {
	    jdkPathPrevButton = new JButton();
	    jdkPathPrevButton.setText("Previous");
	    jdkPathPrevButton.setBounds(200, getFrameHeight()-100, 100, 30);
			
	    jdkPathPrevButton.addActionListener(new JdkPathPrevListener());
	}
	return jdkPathPrevButton;
    }
	
    private JButton getJdkPathNextButton() {
	if(jdkPathNextButton == null) {
	    jdkPathNextButton = new JButton();
	    jdkPathNextButton.setText("Next");
	    jdkPathNextButton.setBounds(getFrameWidth()-350, getFrameHeight()-100, 100, 30);
		
	    jdkPathNextButton.addActionListener(new JdkPathNextListener());
	}
	return jdkPathNextButton;
    }
	
    private JButton getJdkPathChooseButton() {
	if(jdkPathChooseButton == null) {
	    jdkPathChooseButton = new JButton();
	    jdkPathChooseButton.setText("Browse");
	    jdkPathChooseButton.setBounds(340, 250, 100, 30);
		
	    jdkPathChooseButton.addActionListener(new JdkPathChooseListener());
	}
	return jdkPathChooseButton;
    }
	
    private JTextField getJdkPathTextField() {
	if(jdkPathTextField == null) {
	    jdkPathTextField = new JTextField();
	    jdkPathTextField.setBounds(30, 250, 300, 30);
	}
	return jdkPathTextField;
    }
	
    private JLabel getJdkPathLabel() {
	if(jdkPathLabel == null) {
	    jdkPathLabel =new JLabel();
	    String message=
		"<html><body>"
		+"<h1>XSB Installation</h1><br/><br/>"
		+"<h3>The environment variable JAVA_HOME has not been set.</h3><br/>"
		+"<h3>Please enter the folder where JDK resides on your system:</h3><br/>"
		+"</body></html>";
	    jdkPathLabel.setText(message);
	    jdkPathLabel.setBounds(30,20,600,200);
	}
	return jdkPathLabel;
    }
	
    //The panel to show user the installation of packages
    private JPanel getInstallContentPane() {
	if (installContentPane == null) {
	    installContentPane = new JPanel();
	    installContentPane.setLayout(null);
	    installContentPane.add(getInstallScrollPane(),null);
	    installContentPane.add(getInstallPrevButton(),null);
	    installContentPane.add(getInstallNextButton(),null);
	    installContentPane.add(getInstallLabel(),null);
			
	    installation();
	}
	return installContentPane;
    }
	
    private JScrollPane getInstallScrollPane() {
	if (installScrollPane == null) {
	    installScrollPane = new JScrollPane();
	    installScrollPane.setBounds(10, 60, getFrameWidth()-20, getFrameHeight()-160);
	    installScrollPane.setViewportView(getInstallTextArea());
	    getInstallTextArea().setLineWrap(true);
	}
	return installScrollPane;
    }
	
    private JTextArea getInstallTextArea() {
	if (installTextArea == null) {
	    installTextArea = new JTextArea();
	}
	return installTextArea;
    }
	
    private JButton getInstallPrevButton() {
	if(installPrevButton == null) {
	    installPrevButton = new JButton();
	    installPrevButton.setText("Previous");
	    installPrevButton.setBounds(200, getFrameHeight()-80, 100, 30);
	    installPrevButton.setEnabled(false);
			
	}
	return installPrevButton;
    }
	
    private JButton getInstallNextButton() {
	if(installNextButton == null) {
	    installNextButton = new JButton();
	    installNextButton.setText("Next");
	    installNextButton.setBounds(getFrameWidth()-350, getFrameHeight()-80, 100, 30);
	    installNextButton.setEnabled(false);
		
	    installNextButton.addActionListener(new InstallNextListener());
	}
	return installNextButton;
    }
	
    private JLabel getInstallLabel() {
	if(installLabel == null) {
	    installLabel =new JLabel();
	    String message=
		"<html><body>"
		+"<h1>The required packages are being installed</h1>"
		+"</body></html>";
	    installLabel.setText(message);
	    installLabel.setBounds(30,0,600,50);
	}
	return installLabel;
    }
	
    //The panel to show user the compilation
    private JPanel getCompileContentPane() {
	if (compileContentPane == null) {
	    compileContentPane = new JPanel();
	    compileContentPane.setLayout(null);
	    compileContentPane.add(getCompileScrollPane(),null);
	    compileContentPane.add(getCompilePrevButton(),null);
	    compileContentPane.add(getCompileNextButton(),null);
	    compileContentPane.add(getCompileLabel(),null);
			
	    compilation();
	}
	return compileContentPane;
    }
	
    private JScrollPane getCompileScrollPane() {
	if (compileScrollPane == null) {
	    compileScrollPane = new JScrollPane();
	    compileScrollPane.setBounds(10, 60, getFrameWidth()-20, getFrameHeight()-160);
	    compileScrollPane.setViewportView(getCompileTextArea());
	    getCompileTextArea().setLineWrap(true);
	}
	return compileScrollPane;
    }
	
    private JTextArea getCompileTextArea() {
	if (compileTextArea == null) {
	    compileTextArea = new JTextArea();
	}
	return compileTextArea;
    }
	
    private JButton getCompilePrevButton() {
	if(compilePrevButton == null) {
	    compilePrevButton = new JButton();
	    compilePrevButton.setText("Previous");
	    compilePrevButton.setBounds(200, getFrameHeight()-80, 100, 30);
	    compilePrevButton.setEnabled(false);
			
	}
	return compilePrevButton;
    }
	
    private JButton getCompileNextButton() {
	if(compileNextButton == null) {
	    compileNextButton = new JButton();
	    compileNextButton.setText("Next");
	    compileNextButton.setBounds(getFrameWidth()-350, getFrameHeight()-80, 100, 30);
	    compileNextButton.setEnabled(false);
			
	    compileNextButton.addActionListener(new CompileNextListener());
	}
	return compileNextButton;
    }
	
    private JLabel getCompileLabel() {
	if(compileLabel == null) {
	    compileLabel =new JLabel();
	    String message=
		"<html><body>"
		+"<h1>XSB is being compiled ...</h1>"
		+"</body></html>";
	    compileLabel.setText(message);
	    compileLabel.setBounds(30,0,600,50);
	}
	return compileLabel;
    }
	
    //The last panel to show whether the installation is successful
    private JPanel getFinishContentPane() {
	if (finishContentPane == null) {
	    finishContentPane = new JPanel();
	    finishContentPane.setLayout(null);
	    if(installSuccess==1)
		{
		    finishContentPane.add(getFinishSuccessLabel());
		}
	    else
		{
		    finishContentPane.add(getFinishFailLabel());	
		}
	    finishContentPane.add(getFinishNextButton());
	    finishContentPane.add(getFinishPrevButton());
	}
	return finishContentPane;
    }
	
    private JButton getFinishPrevButton() {
	if(finishPrevButton == null) {
	    finishPrevButton = new JButton();
	    finishPrevButton.setText("Previous");
	    finishPrevButton.setEnabled(false);
	    finishPrevButton.setBounds(200, getFrameHeight()-100, 100, 30);
	}
	return finishPrevButton;
    }
	
    private JButton getFinishNextButton() {
	if(finishNextButton == null) {
	    finishNextButton = new JButton();
	    finishNextButton.setText("Finish");
	    finishNextButton.setBounds(getFrameWidth()-350, getFrameHeight()-100, 100, 30);
		
	    finishNextButton.addActionListener(new FinishNextListener());
	}
	return finishNextButton;
    }
	
    private JLabel getFinishSuccessLabel() {
	if(finishSuccessLabel == null) {
	    finishSuccessLabel =new JLabel();
	    String message=
		"<html><body>"
		+"<h1>XSB Installation</h1><br/><br/>"
		+"<h2>The installation was successful. "
		+"The log is in " + installerLog + "</h2><br/>"
		+"<h3>You can run XSB using:</h3>"
		+"<h3>&nbsp;&nbsp;" + currentDir + "bin/xsb</h3><br/>"
		+"<br/><h3>Click <i>Finish</i> to exit.</h3>"
		+"</body></html>";
	    finishSuccessLabel.setText(message);
	    finishSuccessLabel.setBounds(30,20,670,400);
	}
	return finishSuccessLabel;
    }
	
    private JLabel getFinishFailLabel() {
	if(finishFailLabel == null) {
	    finishFailLabel =new JLabel();
	    String message=
		"<html><body>"
		+"<h1>XSB Installation</h1><br/><br/>"
		+"<h2>The installation was not successful.</h2><br/>"
		+"<h2>Please check " + installerLog + " for errors.</h2><br/>"
		+"<br/><h2>Click <i>Finish</i> to exit.</h2>"
		+"</body></html>";
	    finishFailLabel.setText(message);
	    finishFailLabel.setBounds(30,20,600,400);
	}
	return finishFailLabel;
    }
	
    class InfoNextListener implements ActionListener{
	public void actionPerformed(ActionEvent arg0) {
	    LinuxFrame.this.setContentPane(getFeatureContentPane());
	    LinuxFrame.this.validate();
	}
    }
	
    class FeaturePrevListener implements ActionListener{
	public void actionPerformed(ActionEvent arg0) {
	    LinuxFrame.this.setContentPane(getInfoContentPane());
	    LinuxFrame.this.validate();
	}
    }
	
    //Based on the features choosen, to check whether we can continue
    class FeatureNextListener implements ActionListener{
	public void actionPerformed(ActionEvent arg0) {
	    //If we install new features we should input the password
	    if(dbCheckBox.isSelected()
	       || httpCheckBox.isSelected()
	       || xmlCheckBox.isSelected()
	       || regCheckBox.isSelected()
	       || javaCheckBox.isSelected()) {
		String message="To install the necessary packaged, please enter your user password:";
		JTextField inputPassword=new JPasswordField();
		Object[] ob={message, inputPassword};
		int result=JOptionPane.showConfirmDialog(LinuxFrame.this, ob, "Password",JOptionPane.CANCEL_OPTION);
		if (result == JOptionPane.OK_OPTION && inputPassword.getText()!=null && !inputPassword.getText().equals("")) {
			Process process;
			try {
			    String command = "sh " + installerShell + osType.toLowerCase()+" checkpassword "+inputPassword.getText();
			    process = Runtime.getRuntime().exec(command);
			    BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
			    String resultStr=bufferedReader.readLine();
			    if(resultStr.contains("granted")) {
				password = inputPassword.getText();
			    }
			    else{
				JOptionPane.showMessageDialog(LinuxFrame.this,
							      "Invalid password");
				return;
			    }
			} catch (IOException e) {
			    e.printStackTrace();
			}
		} else {
		    JOptionPane.showMessageDialog(LinuxFrame.this,
						  "Invalid password");
		    return;
		}
	    }
	    else {
		LinuxFrame.this.setContentPane(getCompileContentPane());
		LinuxFrame.this.validate();
		return;
	    }
		    
	    //check if JDK is installed already
	    if(javaCheckBox.isSelected()) {
		Process process;
		try {
		    String command =
			"sh " + installerShell
			+ osType.toLowerCase() + " checkJava";
		    process = Runtime.getRuntime().exec(command);
		    BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
		    String resultStr=bufferedReader.readLine();
		    if(resultStr.contains("no")) {
			JOptionPane.showMessageDialog(LinuxFrame.this, "For faster XSB-Java interface you must install the Java JDK.");
			return;
		    }
		} catch (IOException e) {
		    e.printStackTrace();
		}
	    }
		    
	    if(dbCheckBox.isSelected()) {
		features[0]=1;
	    }
	    if(httpCheckBox.isSelected()) {
		features[1]=1;
	    }
	    if(xmlCheckBox.isSelected()) {
		features[2]=1;
	    }
	    if(regCheckBox.isSelected()) {
		features[3]=1;
	    }
	    //check whether the default JAVA_HOME is legal. It may not contain jni.h 
	    if(javaCheckBox.isSelected()) {
		features[4]=1;
			
		Process process;
		needJavaHome=1;
		try {
		    String command =
			"sh " + installerShell
			+ osType.toLowerCase() + " checkhome";
		    process = Runtime.getRuntime().exec(command);
		    BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
		    String resultStr=bufferedReader.readLine();
		    System.out.println(resultStr);
		    if(resultStr.contains("yes")) {
			needJavaHome=0;
		    }
		} catch (IOException e) {
		    e.printStackTrace();
		}
	    }
		    
	    //needJavaHome means the user must give a new JAVA_HOME
	    if(needJavaHome==1) {
		//Go to JDK path selection panel
		LinuxFrame.this.setContentPane(getJdkPathContentPane());
		LinuxFrame.this.validate();
		return;
	    }					
	    else {
		//JAVA_HOME is correct. Go to the package installation panel directly.
		LinuxFrame.this.setContentPane(getInstallContentPane());
		LinuxFrame.this.validate();
	    }
	}
    }
    
    class JdkPathPrevListener implements ActionListener{
	public void actionPerformed(ActionEvent arg0) {
	    LinuxFrame.this.setContentPane(getFeatureContentPane());
	    LinuxFrame.this.validate();
	}
    }

    class JdkPathNextListener implements ActionListener{
	public void actionPerformed(ActionEvent arg0) {
	    LinuxFrame.this.setContentPane(getInstallContentPane());
	    LinuxFrame.this.validate();
	}
    }
	
    class JdkPathChooseListener implements ActionListener{
	public void actionPerformed(ActionEvent arg0) {
	    JFileChooser fc=new JFileChooser();
	    fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
	    File f=null;
	    int flag=fc.showOpenDialog(null);      
	    if(flag==JFileChooser.APPROVE_OPTION) {
		f=fc.getSelectedFile();    
		homePath=f.getPath();
	    }
			
	    //Check whether the JDK path specified by user is legal
	    Process process;
	    try {
		String command =
		    "sh " + installerShell
		    + osType.toLowerCase() + " checkhomearg " + homePath;
		process = Runtime.getRuntime().exec(command);
		BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
		String resultStr=bufferedReader.readLine();
		if(resultStr.contains("no")) {
		    JOptionPane.showMessageDialog(LinuxFrame.this,
						  "Invalid JDK folder. No javac or jni.h found.");
		    homePath=null;
		    return;
		}
		else {
		    LinuxFrame.this.getJdkPathTextField().setText(homePath);
		}
	    } catch (IOException e) {
		e.printStackTrace();

	    }

	}
    }
	
    class InstallNextListener implements ActionListener{
	public void actionPerformed(ActionEvent arg0) {
	    LinuxFrame.this.setContentPane(getCompileContentPane());
	    LinuxFrame.this.validate();

	}

    }
	
    class CompileNextListener implements ActionListener{
	public void actionPerformed(ActionEvent arg0) {
	    LinuxFrame.this.setContentPane(getFinishContentPane());
	    LinuxFrame.this.validate();
	}
    }
    
    class FinishNextListener implements ActionListener{
	public void actionPerformed(ActionEvent arg0) {
	    System.exit(0);
	}
    }
    
    //The function to install all required packages
    //To facilitate display of log message, we use SwingUtilities.invokeLater
    public void installation()
    {	
	Thread t=new Thread() {
		Runnable run = new Runnable() {
			public void run() 
			{
			    getInstallTextArea().setText(readline);
			    JScrollBar sbar=getInstallScrollPane().getVerticalScrollBar();  
			    sbar.setValue(sbar.getMaximum());  
			}
		    };
		
		public void run()
		{
		    //sh installer/unixinstall.sh <OS> installFeatures your-password xml reg.....
		    String command1, command="sh " + installerShell;
		    command1 = command+osType.toLowerCase()+" "+"installFeatures"+" <your password>";
		    command=command+osType.toLowerCase()+" "+"installFeatures"+" "+password;
		    System.out.println(command1);
		    
		    if(features[0]==1) {
			command=command+" "+"db";
		    }
		    if(features[1]==1) {
			command=command+" "+"http";
		    }
		    if(features[2]==1) {
			command=command+" "+"xml";
		    }
		    if(features[3]==1) {
			command=command+" "+"reg";
		    }
		    
		    try {
			Process process = Runtime.getRuntime().exec(command);
			Scanner s = new Scanner(process.getInputStream());
			
			while(true){
			    if(s.hasNextLine()) {
				String nextLine=s.nextLine();
				//With  SwingUtilities.invokeLater it is hard
				// to detect whether the thread ends. We add a
				// special string in the shell script
				if(nextLine.contains("=== done ===")) {
				    break;
				}
				readline=readline+nextLine+"\n";
				SwingUtilities.invokeLater(run);
			    }
			    else {
				JOptionPane.showMessageDialog(LinuxFrame.this, "Something wrong during package installation: no output from shell");
				getInstallNextButton().setEnabled(true);
				return;
			    }
			}

			JOptionPane.showMessageDialog(LinuxFrame.this, "The required packages have been installed. Click Next to compile.");
			getInstallNextButton().setEnabled(true);
			
		    } catch (IOException e) {
			e.printStackTrace();
		    }
		}
	    };
	
	t.start();
    }
    
    //Function to configure and make
    //To facilitate display of log message, we use SwingUtilities.invokeLater
    private void compilation()
    {
	readline="";
	Thread t=new Thread() {
		Runnable run = new Runnable() {
			public void run() 
			{
			    getCompileTextArea().setText(readline);
			    JScrollBar sbar=getCompileScrollPane().getVerticalScrollBar();  
			    sbar.setValue(sbar.getMaximum());  
			}
		    };
		
		public void run()
		{
		    //sh installer/unixinstall.sh <OS> configure1 [path of jdk]
		    String command =
			"sh " + installerShell
			+ osType.toLowerCase() + " ";
		    
		    if(features[0]==1 ) {
			command=command+"configure2";
		    }
		    else {
			command=command+"configure1";
		    }
		    
		    if(needJavaHome==1) {
			command=command+" "+homePath;
		    }
		    
		    try {
			Process process = Runtime.getRuntime().exec(command);
			Scanner s = new Scanner(process.getInputStream());
			
			while(true) {
			    if(s.hasNextLine()) {
				String nextLine=s.nextLine();
				if(nextLine.contains("=== done ===")) {
				    Process resultProcess = Runtime.getRuntime().exec("sh "+currentDir+"bin/xsb -v");
				    try {
					resultProcess.waitFor();
			            } catch(InterruptedException e) {
					e.printStackTrace();
				    }
				    if (resultProcess.exitValue()==1) {
				    	installSuccess=0;
				    } else {
					installSuccess=1;
				    }
				    break;
				}
				readline=readline+nextLine+"\n\r";
				SwingUtilities.invokeLater(run);
			    }
			    else {
				JOptionPane.showMessageDialog(LinuxFrame.this, "Something wrong during compilation: no output from shell");
				getCompileNextButton().setEnabled(true);
				
				//create log file 
				File file = new File(installerLog);
				if(!file.exists()) {
				    file.createNewFile();
				}
				  
				FileOutputStream fw = new FileOutputStream(file);
				fw.write(readline.getBytes());
				fw.flush();
				fw.close();
				return;
			    }
			}

			JOptionPane.showMessageDialog(LinuxFrame.this, "Compilation is complete. Click OK then Next.");
			getCompileNextButton().setEnabled(true);
 			
			//create log file
			File file = new File(installerLog);
			
			if (!file.exists()) {
			    file.createNewFile();
			}
			
			FileOutputStream fw = new FileOutputStream(file);
			fw.write(readline.getBytes());
			fw.flush();
			fw.close();
		    } catch (IOException e) {
			e.printStackTrace();
		    }
		}
	    };
	
	t.start();		
    }
}
