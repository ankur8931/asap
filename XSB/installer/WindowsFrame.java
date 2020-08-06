/****************************************************************************
			       XSB Installation
File name: 			WindowsFrame.java
Author(s): 			Dongli Zhang
Brief description: 	This is the Frame under Windows.
****************************************************************************/


import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Iterator;
import java.util.Map;
import java.net.URLDecoder;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import java.util.Properties;
import java.io.File;
import java.io.InputStream;
import java.io.InputStreamReader;

public class WindowsFrame extends JFrame {

    private static final long serialVersionUID = 1L;
    private String osType;
    private String vsPath;
    private String jdkPath;
    private int requireJDK;
    private String errorMessage="\n\n\nList of error messages:\n";
    private int countError=0;
    private int installSuccess;
    private String currentDir="";
    private String installerShell="";
    private String installerLog="";
    private String vsLinkUrl;
    private String sdkLinkUrl;
    private String jdkLinkUrl;
	
    private JPanel infoContentPane = null;
    private JPanel vsPathContentPane = null;
    private JPanel jdkPathContentPane = null;
    private JPanel compileContentPane = null;
    private JPanel finishContentPane = null;
	
    private JButton infoPrevButton = null;
    private JButton infoNextButton = null;
    private JButton vsPathPrevButton = null;
    private JButton vsPathNextButton = null;
    private JButton jdkPathPrevButton = null;
    private JButton jdkPathNextButton = null;
    private JButton compilePrevButton = null;
    private JButton compileNextButton = null;
    private JButton finishPrevButton = null;
    private JButton finishNextButton = null;
	
    private JLabel infoLabel = null;
    private JLabel infoLinkLabel = null;
    private JLabel infoSDKLinkLabel = null;
    private JLabel vsPathLabel = null;
    private JLabel jdkPathLabel = null;
    private JLabel compileLabel = null;
    private JLabel finishSuccessLabel = null;
    private JLabel finishFailLabel = null;
	
    private JTextField vsPathTextField = null;
    private JButton vsPathChooseButton = null;
    private JButton jdkPathChooseButton = null;
    private JTextField jdkPathTextField = null;
	
    private JTextArea compileTextArea = null;
    private JScrollPane compileScrollPane = null;
	
    private JCheckBox vsPathUseJavaCheckBox = null;
	
    public WindowsFrame() {
	super();
	initialize();
    }
	
    public WindowsFrame(String osType) {
	super();
		
	this.osType=osType;
		
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
	    // so, we get the current dir in a round-about way
	    // we need to pass currentDir to the shell
	    currentDir = MainRun.class.getProtectionDomain().getCodeSource().getLocation().getPath();
	    currentDir=currentDir.substring(0,currentDir.lastIndexOf('/')+1);
	    currentDir = URLDecoder.decode(currentDir, "utf-8");
	    currentDir = new File(currentDir).getPath();

	    installerShell =
		currentDir + "\\installer\\windowsinstall.bat \""
		+ currentDir + "\" ";
	    installerLog = currentDir+"\\Installer.log";
	} catch (Exception e) {
	    e.printStackTrace();
	}
    }
	
    private int getFrameHeight() {
	return this.getHeight();
    }
	
    private int getFrameWidth() {
	return this.getWidth();
    }

    private JPanel getInfoContentPane() {
	if (infoContentPane == null) {
	    infoContentPane = new JPanel();
	    infoContentPane.setLayout(null);
	    infoContentPane.add(getInfoLabel());
	    infoContentPane.add(getInfoLinkLabel());
	    if(osType.contains("64")) {
		infoContentPane.add(getInfoSDKLinkLabel());
	    }
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
	    infoLabel = new JLabel();
	    String message =
		"<html><body>"
		+"<h1>XSB Installation</h1><br/><br/>"
		+"<h3>Operating system: "+osType
		+".</h3><br/>"
		+"<h3>Please make sure that Visual Studio C++ is installed in your system.<br/>"
		+"<h3>If not, you can download it for free from:</h3>"
		+"</body></html>";
	    infoLabel.setText(message);
	    infoLabel.setBounds(30,20,600,300);
	}
	return infoLabel;
    }
	
    private JLabel getInfoLinkLabel() {
	if(infoLinkLabel == null) {
	    infoLinkLabel = new JLabel();

	    Properties props = new Properties();
	    try {
		InputStream in = Object.class.getResourceAsStream("/link.properties");
		props.load(in);
		vsLinkUrl=props.getProperty("visualstudio");
		System.out.println(vsLinkUrl);
	    } catch(Exception e) {
		e.printStackTrace();
	    }

	    String message = "<html><body><h3><a href=\"\">"+vsLinkUrl+"</a></body></h3></html>";
	    infoLinkLabel.setText(message);
	    infoLinkLabel.setBounds(40,300,300,30);
			
	    infoLinkLabel.addMouseListener(new MouseAdapter() {
		    public void mouseClicked(MouseEvent e) {
	                try {
			    Runtime.getRuntime().exec("cmd.exe /c start " + WindowsFrame.this.vsLinkUrl);
	                } catch (Exception ex) {
	                    ex.printStackTrace();
	                }
	            }
		    public void mouseEntered(MouseEvent e) 
		    {
			setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));
		    }
		    public  void mouseExited(MouseEvent e) 
		    {
			setCursor(Cursor.getDefaultCursor());
		    }
	        });
	}
	return infoLinkLabel;
    }
	
    private JLabel getInfoSDKLinkLabel() {
	if(infoSDKLinkLabel == null) {
	    infoSDKLinkLabel = new JLabel();

	    Properties props = new Properties();
	    try {
		InputStream in = Object.class.getResourceAsStream("/link.properties");
		props.load(in);
		sdkLinkUrl=props.getProperty("sdk");
	    } catch(Exception e) {
		e.printStackTrace();
	    }

	    String message = "<html><body><h3>Since you are using a 64-bit Windows system, you must also install</h3><br/><h3>&nbsp;&nbsp;<a href=\"\">Windows SDK and .Net Framework</a>.</h3></html>";
	    infoSDKLinkLabel.setText(message);
	    infoSDKLinkLabel.setBounds(30,350,400,120);
			
	    infoSDKLinkLabel.addMouseListener(new MouseAdapter() {
		    public void mouseClicked(MouseEvent e) {
	                try {
			    Runtime.getRuntime().exec("cmd.exe /c start " + WindowsFrame.this.sdkLinkUrl);
	                } catch (Exception ex) {
	                    ex.printStackTrace();
	                }
	            }
		    public void mouseEntered(MouseEvent e) 
		    {
			setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));
		    }
		    public  void mouseExited(MouseEvent e) 
		    {
			setCursor(Cursor.getDefaultCursor());
		    }
	        });
	}
	return infoSDKLinkLabel;
    }
	
    private JPanel getVsPathContentPane() {
	if (vsPathContentPane == null) {
	    vsPathContentPane = new JPanel();
	    vsPathContentPane.setLayout(null);
	    vsPathContentPane.add(getVsPathLabel());
	    vsPathContentPane.add(getVsPathNextButton());
	    vsPathContentPane.add(getVsPathPrevButton());
	    vsPathContentPane.add(getVsPathChooseButton());
	    vsPathContentPane.add(getVsPathTextField());
	    vsPathContentPane.add(getVsPathUseJavaCheckBox());
	}
	return vsPathContentPane;
    }
	
    private JButton getVsPathPrevButton() {
	if(vsPathPrevButton == null) {
	    vsPathPrevButton = new JButton();
	    vsPathPrevButton.setText("Previous");
	    vsPathPrevButton.setBounds(200, getFrameHeight()-100, 100, 30);
			
	    vsPathPrevButton.addActionListener(new VsPathPrevListener());
	}
	return vsPathPrevButton;
    }
	
    private JButton getVsPathNextButton() {
	if(vsPathNextButton == null) {
	    vsPathNextButton = new JButton();
	    vsPathNextButton.setText("Next");
	    vsPathNextButton.setBounds(getFrameWidth()-350, getFrameHeight()-100, 100, 30);
			
	    vsPathNextButton.addActionListener(new VsPathNextListener());
	}
	return vsPathNextButton;
    }
	
    private JLabel getVsPathLabel() {
	if(vsPathLabel == null) {
	    vsPathLabel = new JLabel();
	    String message =
		"<html><body>"
		+"<h1>XSB Installation</h1><br/><br/>"
		+"<h3>Please enter the folder where your installation of Visual C++ resides:</h3><br/>"
		+"</body></html>";
	    vsPathLabel.setText(message);
	    vsPathLabel.setBounds(30,20,600,200);
	}
	return vsPathLabel;
    }
	
    private JButton getVsPathChooseButton() {
	if(vsPathChooseButton == null) {
	    vsPathChooseButton = new JButton();
	    vsPathChooseButton.setText("Browse");
	    vsPathChooseButton.setBounds(340, 190, 100, 30);
			
	    vsPathChooseButton.addActionListener(new VsPathChooseListener());
	}
	return vsPathChooseButton;
    }
	
    private JTextField getVsPathTextField() {
	if(vsPathTextField == null) {
	    vsPathTextField = new JTextField();
	    vsPathTextField.setBounds(30, 190, 300, 30);
	}
	return vsPathTextField;
    }
	
    private JCheckBox getVsPathUseJavaCheckBox() {
	if(vsPathUseJavaCheckBox == null) {
	    vsPathUseJavaCheckBox = new JCheckBox();
	    vsPathUseJavaCheckBox.setText("Check if you require faster XSB-Java interface");
	    vsPathUseJavaCheckBox.setBounds(30, 240, 300, 30);
	}
	return vsPathUseJavaCheckBox;
    }
	
    private JPanel getJdkPathContentPane() {
	if (jdkPathContentPane == null) {
	    jdkPathContentPane = new JPanel();
	    jdkPathContentPane.setLayout(null);
	    jdkPathContentPane.add(getJdkPathLabel());
	    jdkPathContentPane.add(getJdkPathNextButton());
	    jdkPathContentPane.add(getJdkPathPrevButton());
	    jdkPathContentPane.add(getJdkPathChooseButton());
	    jdkPathContentPane.add(getJdkPathTextField());
	}
	return jdkPathContentPane;
    }
	
    private JButton getJdkPathPrevButton() {
	if(jdkPathPrevButton == null) {
	    jdkPathPrevButton = new JButton();
	    jdkPathPrevButton.setText("Previous");
	    jdkPathPrevButton.setEnabled(false);
	    jdkPathPrevButton.setBounds(200, getFrameHeight()-100, 100, 30);
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
	
    private JLabel getJdkPathLabel() {
	if(jdkPathLabel == null) {
	    jdkPathLabel = new JLabel();

	    Properties props = new Properties();
	    try {
		InputStream in =
		    Object.class.getResourceAsStream("/link.properties");
		props.load(in);
		jdkLinkUrl=props.getProperty("jdk");
	    } catch(Exception e) {
		e.printStackTrace();
	    }

	    String message =
		"<html><body>"
		+"<h1>XSB Installation</h1><br/><br/>"
		+"<h3>The environment variable JAVA_HOME has not been set.</h3><br/>"
		+"<h3>Make sure that Java JDK (not just JRE!) is installed. If it is not, download it from</h3><br/>"
		+"<h3>&nbsp;&nbsp;<a href=\"\">"+jdkLinkUrl+"</a></h3><br/>"
		+"<br/><h3>Once the JDK is installed, please enter the folder where it resides on your system:</h3><br/><br/>"
		+"</body></html>";
	    jdkPathLabel.setText(message);
	    jdkPathLabel.setBounds(30,20,600,300);

	    jdkPathLabel.addMouseListener(new MouseAdapter() {
		    public void mouseClicked(MouseEvent e) {
	                try {
			    Runtime.getRuntime().exec("cmd.exe /c start " + jdkLinkUrl);
	                } catch (Exception ex) {
	                    ex.printStackTrace();
	                }
	            }
		    public void mouseEntered(MouseEvent e) 
		    {
			setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));
		    }
		    public  void mouseExited(MouseEvent e) 
		    {
			setCursor(Cursor.getDefaultCursor());
		    }
	        });
	}
	return jdkPathLabel;
    }
	
    private JButton getJdkPathChooseButton() {
	if(jdkPathChooseButton == null) {
	    jdkPathChooseButton = new JButton();
	    jdkPathChooseButton.setText("Browse");
	    jdkPathChooseButton.setBounds(340, 340, 100, 30);
			
	    jdkPathChooseButton.addActionListener(new JdkPathChooseListener());
	}
	return jdkPathChooseButton;
    }
	
    private JTextField getJdkPathTextField() {
	if(jdkPathTextField == null) {
	    jdkPathTextField = new JTextField();
	    jdkPathTextField.setBounds(30, 340, 300, 30);
	}
	return jdkPathTextField;
    }
	
    private JPanel getCompileContentPane() {
	if (compileContentPane == null) {
	    compileContentPane = new JPanel();
	    compileContentPane.setLayout(null);
	    compileContentPane.add(getCompileLabel());
	    compileContentPane.add(getCompileNextButton());
	    compileContentPane.add(getCompilePrevButton());
	    compileContentPane.add(getCompileScrollPane());
			
	    installation();
	}
	return compileContentPane;
    }
	
    private JButton getCompilePrevButton() {
	if(compilePrevButton == null) {
	    compilePrevButton = new JButton();
	    compilePrevButton.setText("Previous");
	    compilePrevButton.setEnabled(false);
	    compilePrevButton.setBounds(200, getFrameHeight()-80, 100, 30);
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
	    compileLabel = new JLabel();
	    String message =
		"<html><body>"
		+"<h1>XSB is being compiled ...</h1><br/>"
		+"</body></html>";
	    compileLabel.setText(message);
	    compileLabel.setBounds(30,0,600,50);
	}
	return compileLabel;
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
	    finishSuccessLabel = new JLabel();
	    String message, xsbShell;
	    if(osType.contains("32")) {
		xsbShell = "xsb.bat";
	    } else {
		xsbShell = "xsb64.bat";
	    }
	    message =
		"<html><body>"
		+"<h1>XSB Installation</h1><br/><br/>"
		+"<h2>The installation was successful. "
		+"The log is in " + installerLog + "</h2><br/>"
		+"<h3>You can run XSB using:</h3>"
		+"<h3>&nbsp;&nbsp;" + currentDir 
		+ "\\bin\\" + xsbShell + "</h3></br>"
		+"<br/><h3>Click <i>Finish</i> to exit.</h3>"
		+"</body></html>";
	    finishSuccessLabel.setText(message);
	    finishSuccessLabel.setBounds(30,20,600,400);
	}
	return finishSuccessLabel;
    }
	
    private JLabel getFinishFailLabel() {
	if(finishFailLabel == null) {
	    finishFailLabel = new JLabel();
	    String message =
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
	
    class InfoNextListener implements ActionListener {
	public void actionPerformed(ActionEvent arg0) {
	    WindowsFrame.this.setContentPane(getVsPathContentPane());
	    WindowsFrame.this.validate();
	}
    }
	
    class VsPathPrevListener implements ActionListener {
	public void actionPerformed(ActionEvent arg0) {
	    WindowsFrame.this.setContentPane(getInfoContentPane());
	    WindowsFrame.this.validate();
	}
    }
	
    class VsPathNextListener implements ActionListener {
	public void actionPerformed(ActionEvent arg0) {
	    vsPath=getVsPathTextField().getText();
			
	    if(Tools.fileExist(vsPath+"\\VC\\bin\\nmake.exe")==0 || Tools.fileExist(vsPath+"\\VC\\bin\\cl.exe")==0)
		{
		    JOptionPane.showMessageDialog(WindowsFrame.this, "Visual C++ compiler not found at the given location.");
		    return;
		}
			
	    requireJDK=0;
	    if(getVsPathUseJavaCheckBox().isSelected()) {
		requireJDK=1;
		int needJavaHome=1;
		Map env = System.getenv();
		for(Iterator it=env.entrySet().iterator();it.hasNext(); ) {
		    Map.Entry entry = (Map.Entry)it.next();
		    if(entry.getKey().toString().equals("JAVA_HOME")) {
			jdkPath=entry.getValue().toString();
			if(Tools.fileExist(jdkPath+"\\bin\\javac.exe")==0 || Tools.fileExist(jdkPath+"\\include\\jni.h")==0 || Tools.fileExist(jdkPath+"\\include\\win32\\jni_md.h")==0)
			    {
				needJavaHome=0;
			    }
			break;
		    }
		}
		
		if(needJavaHome==1) {
		    WindowsFrame.this.setContentPane(getJdkPathContentPane());
		    WindowsFrame.this.validate();
		    return;
		}
	    }
	    WindowsFrame.this.setContentPane(getCompileContentPane());
	    WindowsFrame.this.validate();
	}
    }
	
    class VsPathChooseListener implements ActionListener {
	public void actionPerformed(ActionEvent arg0) {
	    JFileChooser fc=new JFileChooser("C:\\Program Files");
	    fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
	    File f=null;
	    int flag=fc.showOpenDialog(null);      
	    if(flag==JFileChooser.APPROVE_OPTION)
		{
		    f=fc.getSelectedFile();    
		    vsPath=f.getPath();
		}
	    WindowsFrame.this.getVsPathTextField().setText(vsPath);
	}
    }

    class JdkPathNextListener implements ActionListener {
	public void actionPerformed(ActionEvent arg0) {
	    jdkPath=WindowsFrame.this.getJdkPathTextField().getText();
	    if (Tools.fileExist(jdkPath+"\\bin\\javac.exe")==0 || Tools.fileExist(jdkPath+"\\include\\jni.h")==0 || Tools.fileExist(jdkPath+"\\include\\win32\\jni_md.h")==0) {
		JOptionPane.showMessageDialog(WindowsFrame.this, "JDK not found at the given location.");
		return;
	    }
	    WindowsFrame.this.setContentPane(getCompileContentPane());
	    WindowsFrame.this.validate();
	}
    }
	
    class JdkPathChooseListener implements ActionListener {
	public void actionPerformed(ActionEvent arg0) {
	    JFileChooser fc=new JFileChooser("C:\\Program Files");
	    fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
	    File f=null;
	    int flag=fc.showOpenDialog(null);      
	    if(flag==JFileChooser.APPROVE_OPTION) {
		f=fc.getSelectedFile();    
		jdkPath=f.getPath();
	    }
	    WindowsFrame.this.getJdkPathTextField().setText(jdkPath);
	}
    }
	
    class CompileNextListener implements ActionListener {
	public void actionPerformed(ActionEvent arg0) {
	    WindowsFrame.this.setContentPane(getFinishContentPane());
	    WindowsFrame.this.validate();
	}
    }
	
    class FinishNextListener implements ActionListener {
	public void actionPerformed(ActionEvent arg0) {
	    System.exit(0);
	}
    }
	
    private void installation()
    {
	if(requireJDK==1) {
	    if(jdkPath.charAt(jdkPath.length()-1)=='\\') {
		jdkPath=jdkPath.substring(0,jdkPath.length()-1);
	    }
		
	    String customSetting1 = "XSB_INTERPROLOG=yes";
	    String customSetting2 =	"MY_INCLUDE_DIRS=/I\""+jdkPath+"\\include"+"\" /I\""+jdkPath+"\\include\\win32"+"\"";
	    String windowsType = null;
	    if(osType.contains("32")) {
		windowsType="windows";
	    }
	    else {
		windowsType="windows64";
	    }
	    try {
		Process proc= Runtime.getRuntime().exec("cmd /c echo "+customSetting1+" > build\\"+windowsType+"\\custom_settings.mak");
		proc.waitFor();
		Process proc2= Runtime.getRuntime().exec("cmd /c echo "+customSetting2+" >> build\\"+windowsType+"\\custom_settings.mak");
	    } catch (IOException e) {
		e.printStackTrace();
	    } catch (InterruptedException e) {
		e.printStackTrace();
	    }
	}
	    
	try {
	    System.out.println(vsPath);
	    String vsDisk=vsPath.substring(0, 2);
	    String xsbDisk=currentDir.substring(0,2);
		
	    String makexsb;
	    String setvcvar;
	    if(osType.contains("32")) {
		makexsb="makexsb.bat";
		setvcvar="vcvars32.bat";
	    }
	    else {
		String cpuType=System.getProperty("sun.cpu.isalist");
		    
		if(cpuType.contains("amd64")) {
		    makexsb="makexsb64.bat";
		    setvcvar="vcvarsx86_amd64.bat";
		}
		else {
		    makexsb="makexsb64.bat";
		    setvcvar="vcvarsx86_ia64.bat";
		}
	    }
		
	    Process process =
		Runtime.getRuntime().exec("cmd /c "
					  + installerShell
					  + "\""+vsPath+"\" "
					  +vsDisk + " " + xsbDisk
					  +" \""+currentDir+"\" "
					  +makexsb+" "+setvcvar);
        	
	    final BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(process.getInputStream()));
	    final BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
        	
	    Thread t1 = new Thread() {
		    public void run() {
			String line = null;
			try {
			    while ((line = bufferedReader.readLine()) != null) {
				if(line.contains("=== done ===")) {
				    JOptionPane.showMessageDialog(WindowsFrame.this, "Compilation is complete. Click OK then Next.");
				    compileNextButton.setEnabled(true);
				    if(countError==0) {
					errorMessage=errorMessage+"No\n";
				    }
				    getCompileTextArea().setText(getCompileTextArea().getText()+errorMessage);
				    String testSucc;
				    if(osType.contains("32")) {
					testSucc=currentDir+"\\bin\\xsb.bat -v";
				    } else {
					testSucc=currentDir+"\\bin\\xsb64.bat -v";
				    }
				    Process resultProcess = Runtime.getRuntime().exec(testSucc);
				    try {
					resultProcess.waitFor();
			            } catch(InterruptedException e) {
					e.printStackTrace();
				    }
				    if(resultProcess.exitValue()==1) {
				    	installSuccess=0;
				    } else {
					installSuccess=1;
				    }
				    
				    //create log file
				    File file = new File(installerLog);
				    if (!file.exists()) {
				    	file.createNewFile();
				    }
				    
				    String totalMessage=getCompileTextArea().getText();
				    FileOutputStream fw = new FileOutputStream(file);
				    fw.write(totalMessage.getBytes());
				    fw.flush();
				    fw.close();
				    
				    break;
				}
				getCompileTextArea().setText(getCompileTextArea().getText()+"\n   >"+line);
				JScrollBar sbar=getCompileScrollPane().getVerticalScrollBar();  
				sbar.setValue(sbar.getMaximum());  
			    }
			}catch (IOException e) {
			    e.printStackTrace();
			}
		    }
		};
		
	    Thread t2 = new Thread() {
		    public void run(){
			String line = null;
			try {
			    while ((line = errorReader.readLine()) != null) {
				System.out.println(line);
				if(line.contains("Could Not Find") || (line.contains("Microsoft (R) Program Maintenance Utility Version")) || (line.contains("Copyright (C) Microsoft Corporation"))) {
				    continue;
				}
				countError++;
				errorMessage=errorMessage+"\n"+line;
			    }
			} catch (IOException e) {
			    e.printStackTrace();
			}
		    }
		};
	    
	    t1.start();
	    t2.start();
		
	} catch (IOException e) {
	    e.printStackTrace();
	}
    }
}
