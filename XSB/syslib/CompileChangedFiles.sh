#! /bin/sh

XSB=$1

../build/touch.sh cmd...

split -l 40 cmd... cmd..._

for f in cmd..._*; do
     cat cmd...hdr $f | "$XSB"
done

rm cmd... cmd...hdr cmd..._*
