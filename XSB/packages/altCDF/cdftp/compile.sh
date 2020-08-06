#! /bin/sh

EMU=$1

echo "--------------------------------------------------------------------"
echo "Compiling CDFTP"

$EMU << EOF

['../cdf_config'].

compile(cdftp_cdfsc).

compile(cdftp_chkCon).

compile(cdftp_getObj).

compile(cdftp_meta).

compile(cdftp_preproc).

compile(cdftp_rules).

compile(tp_utils).

EOF
