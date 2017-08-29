/*
 *  parser.y
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

%{
       #include <stdio.h>
       #include <stdlib.h>
       #include <string.h>
       #include <math.h>

  /*  #define lround gaaekraf adf adf
       #define lroundf gaaekraf adf adf 
*/

#include "tparser.h"
#include "tparserprivate.h"
#include "tparserdebug.h"

       int yylex (void);
       void yyerror (char const *);
/*  Keeps track of the current type of input line in the file */
int  currentLine = -1;

pt_script script;

/* defining it gives better error messages. It might be an overkill */
#define YYERROR_VERBOSE 1 

static pt_script_image *image;
static pt_script_ctrl_point *ctrlPoint;
static pt_script_morph_point *morphPoint;

// Look ahead variables
//   Hugin  uses a comment line _before_ the i line: for example
//   #-hugin  autoCenterCrop=1 cropFactor=1

#define PT_AUTO_CENTER_CROP_ST "autoCenterCrop"
#define PT_AUTO_CENTER_CROP_DEFAULT 0
static int autoCenterCrop = PT_AUTO_CENTER_CROP_DEFAULT;
static int huginPTOfileVersion = 0;


#define PT_CROP_FACTOR_ST "cropFactor"
#define PT_CROP_FACTOR_DEFAULT 0
static int cropFactor = PT_CROP_FACTOR_DEFAULT;

/* copy a string while allocating and checking for memory */
static void ParserStringCopy(char **dest, char *from)
{
    if (*dest) 
	free(*dest);
    *dest = strdup(from);
    if (*dest == NULL)
	panoScriptParserError(1,"Not enough memory");
}


%}
     
%defines

%union {
  int   iVal;
  float fVal;
  char strVal[PT_TOKEN_MAX_LEN+1];
  char cVal;
}


%token NUM
%token <fVal> PT_TOKEN_NUMBER
%token <strVal> PT_TOKEN_STRING
%token <strVal> PT_TOKEN_HUGIN_KEYWORD
%token <cVal> PT_TOKEN_KEYWORD
%token <cVal> PT_TOKEN_CROPPING

%token PT_TOKEN_EOL
%token PT_TOKEN_SEP
%token PT_TOKEN_INPUT_LINE
%token PT_TOKEN_OUTPUT_LINE
%token PT_TOKEN_PANO_LINE
%token PT_TOKEN_OPTIMIZE_OPT_LINE
%token PT_TOKEN_CONTROL_PT_LINE
%token PT_TOKEN_COMMA
%token PT_TOKEN_REFERENCE
%token PT_TOKEN_MORPH_PT_LINE
%token PT_TOKEN_HUGIN_VERSION
%token PT_TOKEN_HUGIN_LINE
%token PT_TOKEN_HUGIN_OPTIONS_LINE
%token PT_TOKEN_ERROR
%token PT_TOKEN_EOF

%start input

%% /* Grammar rules and actions follow.  */

input: lines PT_TOKEN_EOF
      { 
          int i;
          DEBUG_1("Lines");

          pt_script_morph_point *morphPoint2;
          int image;
          pt_script_image *imageSpec;
          // We have to move the morphing control points to their corresponding images
          DEBUG_2("Number of control points %d", script.iMorphPointsCount);

          for (i = 0; i < script.iMorphPointsCount; i++) {
              // get a pointer to the point to make life easier
              morphPoint = &morphPointsSpec[i];
              image = morphPoint->iImage;
              // we need to verify that the iamge pointer exists
              if (image < 0 || image > script.iInputImagesCount) {
                  panoScriptParserError(0,"Morphing control point number %d references non existent input image [%c].\n", i, image);
                  return -1;
              }
              // point to the image, avoids long dereferrencing
              imageSpec = &script.inputImageSpec[image];

              // allocate the pointer
              morphPoint2 = (pt_script_morph_point *)panoScriptReAlloc((void*)&imageSpec->morphPoints, 
                                                                  sizeof(pt_script_morph_point), &imageSpec->morphPointsCount);
              
              memcpy(morphPoint2, morphPoint, sizeof(pt_script_morph_point));
          }
          DEBUG_2("Lines: number of images %d", script.iInputImagesCount);

          for (i=0;i<script.iInputImagesCount; i++)  {
              DEBUG_2("Image %d", script.inputImageSpec[i].morphPointsCount);              
          }
          DEBUG_2("Number of control points %d", script.iMorphPointsCount);

          return 0;
      }

lines:  line lines | 
{
    DEBUG_1("End of line\n");
}
     
line:          inputline
             | outputline
             | panoline
             | optimizeline
             | ctrlPtsLine
             | morphPtsLine
             | huginImageParmsLine
             | huginOptionsLine
             | huginPTOversion
             | parsingError
;
     

parsingError: PT_TOKEN_ERROR 
{
  DEBUG_1("What the heck\n");
    return -1;
} 


huginPTOversion: PT_TOKEN_HUGIN_VERSION PT_TOKEN_SEP PT_TOKEN_NUMBER eoln
         {
            huginPTOfileVersion = lroundf($3);
            printf("Version %d", huginPTOfileVersion);
         }

huginOptionsLine: PT_TOKEN_HUGIN_OPTIONS_LINE 
         { 
            currentLine = PT_TOKEN_HUGIN_OPTIONS_LINE;
         } vars  eoln
     

huginImageParmsLine: PT_TOKEN_HUGIN_LINE 
        {
            currentLine = PT_TOKEN_HUGIN_LINE;
        }
        huginVars
        eoln  { ; }
//        


ctrlPtsLine: PT_TOKEN_CONTROL_PT_LINE  PT_TOKEN_SEP
        { 
            currentLine = PT_TOKEN_CONTROL_PT_LINE;
            ctrlPoint = (pt_script_ctrl_point *)panoScriptReAlloc((void*)&script.ctrlPointsSpec, 
                                                                  sizeof(pt_script_ctrl_point), &script.iCtrlPointsCount);
        }  
        varsparms eoln       { ; }

morphPtsLine: PT_TOKEN_MORPH_PT_LINE  PT_TOKEN_SEP
        { 
            currentLine = PT_TOKEN_MORPH_PT_LINE;
            morphPoint = (pt_script_morph_point *)panoScriptReAlloc((void*)&morphPointsSpec, 
                                                                  sizeof(pt_script_morph_point), &script.iMorphPointsCount);
        }  
        varsparms eoln       { ; }


eoln:     PT_TOKEN_EOL  { DEBUG_1("ENDOFLINE");currentLine = -1; /* This says we don't know the type of line being processed */}

inputline:  PT_TOKEN_INPUT_LINE PT_TOKEN_SEP
        {

	    currentLine = PT_TOKEN_INPUT_LINE; 

            image = (pt_script_image *)panoScriptReAlloc((void*)&(script.inputImageSpec),
                                                         sizeof(pt_script_image), &script.iInputImagesCount);

            // This might require more though. Are the defaults of these two variables zero?
            image->autoCenterCrop = autoCenterCrop;
            image->cropFactor = cropFactor;
            cropFactor = PT_CROP_FACTOR_DEFAULT;
            autoCenterCrop = PT_AUTO_CENTER_CROP_DEFAULT;


#ifdef asdfasf
	    // allocate the new output
	    script.iInputImagesCount++;
	    script.inputImageSpec = realloc(script.inputImageSpec, 
					     sizeof(*script.inputImageSpec) * script.iInputImagesCount);
	    if (script.inputImageSpec == NULL) {
		yyerror("Not enough memory");
	    }
	    // clear the end of the reallocated region
	    bzero(&(script.inputImageSpec[script.iInputImagesCount-1]), sizeof(*script.inputImageSpec));
	    image = &script.inputImageSpec[script.iInputImagesCount-1];
#endif
        }   
        vars eoln      { ; }

outputline: PT_TOKEN_OUTPUT_LINE PT_TOKEN_SEP
        {
            DEBUG_1("Reading outputline");
	    currentLine = PT_TOKEN_OUTPUT_LINE; 
	    // allocate the new output
	    script.iOutputImagesCount++;
	    script.outputImageSpec = realloc(script.outputImageSpec, 
					     sizeof(*script.outputImageSpec) * script.iOutputImagesCount);
	    if (script.outputImageSpec == NULL) {
		yyerror("Not enough memory");
	    }
	    // clear the end of the reallocated region
	    bzero(&(script.outputImageSpec[script.iOutputImagesCount-1]), sizeof(*script.outputImageSpec));
	    image = &script.outputImageSpec[script.iOutputImagesCount-1];
	}      vars eoln      { ; }

optimizeline: PT_TOKEN_OPTIMIZE_OPT_LINE PT_TOKEN_SEP 
          { 
              currentLine=PT_TOKEN_OPTIMIZE_OPT_LINE; 
          } vars eoln      { ; }

panoline: PT_TOKEN_PANO_LINE PT_TOKEN_SEP
          { 
             currentLine=PT_TOKEN_PANO_LINE; 
          } vars eoln      { ; }


huginVars: huginVar
           | huginVars huginVar

huginVar: PT_TOKEN_HUGIN_KEYWORD PT_TOKEN_NUMBER 
          {
              // The tokens include the = at the end
              // we could do a substr, but there is no real value on that
              if (strcmp(PT_AUTO_CENTER_CROP_ST "=", $1) == 0) {
                  autoCenterCrop = lroundf($2);
              } else if (strcmp(PT_CROP_FACTOR_ST "=" , $1) == 0) {
                  cropFactor = $2;
              } else {
		panoScriptParserError(1,"Warning: invalid variable %s found in Hugin image line. Ignored...\n", $1);
              }
          }

vars:      var
          | vars PT_TOKEN_SEP var

/* a variable can be a cropping one (with 4 parms), a one-parm one, a reference to another variable, or finally, a name 
   only */
var:      varcropping |  varparameter  | varreference | varonly

varsparms:  varparameter
          | varsparms PT_TOKEN_SEP varparameter



 /*  Rule for input image field references <var>=<index> */
varreference: PT_TOKEN_KEYWORD PT_TOKEN_REFERENCE PT_TOKEN_NUMBER 
   {
       int imageRef = lroundf($3) + 1;

       DEBUG_1("Reading varreference");

       switch (currentLine) {
       case PT_TOKEN_INPUT_LINE:
	   switch ($1) {
	    case 'v':
		image->fHorFOVIndex = imageRef;
		break;
	    case 'y':
		image->yawIndex = imageRef;
		break;
	    case 'p':
		image->pitchIndex = imageRef;
		break;
	    case 'r':
		image->rollIndex = imageRef;
		break;
	    case 'a':
	    case 'b':
	    case 'c':
	    case 'd':
	    case 'e':
		image->coefIndex[$1-'a'] = imageRef;
		break;
	    case 'g':
		image->coefIndex[5] = imageRef;
		break;
	    case 't':
		image->coefIndex[6] = imageRef;
		break;
            case 'R': // This is a hugin Ra,Rb, ...Re
                // This is a hack. $1 is referenced as a char, but the next character is the 
                // a,b,c,d,e so let us get at it
                {
                    char *temp = &$1;
                    temp++;
                    switch (*temp) {
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'e':
                        image->responseCurveCoefIndex[(*temp)-'a'] = imageRef;
                        break;
                    default:
                        panoScriptParserError(1,"Invalid variable name R[%c] in image line...\n", *temp);
                        return -1;
                    }
                }
                break;
            case 'V': // This is a hugin Va, Vb, Vc, Vd, Vx, Vy
                // This is a hack. $1 is referenced as a char, but the next character is the 
                // a,b,c,d,e so let us get at it
                {
                    char *temp = &$1;
                    temp++;
                    switch (*temp) {
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                        image->vignettingCorrectionCoefIndex[(*temp)-'a'] = imageRef;
                        break;
                    case 'x':
                    case 'y':
                        image->vignettingCorrectionCoefIndex[(*temp)-'x'+4] = imageRef;
                        break;
                    default:
                        panoScriptParserError(1,"Invalid variable name V[%c] in image line...\n", *temp);
                        return -1;
                    }
                }
                break;
	   default:
	       panoScriptParserError(1,"Invalid variable name [%c=] in input line.\n", $1);
	       return -1;
	       break;
	   }
	   break;
       default:
	   panoScriptParserError(1,"Error Not handled 3 [%c]\n", $1);
	   return -1;
       }
   }

 /* Rule for [CS]<x>,<x>,<x>,<x> */
varcropping: PT_TOKEN_CROPPING PT_TOKEN_NUMBER PT_TOKEN_COMMA PT_TOKEN_NUMBER PT_TOKEN_COMMA PT_TOKEN_NUMBER PT_TOKEN_COMMA PT_TOKEN_NUMBER
    {
	switch (currentLine) {
	case PT_TOKEN_OUTPUT_LINE:
	case PT_TOKEN_INPUT_LINE:
	    switch ($1) {
	    case 'C':
	    case 'S':
		image->cropType = $1;
		image->cropArea[0] = lroundf($2);
		image->cropArea[1] = lroundf($4);
		image->cropArea[2] = lroundf($6);
		image->cropArea[3] = lroundf($8);
		break;
	    default:
		panoScriptParserError(1,"Invalid variable name- [%c] in image line\n", $1);
		return -1;
	    }
	    break;
	default:
	    panoScriptParserError(1,"Error Not handled 3\n");
	    return -1;
	}
    }

/*  
    Rules for <variable><parameter> 
 */
varparameter: PT_TOKEN_KEYWORD PT_TOKEN_STRING 
    { 

        DEBUG_2("Token %s", $2);

        /* Processing of string variables */
	switch (currentLine) {
	case PT_TOKEN_PANO_LINE:
	    switch ($1) {
	    case 'f':
		ParserStringCopy(&script.pano.projectionName, $2);
		break;
	    case 'n':
		ParserStringCopy(&script.pano.outputFormat, $2);
		break;
	    case 'P':
		ParserStringCopy(&script.pano.projectionParmsString, $2);
		break;
	    case 'T':
		ParserStringCopy(&script.pano.bitDepthOutput, $2);
	    default:
		panoScriptParserError(1,"Invalid variable name [%c] in pano line\n", $1);
		return -1;
	    }
	    break;
	case PT_TOKEN_OUTPUT_LINE:
	case PT_TOKEN_INPUT_LINE:
	    switch ($1) {
	    case 'n':
		ParserStringCopy(&image->name, $2);
		break;
	    default:
		panoScriptParserError(1,"Invalid variable name [%c] in image line...\n", $1);
		return -1;
	    }
	    break;
	default:
	    panoScriptParserError(1,"Error Not handled case [%c]\n", $1);
	    return -1;
	}
    }
   | PT_TOKEN_KEYWORD PT_TOKEN_NUMBER
    {
        /* Processing of int variables */
	switch (currentLine) {
        case PT_TOKEN_HUGIN_OPTIONS_LINE:
            switch ($1) {
            case 'r':
                script.optimize.optimizeReferenceImage = lround($2);
                break;
            case 'e':
                script.optimize.blendMode = lround($2);
                break;
            default:
                panoScriptParserError(1, "Invalid variable name [%c] in hugin options line.\n", $1);
                return -1;
                break;
            }
            break;
        case PT_TOKEN_CONTROL_PT_LINE:
	   switch ($1) {
           case 'n':
               ctrlPoint->iImage1 = lround($2);
               break;
           case 'N':
               ctrlPoint->iImage2 = lround($2);
               break;
           case 'x':
               ctrlPoint->p1.x = $2;
               break;
           case 'y':
               ctrlPoint->p1.y = $2;
               break;
           case 'X':
               ctrlPoint->p2.x = $2;
               break;
           case 'Y':
               ctrlPoint->p2.y = $2;
               break;
           case 't':
               ctrlPoint->type = $2;
               break;
	   default:
	       panoScriptParserError(1, "Invalid variable name [%c] in control point line.\n", $1);
	       return -1;
	       break;
	   }
           break;
        case PT_TOKEN_MORPH_PT_LINE:
	   switch ($1) {
           case 'i':
               morphPoint->iImage = lround($2);
               break;
           case 'x':
               morphPoint->p1.x = $2;
               break;
           case 'y':
               morphPoint->p1.y = $2;
               break;
           case 'X':
               morphPoint->p2.x = $2;
               break;
           case 'Y':
               morphPoint->p2.y = $2;
               break;
	   default:
	       panoScriptParserError(1, "Invalid variable name [%c] in morph line.\n", $1);
	       return -1;
	       break;
	   }
           break;
	case PT_TOKEN_PANO_LINE:
	    switch ($1) {
	    case 'w':
		script.pano.width = lroundf($2);
		break;
	    case 'h':
		script.pano.height = lroundf($2);
		break;
	    case 'f':
		script.pano.projection = lroundf($2);
		break;
	    case 'v':
		script.pano.fHorFOV = $2;
		break;
	    case 'E':
                script.pano.exposureValue = $2;
		break;
	    case 'R':
                script.pano.dynamicRangeMode = lroundf($2);
		break;
	    default:
		panoScriptParserError(1,"Error Invalid variable name [%c] in pano line\n", $1);
		return -1;
	    }
	    break;
	case PT_TOKEN_OUTPUT_LINE:
	case PT_TOKEN_INPUT_LINE:
	    switch ($1) {
	    case 'w':
		image->width = lroundf($2);
		break;
	    case 'h':
		image->height = lroundf($2);
		break;
	    case 'f':
		image->projection = lroundf($2);
		break;
	    case 'v':
		image->fHorFOV = $2;
		break;
	    case 'y':
		image->yaw = $2;
		break;
	    case 'p':
		image->pitch = $2;
		break;
	    case 'r':
		image->roll = $2;
		break;
	    case 'a':
	    case 'b':
	    case 'c':
	    case 'd':
	    case 'e':
		image->coef[$1-'a'] = $2;
		break;
	    case 'g':
		image->coef[5] = $2;
		break;
	    case 't':
		image->coef[6] = $2;
		break;
	    case 'u':
                image->featherSize = lroundf($2);
                break;
	    case 'm':
		panoScriptParserError(1,"Warning: Option %c in image line deprecated. Ignored...\n", $1);
		break;
            case 'R': // This is a hugin Ry, Ra,Rb, ...Re
                // This is a hack. $1 is referenced as a char, but the next character is the 
                // a,b,c,d,e so let us get at it
                {
                    char *temp = &$1;
                    temp++;
                    switch (*temp) {
                    case 'y':
                        image->responseCurveType = lroundf($2);
                        break;
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'e':
                        image->responseCurveCoef[(*temp)-'a'] = $2;
                        break;
                    default:
                        panoScriptParserError(1,"Invalid variable name R[%c] in image line...\n", *temp);
                        return -1;
                    }
                }
                break;
            case 'E':
                {
                    char *temp = &$1;
                    temp++;
                    if (strcmp(temp, "ev") == 0) {
                        image->imageEV = $2;
                    } else if (strcmp(temp, "r") == 0) {
                        image->whiteBalanceFactorRed = $2;
                    } else if (strcmp(temp, "b") == 0) {
                        image->whiteBalanceFactorBlue = $2;
                    } else {
                        panoScriptParserError(1,"Invalid variable name E[%c] in image line...\n", *temp);
                    }
                }
                break;
            case 'V': // This is a hugin Va, Vb, Vc, Vd, Vx, Vy
                // This is a hack. $1 is referenced as a char, but the next character is the 
                // a,b,c,d,e so let us get at it
                {
                    char *temp = &$1;
                    temp++;
                    switch (*temp) {
                    case 'm':
                        image->vignettingCorrectionMode = lroundf($2);
                        break;
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                        image->vignettingCorrectionCoef[(*temp)-'a'] = $2;
                        break;
                    case 'x':
                    case 'y':
                        image->vignettingCorrectionCoef[(*temp)-'x'+4] = $2;
                        break;
                    default:
                        panoScriptParserError(1,"Invalid variable name V[%c] in image line...\n", *temp);
                        return -1;
                    }
                }
                break;
	    default:
		panoScriptParserError(1,"Invalid variable name [%c] in image line...\n", $1);
		return -1;
	    }
	    break;
	case PT_TOKEN_OPTIMIZE_OPT_LINE:
	    switch ($1) {
	    case 'g':
		script.optimize.fGamma = $2;
                if (script.optimize.fGamma <= 0.0) {
                  panoScriptParserError(1,"Invalid value for gamma %f. Must be bigger than zero\n", script.optimize.fGamma);
                }
		break;
	    case 'i':
		script.optimize.interpolator = lroundf($2);
                // should we verify its value here?
		break;
	    case 'f':
		script.optimize.fastFT = lroundf($2);
		break;
	    case 'm':
		script.optimize.huberEstimator = $2;
		break;
	    case 'p':
		script.optimize.photometricHuberSigma = $2;
		break;
	    default:
		panoScriptParserError(1,"Invalid variable name [%c] in optimize line\n", $1);
		return -1;
	    }
	    break;
	default:
	    panoScriptParserError(1,"Error. Not handled (token int [%c])\n", $1);
	    return -1;
	}
    }
             

varonly: PT_TOKEN_KEYWORD
    {
	switch (currentLine) {
	case PT_TOKEN_PANO_LINE:
	    switch ($1) {
	    case 'T':
		ParserStringCopy(&script.pano.bitDepthOutput, "");
		break;
	    default:
		panoScriptParserError(1,"Invalid variable name [%c] in pano line\n", $1);
		return -1;
	    }
	    break;
	case PT_TOKEN_OUTPUT_LINE:
	case PT_TOKEN_INPUT_LINE:
	    switch ($1) {
	    default:
		panoScriptParserError(1,"Invalid variable name [%c] in image line....\n", $1);
		return -1;
	    }
	    break;
	default:
	    panoScriptParserError(1,"Error Not handled 3\n");
	    return -1;
	}
    }




%%
