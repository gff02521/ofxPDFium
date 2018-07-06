#include "ofxPDFium.h"
#include <algorithm>

/************sys init/cleanup**************/
bool ofxPDFium::pdfInitFlag = false;

void ofxPDFium::PdfLibInit()
{
	if (!pdfInitFlag)
	{
		FPDF_LIBRARY_CONFIG config;
		config.version = 2;
		config.m_pUserFontPaths = NULL;
		config.m_pIsolate = NULL;
		config.m_v8EmbedderSlot = 0;

		FPDF_InitLibraryWithConfig(&config);

		pdfInitFlag = true;
	}
}

void ofxPDFium::PdfLibCleanup()
{
	if (pdfInitFlag)
	{
		FPDF_DestroyLibrary();
	}
}

void ofxPDFium::Unsupported_Handler(UNSUPPORT_INFO*, int type) 
{
  std::string feature = "Unknown";
  switch (type) {
    case FPDF_UNSP_DOC_XFAFORM:
      feature = "XFA";
      break;
    case FPDF_UNSP_DOC_PORTABLECOLLECTION:
      feature = "Portfolios_Packages";
      break;
    case FPDF_UNSP_DOC_ATTACHMENT:
    case FPDF_UNSP_ANNOT_ATTACHMENT:
      feature = "Attachment";
      break;
    case FPDF_UNSP_DOC_SECURITY:
      feature = "Rights_Management";
      break;
    case FPDF_UNSP_DOC_SHAREDREVIEW:
      feature = "Shared_Review";
      break;
    case FPDF_UNSP_DOC_SHAREDFORM_ACROBAT:
    case FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM:
    case FPDF_UNSP_DOC_SHAREDFORM_EMAIL:
      feature = "Shared_Form";
      break;
    case FPDF_UNSP_ANNOT_3DANNOT:
      feature = "3D";
      break;
    case FPDF_UNSP_ANNOT_MOVIE:
      feature = "Movie";
      break;
    case FPDF_UNSP_ANNOT_SOUND:
      feature = "Sound";
      break;
    case FPDF_UNSP_ANNOT_SCREEN_MEDIA:
    case FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA:
      feature = "Screen";
      break;
    case FPDF_UNSP_ANNOT_SIG:
      feature = "Digital_Signature";
      break;
  }
  printf("Unsupported feature: %s.\n", feature.c_str());
}
/************sys init/cleanup**************/

ofxPDFium::ofxPDFium(std::string file)
	: m_file(file), m_fileBuf(NULL), m_currPage(-1), m_pageCount(0)
{
	loadpdf();
}

ofxPDFium::~ofxPDFium()
{
	releaseBuf();
}

void ofxPDFium::releaseBuf()
{
	free(m_fileBuf);
	m_fileBuf = NULL;
	m_bufLen = 0;
}

void ofxPDFium::loadpdf()
{
	if (m_file.size() > 4 && m_file.substr(m_file.size()-4,4) == ".pdf")
	{
		FILE* file = fopen(m_file.c_str(), "rb");
		if (!file)
		{
			fprintf(stderr, "Failed to open: %s\n", m_file.c_str());
		}
		else
		{
			(void) fseek(file, 0, SEEK_END);
			size_t len = ftell(file);
			(void) fseek(file, 0, SEEK_SET);
			m_fileBuf = (char*) malloc(len);
			size_t ret = fread(m_fileBuf, 1, len, file);
			(void) fclose(file);
			m_bufLen = len;
			if (ret != len) 
			{
				fprintf(stderr, "Failed to read: %s\n", m_file.c_str());
				releaseBuf();
			}
		}
	}
}

void ofxPDFium::loadPage(int pageNum)
{
	IPDF_JSPLATFORM platform_callbacks;
	memset(&platform_callbacks, '\0', sizeof(platform_callbacks));
	platform_callbacks.version = 1;
	platform_callbacks.app_alert = Form_Alert;

	FPDF_FORMFILLINFO form_callbacks;
	memset(&form_callbacks, '\0', sizeof(form_callbacks));
	form_callbacks.version = 1;
	form_callbacks.m_pJsPlatform = &platform_callbacks;

	TestLoader loader(m_fileBuf, m_bufLen);

	FPDF_FILEACCESS file_access;
	memset(&file_access, '\0', sizeof(file_access));
	file_access.m_FileLen = static_cast<unsigned long>(m_bufLen);
	file_access.m_GetBlock = Get_Block;
	file_access.m_Param = &loader;

	FX_FILEAVAIL file_avail;
	memset(&file_avail, '\0', sizeof(file_avail));
	file_avail.version = 1;
	file_avail.IsDataAvail = Is_Data_Avail;

	FX_DOWNLOADHINTS hints;
	memset(&hints, '\0', sizeof(hints));
	hints.version = 1;
	hints.AddSegment = Add_Segment;

	FPDF_DOCUMENT doc;
	FPDF_AVAIL pdf_avail = FPDFAvail_Create(&file_avail, &file_access);

	(void) FPDFAvail_IsDocAvail(pdf_avail, &hints);

	if (!FPDFAvail_IsLinearized(pdf_avail)) 
	{
		printf("Non-linearized path...\n");
		doc = FPDF_LoadCustomDocument(&file_access, NULL);
	} 
	else 
	{
		printf("Linearized path...\n");
		doc = FPDFAvail_GetDocument(pdf_avail, NULL);
	}

	(void) FPDF_GetDocPermissions(doc);
	(void) FPDFAvail_IsFormAvail(pdf_avail, &hints);

	FPDF_FORMHANDLE form = FPDFDOC_InitFormFillEnvironment(doc, &form_callbacks);
	FPDF_SetFormFieldHighlightColor(form, 0, 0xFFE4DD);
	FPDF_SetFormFieldHighlightAlpha(form, 100);

	int first_page = FPDFAvail_GetFirstPageNum(doc);
	(void) FPDFAvail_IsPageAvail(pdf_avail, first_page, &hints);

	int page_count = FPDF_GetPageCount(doc);
	for (int i = 0; i < page_count; ++i)
	{
		(void) FPDFAvail_IsPageAvail(pdf_avail, i, &hints);
	}
	m_pageCount = page_count;

	if (pageNum >= 0 && pageNum < page_count)
	{
		m_currPage = pageNum;

		FORM_DoDocumentJSAction(form);
		FORM_DoDocumentOpenAction(form);

		FPDF_PAGE page = FPDF_LoadPage(doc, pageNum);
		FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);

		FORM_OnAfterLoadPage(page, form);
		FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_OPEN);

		int width = static_cast<int>(FPDF_GetPageWidth(page));
		int height = static_cast<int>(FPDF_GetPageHeight(page));
		FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
		//(bitmap, 0, 0, width, height, 255, 255, 255, 255);
		FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xffffffff);

		FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
		FPDF_FFLDraw(form, bitmap, page, 0, 0, width, height, 0, 0);

		const char* buffer = reinterpret_cast<const char*>(FPDFBitmap_GetBuffer(bitmap));
		int stride = FPDFBitmap_GetStride(bitmap);
		WritePpm(pageNum, buffer, stride, width, height);

		FPDFBitmap_Destroy(bitmap);

		FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_CLOSE);
		FORM_OnBeforeClosePage(page, form);
		FPDFText_ClosePage(text_page);
		FPDF_ClosePage(page);
	}

	FORM_DoDocumentAAction(form, FPDFDOC_AACTION_WC);	
	FPDFDOC_ExitFormFillEnvironment(form);
	FPDF_CloseDocument(doc);
	FPDFAvail_Destroy(pdf_avail);
}

void ofxPDFium::draw(float x, float y, float z)
{
	if (m_fileBuf && m_currPage >= 0)
	{
		m_pdfImg.draw(x, y, z);
	}
}

void ofxPDFium::draw(float x, float y, float w, float h)
{
	if (m_fileBuf && m_currPage >= 0)
	{
		m_pdfImg.draw(x, y, w, h);
	}
}

void ofxPDFium::drawToScale(float x, float y, float scale)
{
	draw(x, y, m_pdfImg.getWidth() * scale, m_pdfImg.getHeight() * scale);
}
void ofxPDFium::drawToHeight(float x, float y, float heightToScaleTo)
{
	float scale = heightToScaleTo/m_pdfImg.getHeight();
	drawToScale(x, y, scale);
}
void ofxPDFium::drawToWidth(float x, float y, float widthToScaleTo)
{
	float scale = widthToScaleTo/m_pdfImg.getWidth();
	drawToScale(x, y, scale);
}

void ofxPDFium::WritePpm(int num, const char* buffer, int stride, int width, int height) 
{
	if (stride < 0 || width < 0 || height < 0)
		return;
	if (height > 0 && width > INT_MAX / height)
		return;
	int out_len = width * height;
	if (out_len > INT_MAX / 3)
		return;
	out_len *= 3;

	// Source data is B, G, R, unused.
	// Dest data is R, G, B.
	char* result = new char[out_len];
	if (result)
	{
		for (int h = 0; h < height; ++h)
		{
			const char* src_line = buffer + (stride * h);
			char* dest_line = result + (width * h * 3);
			for (int w = 0; w < width; ++w)
			{
				// R
				dest_line[w * 3] = src_line[(w * 4) + 2];
				// G
				dest_line[(w * 3) + 1] = src_line[(w * 4) + 1];
				// B
				dest_line[(w * 3) + 2] = src_line[w * 4];
			}
		}

		//ofBuffer buf(result, out_len);
		ofPixels pix;
		pix.setFromExternalPixels((unsigned char*)result, width, height, 3);
		m_pdfImg.clear();
		m_pdfImg.setFromPixels(pix);
		m_pdfImg.setUseTexture(true);

		delete [] result;
	}
}



ofxPDFium::TestLoader::TestLoader(const char* pBuf, size_t len)
    : m_pBuf(pBuf), m_Len(len)
{
}

int ofxPDFium::Get_Block(void* param, unsigned long pos, unsigned char* pBuf,
              unsigned long size) 
{
  ofxPDFium::TestLoader* pLoader = (ofxPDFium::TestLoader*) param;
  if (pos + size < pos || pos + size > pLoader->m_Len) return 0;
  memcpy(pBuf, pLoader->m_pBuf + pos, size);
  return 1;
}

FPDF_BOOL ofxPDFium::Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size)
{
  return true;
}

int ofxPDFium::Form_Alert(IPDF_JSPLATFORM*, FPDF_WIDESTRING, FPDF_WIDESTRING, int, int) 
{
  printf("Form_Alert called.\n");
  return 0;
}

void ofxPDFium::Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) 
{
}