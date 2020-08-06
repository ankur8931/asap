#! /bin/bash
for i in smodels/*.h; do ln -s $i `basename $i .h`.h; done
#for i in smodels/*.o; do ln -s $i `basename $i .o`.o; done
