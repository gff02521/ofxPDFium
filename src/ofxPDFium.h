#pragma once

#include "ofMain.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>
#include <string>
#include <vector>
#include <utility>

#include "fpdf_annot.h"
#include "fpdf_attachment.h"
#include "fpdf_catalog.h"
#include "fpdf_dataavail.h"
#include "fpdf_doc.h"
#include "fpdf_edit.h"
#include "fpdf_ext.h"
#include "fpdf_flatten.h"
#include "fpdf_formfill.h"
#include "fpdf_fwlevent.h"
#include "fpdf_ppo.h"
#include "fpdf_progressive.h"
#include "fpdf_save.h"
#include "fpdf_searchex.h"
#include "fpdf_structtree.h"
#include "fpdf_sysfontinfo.h"
#include "fpdf_text.h"
#include "fpdf_transformpage.h"
#include "fpdfview.h"
#include "cpp/fpdf_deleters.h"
#include "cpp/fpdf_scopers.h"

using namespace std;

class ofxPDFium
{
public:
	ofxPDFium(std::string file);
	~ofxPDFium();

	void loadPage(int pageNum);
	void draw(float x, float y, float z = 0.);
	void draw(float x, float y, float w, float h);
	void drawToScale(float x, float y, float scale);
	void drawToHeight(float x, float y, float heightToScaleTo);
	void drawToWidth(float x, float y, float widthToScaleTo);

	int getCurrentPage() { return m_currPage; }
	int getPageCount() { return m_pageCount; }

protected:
	void WritePpm(int num, const char* buffer, int stride, int width, int height);
	static int Get_Block(void* param, unsigned long pos, unsigned char* pBuf, unsigned long size);
	static FPDF_BOOL Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size);
	static void Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size);
	static int Form_Alert(IPDF_JSPLATFORM*, FPDF_WIDESTRING, FPDF_WIDESTRING, int, int);

	class TestLoader
	{
	public:
		TestLoader(const char* pBuf, size_t len);

		const char* m_pBuf;
		size_t m_Len;
	};

	void loadpdf();
	void releaseBuf();

protected:
	ofImage  m_pdfImg;
	char * m_fileBuf;
	size_t m_bufLen;
	std::string m_file;
	int m_currPage;
	int m_pageCount;


public:
	static void PdfLibInit();
	static void PdfLibCleanup();

protected:
	static bool pdfInitFlag;
	static void Unsupported_Handler(UNSUPPORT_INFO*, int type);
};