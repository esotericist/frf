#include <gc.h>
#include <stdio.h>

#include "sds.h"
#include "frf.h"


int main(int argc, char **argv) {
     GC_INIT();

     int max = 1000000;

     sds buffer[max];
     int i = 0;


     while (i < max)
     {
          buffer[i] = sdsnew( "Hi\n" );

          i++;

     }

     while ( 1 ) 
     {

     }
     

     return 0;
}
