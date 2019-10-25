# genegos
A highly efficient tool specially designed for converting genome coordinate from GRCh37 to GRCh38.

Overview
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here we present a fast and efficient tool Genegos, which is specially designed for 
converting genome coordinate from  GRCh37 to GRCh38.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Building and installing Genegos
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
To install from a tar.gz distribution:
  1. Run "./configure", with option for alternative directory using --prefix=/where/you/want/it/installed.
  2. Run "make".
  3. Run "make install", possibly requires root permission.
  4. See if it works.  Try "genegos ./test/test37.bed ./ ./Genegos-chains".  

  Either this works, or it pops out with some complaints.  
  In that case, please let us know.  
 Follow the installation instructions there or use the following commands:
   > wget https://raw.githubusercontent.com/liu-yongji/genegos/master/genegos-1.0.0.tar.gz
   > tar -xvf genegos-1.0.0.tar.gz
   > cd genegos-1.0.0
   > ./configure
   > make
   > make install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   
Test
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Running Genegos
> genegos [GRCh37 BED or VCF] [Destination path] [path of Genegos-chains]

For example: 
>genegos ./test/test37.bed ./ ./Genegos-chains
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Important!  
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  1. Do not move the Genegos installation into a place
     different from that specified by --prefix at build time.  This will
     cause things to break in subtle ways.

  2. Do not change any file in the folder named Genegos-chains. This will
      cause conversion errors.

