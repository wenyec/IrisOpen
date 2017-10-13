/**************************************************************************************
* modify the file to image processing for the input image
* preparing the input image to adapot IrisID SDK APIs
* -- created by Wenye Cheng 10/12/2017
**************************************************************************************/

#pragma once
#include "afxwin.h"

class CPictureCtrl :
	public CStatic
{
/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:

	/* Constructor */
	CPictureCtrl(void);

	/* Destructor */
	~	CPictureCtrl(void);

/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
public:

	/* Loads an image from a file */
	BOOL	LoadFromFile(CString &szFilePath);

	/* Loads an image from an IStream interface */
	BOOL	LoadFromStream(IStream *piStream);

	/*
	 * Loads an image from a byte stream;
	 */
	BOOL	LoadFromStream(BYTE *pData, size_t nSize);

	/* Overload - Single load function */
	BOOL	Load(CString &szFilePath);
	BOOL	Load(IStream *piStream);
	BOOL	Load(BYTE *pData, size_t nSize);

	/* Frees the image data */
	void	FreeData();

	/* Cleans up image from display */
	void	FreeImage();

/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
protected:
	virtual void	PreSubclassWindow();

	/* Draws the Control */
	virtual void	DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL	OnEraseBkgnd(CDC *pDC);

/*
 -----------------------------------------------------------------------------------------------------------------------
 -----------------------------------------------------------------------------------------------------------------------
 */
private:

	/* Internal image stream buffer */
	IStream		*m_pStream;

	/* Control flag if a picture is loaded */
	BOOL		m_bIsPicLoaded;

	/* GDI Plus Token */
	ULONG_PTR	m_gdiplusToken;
};
