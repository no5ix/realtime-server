#!/bin/sh

chmod u+x build_rts.sh;
chmod u+x kill_rts.sh;
chmod u+x run_rts.sh;
chmod u+x show_rts.sh;
chmod u+x init_rts_env.sh;

mkdir ../build;

rts_path=`find ~ -name rts | head -1`

ln -s $rts_path ~/rts;

exit 0