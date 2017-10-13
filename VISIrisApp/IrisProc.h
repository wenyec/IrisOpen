/*************************************************************
*
* The file, IrisProc.h is the header file used to manipulate iris image.
*
*
**************************************************************/
#pragma once
#include "afxwin.h"
#include <string>
#include "iDataIris.h"
#include "iDataIrisStructs.h"
#include "VISIrisApp.h"

long    GetIrisImageQuality(BYTE* pImage);
void    CreateIrisTemplate(BYTE* pImage);
void    MatchIirisImage(BYTE* pImage);
