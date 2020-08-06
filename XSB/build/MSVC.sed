# makedepend adds @@@ so that we could find the relevant lines
/@@@/ {
h
# Create line SOURCE=foo.c
s/^@@@\([^:]*\)\.obj:.*$/SOURCE=$(INTDIR)\\\1.c/
# Save it and put the original makedepend line in the text buffer
x
# Replace slashes with backslashes
s,/,\\,g
# Add $(OBJDIR) to .obj, $(SOURCE) after `:',
s,^@@@\([^:]*\):,"$(OBJDIR)\\\1" : $(SOURCE),
# prepend {$(INTDIR)} to every .h file -- seems unnecessary, so comment out
#s, \([^ ]*\.h\), {$(INTDIR)}"\\\1",g
# MacOS has a problem with the next line
# Next is cosmetic (adds \n), but MacOS does not understand \n, so not used
#s/$/\n/
x
G
# append CPP line
#a\
#   $(CPP) $(CPP_PROJ) $(SOURCE)\
#\

}

