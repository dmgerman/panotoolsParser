/*
 *  tparserprivate.h
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

// Defines that are not to be made public outside the parser

#ifndef __TPARSER_PUBLIC_H__
#define __TPARSER_PUBLIC_H__

static pt_script_morph_point *morphPointsSpec = NULL;

void TokenBegin(char *t) ;
int  panoScriptScannerGetNextChar(char *b, int maxBuffer) ;
void panoScriptScannerTokenBegin(char *t) ;
void panoScriptParserError(int showLine, char const *errorstring, ...) ;
void *panoScriptReAlloc(char **ptr, int size, int *count);


#endif
