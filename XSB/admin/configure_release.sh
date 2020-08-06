
XSBDIR=$1
 
rm ../config/*/saved.o/*.o ; 

echo "--------------- opt configure -------------------------"

cd ../build

configure > /tmp/config ;
makexsb ;

 echo "--------------- mt-opt configure -------------------------"
 
 configure --enable-mt  > /tmp/config ; 
 makexsb --config-tag=mt ;
 
 echo "--------------- batched configure -------------------------"
 
 configure --enable-batched  > /tmp/config ; 
 makexsb --config-tag=btc ;

 echo "--------------- 64-bit configure -------------------------"

 configure --with-bits64 ;
 makexsb --config-tag=bits64;

echo "--------------- mt-64 configure -------------------------"

 cd ../XSB/build

 configure --with-bits64 --enable-mt ;
 makexsb --config-tag=bits64-mt ;

#--------------------------------------------------------
 
rm ../config/*/saved.o/*.o ; 

 
