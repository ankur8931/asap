#! /bin/sh


cat <<EOF
*******************************************************************************

The installation process is now complete. The log is in: Installation_summary

We would like to ask you to email this log to us.
Installation logs help the XSB group to keep track of the usage of the
system on different architectures and to isolate problems more easily.

The log will be sent automatically to  xsb-installation@lists.sourceforge.net
Would you like to send us the installation log? (y/n): y
EOF

read sendlog

if test "$sendlog" != "n" -a "$sendlog" != "no" -a "$sendlog" != "N" ; then
    (cat sendlog.msg Installation_summary \
	| mail xsb-installation@lists.sourceforge.net) \
    && echo "" ; echo "Thank you!"; echo ""
fi

cat <<EOF
Should you find a bug in XSB, please report it using our bug tracking system at

	http://sourceforge.net/bugs/?group_id=1176 

and also to  xsb-development@lists.sourceforge.net

EOF

