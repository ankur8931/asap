JFLAGS = -g
JC = javac
.SUFFIXES: .java .class
.java.class:
	$(JC) $(JFLAGS) $*.java

CLASSES = \
		  MainRun.java \
		  Tools.java \
		  LinuxFrame.java \
		  WindowsFrame.java

default: classes

classes: $(CLASSES:.java=.class)

clean: 
	rm -f *.class
