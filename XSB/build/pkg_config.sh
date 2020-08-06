#! /bin/sh

# Run XSB on selected packages, so they would be compiled at the
# installation time.
# Gets the full path name of the XSB executable as the
# first argument.

xsb_executable=$1

echo ""
echo "Configuring XSB packages. Wait..."
#echo "[perlmatch]. [gap]. [wildmatch]. [regmatch]. [slx]. \
echo "[gap]. [wildmatch]. [regmatch]. [slx]. \
      halt." \
		    | $xsb_executable 
