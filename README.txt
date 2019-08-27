Overview
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here we present a faster and efficient tool named Genegos specifically for converting genome coordinate from  GRCh37 to GRCh38.


Building and installing Genegos
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To install from a tar.gz distribution:

  1. Run ./configure, with some options if you wish.  The only interesting
     one is the usual --prefix=/where/you/want/it/installed.

  2. Run "make".

  3. Run "make install", possibly as root if the destination permissions
     require that.

  4. See if it works.  Try "genegos ./test/test37.bed ./ ./Genegos-chains".  
		 Either this works, or it bombs out with some complaint.  
		 In that case, please let us know
     
 Follow the installation instructions there or issue the following commands:
    tar -xvf genegos-1.0.0.tar.gz
    cd genegos-1.0.0
    ./configure
    make
    make install
    

Learn and Support
~~~~~~~~~~~~~~~~~~~~~~~~~~

Running Genegos
> genegos [GRCh37 files] [Destination path] [path of Genegos-chains]

for example: 
>genegos ./test/test37.bed ./ ./Genegos-chains



Important!  
~~~~~~~~~~~~~~~~~~~~~~~~~~
  1. Do not move the Genegos installation into a place
     different from that specified by --prefix at build time.  This will
     cause things to break in subtle ways, mostly when Genegos handles
     fork/exec calls.
  2. Do not change any file in the folder Genegos-chains. This will
      cause convertion errors.

