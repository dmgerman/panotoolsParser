/*
 *  tparser.h
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

#ifndef __TPARSER_H__
#define __TPARSER_H__


/* Maximum size for an input token */
#define PARSER_MAX_LINE 1000
#define PT_TOKEN_MAX_LEN PARSER_MAX_LINE

#define PANO_PARSER_MAX_PROJECTION_PARMS 10
/* 
 Data structure where the entire input file will be read 
*/

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#define PANO_PARSER_COEF_COUNT 7
#define PANO_PARSER_RESP_CURVE_COEF_COUNT 5
#define PANO_PARSER_VIGN_COEF_COUNT 6

typedef struct {
    double x;
    double y;
} pt_point;

typedef struct {
    int iImage;
    pt_point p1;
    pt_point p2;
} pt_script_morph_point;

typedef struct {
    int iImage1;
    int iImage2;
    pt_point p1;
    pt_point p2;
    int type;
} pt_script_ctrl_point;

typedef struct {
    int width;
    int height;
    int projection;
    int projectionParmsCount;
    float projectionParms[PANO_PARSER_MAX_PROJECTION_PARMS];
    char *projectionParmsString;
    char *projectionName;
    float fHorFOV;
    char *outputFormat;  // n : file format of output

    // Hugin parameters
    int dynamicRangeMode; // R[01] 0 -> LDR; 1 -> HDR
    char *bitDepthOutput; // T bitdepth of output images, possible values are
    //XXX TO BE IMPLEMENTED
    float exposureValue;  // E exposure value of final panorama
}  pt_script_pano;

typedef struct {
    int projection;
    int width;
    int height;
    float fHorFOV;
    float yaw;
    float pitch;
    float roll;

    int featherSize;
    float coef[PANO_PARSER_COEF_COUNT]; // a, b, c, d, e, g, t


    // These are hugin parameters
    int autoCenterCrop;
    float cropFactor;
    
    // Exposure related
    float imageEV;  // Exposure value of image Eev
    float whiteBalanceFactorRed;  // Er
    float whiteBalanceFactorBlue; // Eb
    
    int responseCurveType; // Ry 0 -> EMoR, 1 -> linear
    float responseCurveCoef[PANO_PARSER_RESP_CURVE_COEF_COUNT]; // R[abcde]

    int vignettingCorrectionMode; // Vm
    float vignettingCorrectionCoef[PANO_PARSER_VIGN_COEF_COUNT]; // V[abcdxy]

    char *name;
    char cropType ; // it can be 'S' or  'C'
    int cropArea[PANO_PARSER_COEF_COUNT]; // the rectangle to crop to
    int morphToFit;  // true if morph to fit
    // pointers to variable if they are used.
    // For the sake of simplicity they start at 1, if they are zero they are unused
    int fHorFOVIndex;
    int yawIndex;
    int pitchIndex;
    int rollIndex;
    int coefIndex[PANO_PARSER_COEF_COUNT]; // a, b, c, d, e       , g, t

    // hugin indexes for de-referencing

    int imageEVIndex; //Exposure value of image
    int whiteBalanceFactorRedIndex;  // Er
    int whiteBalanceFactorBlueIndex; // Eb
    int vignettingCorrectionModeIndex; // Vm
    int vignettingCorrectionCoefIndex[PANO_PARSER_VIGN_COEF_COUNT]; // V[abcdxy]

    int responseCurveCoefIndex[PANO_PARSER_RESP_CURVE_COEF_COUNT]; // R[abcde]

    // Morphing Points
    int morphPointsCount;
    pt_script_morph_point *morphPoints;    
}  pt_script_image;


typedef struct {
    float fGamma;
    int interpolator;
    int fastFT;
    int huberEstimator;
    float photometricHuberSigma;
    int optimizeReferenceImage;
    int blendMode;
} pt_script_optimize;

typedef struct {
    pt_script_pano pano;
    int iInputImagesCount;
    pt_script_image *inputImageSpec;
    int iOutputImagesCount;
    pt_script_image *outputImageSpec;
    int iMorphPointsCount;
    pt_script_optimize optimize;
    int iCtrlPointsCount;
    pt_script_ctrl_point *ctrlPointsSpec;
}  pt_script;



void panoScriptParserSetDefaults(pt_script *ptr);
pt_script *panoScriptParse(char *, int , pt_script *);
void panoScriptParserDispose(pt_script *ptr);
void panoScriptDump(pt_script *script);


// Accessors to the data structures above...
// they  are the preferred way to access the data in them

float panoScriptGetImageCoefA(pt_script_image *pImage);
float panoScriptGetImageCoefB(pt_script_image *pImage);
float panoScriptGetImageCoefC(pt_script_image *pImage);
float panoScriptGetImageCoefD(pt_script_image *pImage);
float panoScriptGetImageCoefE(pt_script_image *pImage);
float panoScriptGetImageSheerX(pt_script_image *pImage);
float panoScriptGetImageSheerY(pt_script_image *pImage);
int panoScriptGetImageCropType(pt_script_image *pImage);
int panoScriptGetImageMorphToFit(pt_script_image *pImage);
int panoScriptGetImageProjection(pt_script_image *pImage);
float panoScriptGetImageHFOV(pt_script_image *pImage);
float panoScriptGetImagePitch(pt_script_image *pImage);
float panoScriptGetImageYaw(pt_script_image *pImage);
float panoScriptGetImageRoll(pt_script_image *pImage);
char *panoScriptGetName(pt_script_image *pImage);

int panoScriptGetInputImagesCount(pt_script *script);
int panoScriptGetOutputImagesCount(pt_script *script);
pt_script_image *panoScriptGetInputImage(pt_script *script, int i);
pt_script_image *panoScriptGetOutputImage(pt_script *script, int i);
int panoScriptGetPanoProjection(pt_script *script);
int panoScriptGetPanoWidth(pt_script *script);
int panoScriptGetPanoHeight(pt_script *script);
float panoScriptGetPanoHFOV(pt_script *script);
float panoScriptGetPanoParmsCount(pt_script *script);
float panoScriptGetPanoParm(pt_script *script, int index);
char *panoScriptGetPanoOutputFormat(pt_script *script);
int panoScriptDeReferenceVariables(pt_script* script);
char *panoScriptGetInputFileNameOfImage(pt_script* script,int index);
char *panoScriptGetOutputFileNameOfImage(pt_script* script,int index);
void panoParserDispose(pt_script **scriptVar);



#endif

