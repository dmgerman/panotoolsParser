/*
 *  tparser.c
 *
 *  Copyright  Daniel M. German
 *  
 *  April 2007
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  Author: Daniel M German dmgerman at uvic doooot ca
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>

#include "tparser.h"


int main(int argc, char *argv[]) 
{
    int i;

    pt_script *script;

    if (argc != 2) {
      printf("Usage %s <filename>\n", argv[0]);
      exit(01);
    }

    printf("Testing parser for panotools and hugin files with input file [%s]\n", argv[1]);

    //    panoScriptParserSetDefaults(script);

    script = panoScriptParse(argv[1], FALSE, NULL);

    if (script == NULL) {
	printf("Parsing error.\n");
    } else {

      printf("\nValues  before dereferencing\n\n");

      panoScriptDump(script);

      if (!panoScriptDeReferenceVariables(script)) {
	printf("Unable to de-reference all variable\n");
      }
      
      printf("\n\n\n\n\n\n-----------------\nValues after dereferencing\n\n");

      panoScriptDump(script);


      /*
      printf("Printing after variables are dereferrenced\n");

      panoScriptDump(&script);
      */
      panoScriptParserDispose(script);
    }

    return 0;
}
