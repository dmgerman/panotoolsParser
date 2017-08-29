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
#include "tparserprivate.h"
#include "parser.h"
#include "tparserdebug.h"

int yyparse(void) ;

int debug =0;

extern char *yytext;

extern pt_script script;

static FILE *file = NULL;
static int eof = 0;
static int nRow = 0;
static int nBuffer = 0;
static int lBuffer = 0;
static int nTokenStart = 0;
static int nTokenLength = 0;
static int nTokenNextStart = 0;
static int lMaxBuffer = PARSER_MAX_LINE;
static char buffer[PARSER_MAX_LINE+1];


char panoScriptParserDumpChar(char c) 
{
    if (  isprint(c)  )
	return c;
    return '@';
}

void panoScriptParserDumpRow(void) 
{
    fprintf(stdout, "\n%6d |%.*s", nRow, lBuffer, buffer);
}


/* Display parsing error, including the current line and a pointer to the error token */

void panoScriptParserError(int showLine, char const *errorstring, ...) 
{
    static char errmsg[1000];
    va_list args;
    
    int start=nTokenNextStart;
    int end=start + nTokenLength - 1;
    int i;

    DEBUG_1("Enterting panoscriptparserror\n");
    
    panoScriptParserDumpRow();
    
    if (  eof  ) {
	fprintf(stdout, "       !");
	for (i=0; i<lBuffer; i++)
	    fprintf(stdout, ".");
	fprintf(stdout, "^-EOF\n");
    } else {
	fprintf(stdout, "       !");
	for (i=1; i<start; i++)
	    fprintf(stdout, ".");
	for (i=start; i<=end; i++)
	    fprintf(stdout, "^");
	fprintf(stdout, "   at line %d column %d\n", nRow, start);
    }

    /* print it using variable arguments -----------------------------*/
    //va_start(args, errorstring);
    //vsprintf(errmsg, errorstring, args);
    //va_end(args);
    
    fprintf(stdout, "%s\n", errmsg);
    fprintf(stdout, "Token [%s]\n", yytext);

    // exit
}


int panoScriptScannerGetNextLine(void) 
{
    char *p;
    
    /* Reset line counters */
    nBuffer = 0;
    nTokenStart = -1;
    nTokenNextStart = 1;
    /* Reset marker for end of file */

    p = fgets(buffer, lMaxBuffer, file);
    if (  p == NULL  ) {
        if (  ferror(file)  )
            return -1;
        eof = TRUE;
        return 1;
    }
    
    nRow += 1;
    lBuffer = strlen(buffer);
    
    return 0;
}

// THis is the function that lex will use to read the next character
int panoScriptScannerGetNextChar(char *b, int maxBuffer) 
{
  int frc;
  
  if (  eof  )
    return 0;
  
  // read next line if at the end of the current
  while (  nBuffer >= lBuffer  ) {
    frc = panoScriptScannerGetNextLine();
    if (  frc != 0  )
      return 0;
    }

  // ok, return character 
  b[0] = buffer[nBuffer];
  nBuffer += 1;

  if (  debug  )
    printf("GetNextChar() => '%c'0x%02x at %d\n",
                        panoScriptParserDumpChar(b[0]), b[0], nBuffer);
  /* if string is empty, return 0 otherwise 1 */
  return b[0]==0?0:1;
}


void panoScriptScannerTokenBegin(char *t) 
{
    // REcord where a token begins
    nTokenStart = nTokenNextStart;
    nTokenLength = strlen(t);
    nTokenNextStart = nTokenStart + nTokenLength;
    DEBUG_4("Scanner token begin start[%d]len[%d]nextstart[%d]",nTokenStart,nTokenLength,nTokenNextStart);
}


void yyerror (char const *st)
{
    panoScriptParserError(1, st);
}

// Reallocs ptr by size, count is the variable with the current number of records allocated
//  actual data is in 1000
//  array located at  20 contains 1000
//  ptr has value 20
//  *ptr is 1000

void *panoScriptReAlloc(char **ptr, int size, int *count)
{
    char *temp;

    (*count)++;
    *ptr = realloc(*ptr, *count * size);
    if (ptr == NULL) {
        yyerror("Not enough memory");
        return NULL;
    }
    // point to the newly allocated record
    temp = (char*)*ptr;
    temp+= size * ((*count)-1);
    // clear the area
    bzero(temp, size);
    return temp;
}

#ifdef delete
void *panoScriptReAllocI(pt_script_image **ptr, int size, int *count)
{
    char *temp = NULL;

    printf("Before ptr %x *ptr %x\n", ptr, *ptr);
    printf("return temp %x *size %d, count %d\n", temp, size, *count);

    (*count)++;
    
    *ptr = realloc(*ptr, (*count) * size);
    if (ptr == NULL) {
        yyerror("Not enough memory");
        return NULL;
    }
    // point to the newly allocated record
    temp = (char*)*ptr;

    printf("To resize count %d, %d\n", *count, (size * ((*count)-1)));
    temp +=  size * ((*count)-1);
    // clear the area
    bzero(temp, size);

    printf("After ptr %x *ptr %x\n", ptr, *ptr);
    printf("return temp %x *size %d, count %d\n", temp, size, *count);
    return temp;
}
#endif

/*
 * dump a control point
 */
void panoParserDumpCtrlPoint(pt_script_ctrl_point *ctrlPoint)
{
    printf("    image1 %-5d (%6.1f,%6.1f) image2 %-5d (%6.1f,%6.1f) type %d\n",
           ctrlPoint->iImage1, 
           ctrlPoint->p1.x,            
           ctrlPoint->p1.y,
           ctrlPoint->iImage2, 
           ctrlPoint->p2.x,
           ctrlPoint->p2.y,
           ctrlPoint->type
           );
}

/*
 * dump a control point
 */
void panoParserDumpMorphPoint(pt_script_morph_point *ctrlPoint)
{
    printf("    image %-5d (%6.1f,%6.1f) to  (%6.1f,%6.1f)\n",
           ctrlPoint->iImage, 
           ctrlPoint->p1.x,            
           ctrlPoint->p1.y,
           ctrlPoint->p2.x,
           ctrlPoint->p2.y
           );
}


/*
  Dump all information we know about a particular image
*/
void panoParserDumpImage(pt_script_image *output)
{
    int i;
    pt_script_morph_point *morphPoint;

    printf("   projection %d\n", output->projection);
    printf("   width, height %dx%d ", output->width, output->height);
    printf("   fHorFOV %f\n", output->fHorFOV);
    printf("   yaw %f ", output->yaw);
    printf("   pitch %f ", output->pitch);
    printf("   roll %f ", output->roll);
    
    printf("   a %f", output->coef[0]);
    printf("   b %f", output->coef[1]);
    printf("   c %f", output->coef[2]);
    printf("   d %f", output->coef[3]);
    printf("   e %f", output->coef[4]);
    printf("   g %f", output->coef[5]);
    printf("   t %f\n", output->coef[6]);

    
    printf("Other parameters.............\n");
    printf("   name %s\n", output->name);
    printf("   cropType %c ", output->cropType);
    printf("   cropArea %i,%i,%i,%i ", 
	   output->cropArea[0],
	   output->cropArea[1],
	   output->cropArea[2],
	   output->cropArea[3]);
    printf("   morphToFit %d", output->morphToFit);
    printf("   feather size  %d", output->featherSize);

    printf("\nHugin parameters\n");

    printf("   autoCenterCrop %d", output->autoCenterCrop);
    printf("   cropFactor %f", output->cropFactor);
    printf("\n   image ev %f", output->imageEV);    
    printf("   whiteBalanceFactorRed %f", output->whiteBalanceFactorRed);    
    printf("   whiteBalanceFactorBlue %f", output->whiteBalanceFactorBlue);    
    printf("\n   responseCurveType %d", output->responseCurveType);    

    printf("   Ra  %f", output->responseCurveCoef[0]);
    printf("   Rb  %f", output->responseCurveCoef[1]);
    printf("   Rc  %f", output->responseCurveCoef[2]);
    printf("   Rd  %f", output->responseCurveCoef[3]);
    printf("   Re  %f\n", output->responseCurveCoef[4]);

    printf("\n  Vignetting Correction Mode %d", output->vignettingCorrectionMode);
    printf("   Va %f", output->vignettingCorrectionCoef[0]);
    printf("   Vb %f", output->vignettingCorrectionCoef[1]);
    printf("   Vc %f", output->vignettingCorrectionCoef[2]);
    printf("   Vd %f", output->vignettingCorrectionCoef[3]);
    printf("   Vx %f", output->vignettingCorrectionCoef[4]);
    printf("   Vy %f", output->vignettingCorrectionCoef[5]);


    printf("\nLinked attributes\n");
    printf("   fHorFOV %d", output->fHorFOVIndex);
    printf("   yaw  %d", output->yawIndex);
    printf("   pitch  %d", output->pitchIndex);
    printf("   roll  %d", output->rollIndex);
    
    printf("\n   a  %d", output->coefIndex[0]);
    printf("   b  %d", output->coefIndex[1]);
    printf("   c  %d", output->coefIndex[2]);
    printf("   d  %d", output->coefIndex[3]);
    printf("   e  %d", output->coefIndex[4]);
    printf("   g  %d", output->coefIndex[5]);
    printf("   t  %d\n", output->coefIndex[6]);

    printf("Hugin linked parameters\n");

    printf("   imageEVIndex %d", output->imageEVIndex);
    printf("   whiteBalanceFactorRedIndex %d", output->whiteBalanceFactorRedIndex);
    printf("   whiteBalanceFactorBlueIndex %d", output->whiteBalanceFactorBlueIndex);

    printf("\n   Va %d", output->vignettingCorrectionCoefIndex[0]);
    printf("   Vb %d", output->vignettingCorrectionCoefIndex[1]);
    printf("   Vc %d", output->vignettingCorrectionCoefIndex[2]);
    printf("   Vd %d", output->vignettingCorrectionCoefIndex[3]);
    printf("   Vx %d", output->vignettingCorrectionCoefIndex[4]);
    printf("   Vy %d", output->vignettingCorrectionCoefIndex[5]);


    printf("\n   Ra %d", output->responseCurveCoefIndex[0]);
    printf("   Rb %d", output->responseCurveCoefIndex[1]);
    printf("   Rc %d", output->responseCurveCoefIndex[2]);
    printf("   Rd %d", output->responseCurveCoefIndex[3]);
    printf("   Re %d\n", output->responseCurveCoefIndex[4]);


    printf("\n");

    printf("   Morphing points count %d\n", output->morphPointsCount);

    for (i=0;i<output->morphPointsCount;i++) {
	printf("  %-5d ", i);
	morphPoint = &output->morphPoints[i];
        panoParserDumpMorphPoint(morphPoint);
    }


}

/*
  Dump all the script information we have read
*/

void panoScriptDump(pt_script *script)
{
    int i;
    pt_script_image *output;
    pt_script_ctrl_point *ctrlPoint;
    pt_script_morph_point *morphPoint;

    assert(script != NULL);
    
    printf("Values\n");
    printf("Pano .................");
    printf(" width,height %dx%d ", script->pano.width, script->pano.height);
    printf(" projection %d ", script->pano.projection);
    printf(" name %s ", script->pano.projectionName);
    printf(" fHorFOV %f ", script->pano.fHorFOV);
    printf(" format %s ", script->pano.outputFormat);
    printf(" bitDepthOutput %s ", script->pano.bitDepthOutput);
    printf(" proj parms %s\n", script->pano.projectionParmsString);
    printf(" proj parms count %d\n", script->pano.projectionParmsCount);
    for (i=0;i<script->pano.projectionParmsCount;i++) {
	printf(" proj parm %d: %f\n", i, script->pano.projectionParms[i]);
    }

    
    printf("Optimize ..................");
    printf("  gamma %f", script->optimize.fGamma);
    printf("  interpolator %d", script->optimize.interpolator);
    printf("  fastFT %d ", script->optimize.fastFT);
    printf("  huber estimator %d \n", script->optimize.huberEstimator);

    printf("Hugin Parameters\n");
    printf("  dynamic range mode %d", script->pano.dynamicRangeMode);
    printf("  exposureValue %f", script->pano.exposureValue);

    printf("  photometricHuberSigma %f \n", script->optimize.photometricHuberSigma);
    printf("  optimize reference image %d \n", script->optimize.optimizeReferenceImage);
    printf("  blend mode %d \n", script->optimize.blendMode);
    printf("\n");

    
    printf("Output  [%d] images ..................\n", script->iOutputImagesCount);
    
    for (i=0;i<script->iOutputImagesCount;i++) {
	output = &script->outputImageSpec[i];
	printf(" image %d\n", i);
	panoParserDumpImage(output);
    }
    printf("Input  [%d] images ..................\n", script->iInputImagesCount);

    for (i=0;i<script->iInputImagesCount;i++) {
	output = &script->inputImageSpec[i];
	printf(" image %d\n", i);
	panoParserDumpImage(output);
    }
    printf("Ctrl points  [%d]  ..................\n", script->iCtrlPointsCount);
    for (i=0;i<script->iCtrlPointsCount;i++) {
	ctrlPoint = &script->ctrlPointsSpec[i];
	printf("  %-5d ", i);
        panoParserDumpCtrlPoint(ctrlPoint);
        //	panoParserDumpImage(output);
    }
#ifdef adfasdf    
    printf("Morphing points  [%d]  ..................\n", script->iMorphPointsCount);
    for (i=0;i<script->iMorphPointsCount;i++) {
	morphPoint = &morphPointsSpec[i];
	printf("  %-5d ", i);
        panoParserDumpMorphPoint(morphPoint);
    }
#endif
}


/*
  This function is be called before the parser is used for the first time, and if it wants to be reused.
  Remember, the parser is not REENTRANT
 */
int panoScriptParserReset(void)
{
    if (file != NULL)  {
	return FALSE;
    }
    /* There should not be anything allocated in script */
    bzero(&script, sizeof(script));    
    eof = FALSE;
    //but some parameters are meaningful when zero
    script.pano.projection = -1;
    return 1;
}

/*
 * Projection parms are outside the scope of the parser. In a way this is good, because they allow the use of
 * _any_ type of parameters. On the other hand, it is bad because they  have to be parsed here instead of the
 * bison parser.
 */
int panoScriptParseProjectionParms()
{
    char *ptr;
    float temp;

    if (script.pano.projectionParmsString != NULL) {
		// We got parameters which we need to parse
	ptr = script.pano.projectionParmsString;
	while (*ptr != '\0') {
	    if (*ptr == ' ' || *ptr  == '\t')
		ptr++;
	    else {
		if (script.pano.projectionParmsCount >= PANO_PARSER_MAX_PROJECTION_PARMS) {
		    panoScriptParserError(1, "Too many parameters for projection %s (limit is %d)", script.pano.projectionParmsString, PANO_PARSER_MAX_PROJECTION_PARMS);
		    return FALSE;
		}
		if (sscanf(ptr, "%f", &temp) != 1) {
		    panoScriptParserError(1, "Illegal floating point number in projection parameters %s", script.pano.projectionParmsString);
		    return FALSE;
		}
		script.pano.projectionParms[script.pano.projectionParmsCount] = temp;
		script.pano.projectionParmsCount++;
	    }
	}
    } 
    return TRUE;
    
}

/*
 * In some cases variables can point to other variables for their actual parameters. This
 * function derefereces them.
 */
int panoScriptDeReferenceImageVariables(pt_script_image *images, int imagesCount)
{
    // if the index != 0 and the indexed image is within bounds, and the indexed image is not a reference...
    // then set the imageindex, and clear the reference
#define UNLINK_VARIABLE(i,a,r) (ref= images[i].r-1,  \
				(ref >= 0          &&	\
				 ref < imagesCount &&	\
				 images[ref].r == 0 ?	\
				 (images[i].a = images[ref].a,		\
				  images[i].r = 0,			\
				  change=1)\
				 :1\
				 ))
    
#define UNLINK_VARIABLE2(a,b,c)

    // First unlink the variables that we can
    int change = 1;
    int ref;
    int i,j;
    // dereferrence until we are done
    while (change) {
	change = 0;
	for (i=0;i<imagesCount;i++) {
	    UNLINK_VARIABLE(i,fHorFOV,fHorFOVIndex);
	    UNLINK_VARIABLE(i,yaw,yawIndex);
	    UNLINK_VARIABLE(i,roll,rollIndex);
	    UNLINK_VARIABLE(i,pitch,pitchIndex);
	    for (j=0;j<PANO_PARSER_COEF_COUNT;j++) {
		UNLINK_VARIABLE(i,coef[j],coefIndex[j]);
	    }
	    for (j=0;j<PANO_PARSER_RESP_CURVE_COEF_COUNT;j++) {
		UNLINK_VARIABLE(i,responseCurveCoef[j],responseCurveCoefIndex[j]);
	    }
            UNLINK_VARIABLE(i,imageEV, imageEVIndex);
            UNLINK_VARIABLE(i,whiteBalanceFactorRed, whiteBalanceFactorRedIndex);
            UNLINK_VARIABLE(i,whiteBalanceFactorBlue, whiteBalanceFactorBlueIndex);
            UNLINK_VARIABLE(i,vignettingCorrectionMode,vignettingCorrectionModeIndex);
	    for (j=0;j<PANO_PARSER_VIGN_COEF_COUNT;j++) {
		UNLINK_VARIABLE(i,vignettingCorrectionCoef[j],vignettingCorrectionCoefIndex[j]);
	    }
	}
    }
#undef UNLINK_VARIABLE
    char *names[7] = {"a","b","c","d","e","g","t"};

#define VERIFY_VARIABLE(i,r,m)  if (images[i].r!=0) {panoScriptParserError(0, "Unable to resolve reference for variable %s in image %i\n", m, i);return FALSE;}

    // Verify that all links are resolved
    for (i=0;i<imagesCount;i++) {
	VERIFY_VARIABLE(i,fHorFOVIndex,"v");
	VERIFY_VARIABLE(i,yawIndex, "y");
	VERIFY_VARIABLE(i,rollIndex, "r");
	VERIFY_VARIABLE(i,pitchIndex, "p");
	for (j=0;j<PANO_PARSER_COEF_COUNT;j++) {
	    VERIFY_VARIABLE(i,coefIndex[j], names[j]);
	}
        for (j=0;j<PANO_PARSER_RESP_CURVE_COEF_COUNT;j++) {
	    VERIFY_VARIABLE(i,responseCurveCoefIndex[j],names[j]);
        }
    }
#undef VERIFY_VARIABLE

    return TRUE;
}

int panoScriptDeReferenceVariables(pt_script* script)
{
    // function that dereferences variables in scripts.
    if (!panoScriptDeReferenceImageVariables(script->inputImageSpec, script->iInputImagesCount))
	return FALSE;
    if (!panoScriptDeReferenceImageVariables(script->outputImageSpec, script->iOutputImagesCount))
	return FALSE;
    return TRUE;
}


pt_script *panoScriptParse(char *filename, int dereferenceVariables, pt_script *scriptVar)
{
    // filaname: input file

    // deferenceVariables: should references to other variables be resolved?

    DEBUG_1("Starting to parse");

    file = NULL;
    if (!panoScriptParserReset() ) {
	// This is really an assertion
	fprintf(stderr, "This parser is not reentrant");
	exit(1);
    }
    file = fopen(filename, "r");
    if (  file == NULL  ) {
	return NULL;
    }
    if ( panoScriptScannerGetNextLine() == 0 ) {
	if (yyparse() ==0) {
	    // AT THIS POINT WE HAVE FINISHED PARSING
	    // This is the best time to verify input values
	    // and to parse some of the optional parameters

          
	    if (!panoScriptParseProjectionParms() ) {
		panoScriptParserError(1, "Illegal parameters to projection");
		goto error;
	    } 

	    // Deference if required
	    if (dereferenceVariables) {
		if (!panoScriptDeReferenceVariables(&script)) 
		    goto error;
	    }
	    // Just making sure..
	    fclose(file);
            file = NULL;
            scriptVar = &script;
	    return scriptVar;

	} else  {
            DEBUG_1("Error in parsing\n");
	    goto error;
	}
    }
    else  {
	panoScriptParserError(0, "Input file is empty");
	goto error;
    }
    // It should never reach here
    assert(0); 
 error:
    if (file != NULL)
	fclose(file);
    return NULL;
    
}

void panoScriptParserSetDefaults(pt_script *ptr)
{
  // This is where the defaults will be
  // At this point. Just clear the data structure.
    memset(ptr, 0, sizeof(*ptr));
}

void panoScriptParserDispose(pt_script *ptr)
{
    int i;
    // free all the data structures it uses

#define FREE(a) if ((a) != NULL) free(a)
    
    // Free pt_script_pano first
    FREE(ptr->pano.projectionParmsString);
    
    FREE(ptr->pano.outputFormat);
    FREE(ptr->pano.bitDepthOutput);

    for (i=0;i<ptr->iOutputImagesCount;i++) {
	FREE(ptr->outputImageSpec[i].name);
    }
    for (i=0;i<ptr->iInputImagesCount;i++) {
	FREE(ptr->inputImageSpec[i].name);
        FREE(ptr->inputImageSpec[i].morphPoints);
    }
    FREE(ptr->outputImageSpec);
    FREE(ptr->inputImageSpec);
    FREE(morphPointsSpec);
    FREE(ptr->ctrlPointsSpec);

#undef FREE    
    // clear the data structure
    memset(ptr, 0, sizeof(*ptr));
}

// ACCESSORS

float panoScriptGetImageCoefA(pt_script_image *pImage)
{
    assert(pImage != NULL);
    return pImage->coef[0];
}

float panoScriptGetImageCoefB(pt_script_image *pImage)
{
    assert(pImage != NULL);
    return pImage->coef[1];
}

float panoScriptGetImageCoefC(pt_script_image *pImage)
{
    assert(pImage != NULL);
    return pImage->coef[2];
}

float panoScriptGetImageCoefD(pt_script_image *pImage)
{
    assert(pImage != NULL);
    return pImage->coef[3];
}

float panoScriptGetImageCoefE(pt_script_image *pImage)
{
    assert(pImage != NULL);
    return pImage->coef[4];
}

float panoScriptGetImageSheerX(pt_script_image *pImage)
{
    assert(pImage != NULL);
    return pImage->coef[5];
}

float panoScriptGetImageSheerY(pt_script_image *pImage)
{
    assert(pImage != NULL);
    return pImage->coef[6];
}

int panoScriptGetImageCropType(pt_script_image *pImage)
{
    assert(pImage != NULL);
    return pImage->cropType;
}


int panoScriptGetImageMorphToFit(pt_script_image *pImage)
{
    assert(pImage != NULL);
    return pImage->morphToFit;
}


int panoScriptGetInputImagesCount(pt_script *script)
{
    assert(script != NULL);
    return script->iInputImagesCount;
}


int panoScriptGetOutputImagesCount(pt_script *script)
{
    assert(script != NULL);
    return script->iOutputImagesCount;
}

pt_script_image *panoScriptGetInputImage(pt_script *script, int i)
{
    assert(script != NULL);
    if (i > panoScriptGetInputImagesCount(script)) {
	return NULL;
    }
    return &(script->inputImageSpec[i]);
}

pt_script_image *panoScriptGetOutputImage(pt_script *script, int i)
{
    assert(script != NULL);
    if (i > panoScriptGetOutputImagesCount(script)) {
	printf("NONE********\n");
	return NULL;
    }
    return &(script->outputImageSpec[i]);
}

int panoScriptGetPanoProjection(pt_script *script)
{
    return script->pano.projection;
}

int panoScriptGetPanoWidth(pt_script *script)
{
    return script->pano.width;
}

int panoScriptGetPanoHeight(pt_script *script)
{
    return script->pano.height;
}

float panoScriptGetPanoHFOV(pt_script *script)
{
    return script->pano.fHorFOV;
}

float panoScriptGetPanoParmsCount(pt_script *script)
{
    return script->pano.projectionParmsCount;
}

float panoScriptGetPanoParm(pt_script *script, int index)
{
    assert(index < PANO_PARSER_MAX_PROJECTION_PARMS);
    return script->pano.projectionParms[index];
}

char *panoScriptGetPanoOutputFormat(pt_script *script)
{
    return script->pano.outputFormat;
}

int panoScriptGetImageProjection(pt_script_image *pImage)
{
    assert(pImage!= NULL);
    return pImage->projection;
}

char *panoScriptGetName(pt_script_image *pImage)
{
    assert(pImage!= NULL);
    return pImage->name;
}



float panoScriptGetImageHFOV(pt_script_image *pImage)
{
    assert(pImage!= NULL);
    return pImage->fHorFOV;
}

float panoScriptGetImagePitch(pt_script_image *pImage)
{
    assert(pImage!= NULL);
    return pImage->pitch;
}
float panoScriptGetImageYaw(pt_script_image *pImage)
{
    assert(pImage!= NULL);
    return pImage->yaw;
}
float panoScriptGetImageRoll(pt_script_image *pImage)
{
    assert(pImage!= NULL);
    return pImage->roll;
}

char *panoScriptGetInputFileNameOfImage(pt_script* script,int index)
{
    pt_script_image *pImage;
    if (index <  panoScriptGetInputImagesCount(script)) {
        pImage = panoScriptGetInputImage(script, index);
        assert(pImage != NULL);
        return panoScriptGetName(pImage);
        
    } else
        return NULL;
}


char *panoScriptGetOutputFileNameOfImage(pt_script* script,int index)
{
    pt_script_image *pImage;
    if (index <  panoScriptGetOutputImagesCount(script)) {
        pImage = panoScriptGetOutputImage(script, index);
        assert(pImage != NULL);
        return panoScriptGetName(pImage);
        
    } else
        return NULL;
}
