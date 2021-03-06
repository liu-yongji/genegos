# genegos
A highly efficient tool specially designed for converting genome coordinate from GRCh37 to GRCh38.

Overview
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here we present a fast and efficient tool Genegos, which is specially designed for 
converting genome coordinate from  GRCh37 to GRCh38.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Building and installing Genegos
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Currently, Genegos only runs in the Linux environment, and you will need the g++ (GCC 4.8.5 or higher) compiler to compile the source code
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
   
 If meet something error ,please download and ues the genegos.demo :
 > chmod +x genegos.demo
 >./genegos.demo  [GRCh37 BED or VCF] [Destination path] [path of chains]
 
 
 
   
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   
Test
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Running Genegos
> genegos [GRCh37 BED or VCF] [Destination path] [path of chains]
> genegos [GRCh38 BED or VCF] [Destination path] [path of Genegos-chains] [-r]

For example: 
genegos-style chain file mode
>genegos ./test/test37.bed ./ ./Genegos-chains

or 
stand chain file mode
>./genegos ./test/test37.bed ./ ./hg19ToHg38.genegos.chain
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Important!  
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  1. Do not move the Genegos installation into a place
     different from that specified by --prefix at build time.  This will
     cause things to break in subtle ways.
  2. Do not change any file in the folder named Genegos-chains. This will
      cause conversion errors.
  3. If the link in github does not work, please try another :
     http://www.genegos.com/downloadtools/genegos_v1/genegos-1.0.0.tar.gz
     
     

