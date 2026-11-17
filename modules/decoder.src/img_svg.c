
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <ldg.h>
#include <ctype.h>

#include "../../src/global.h"
#include "../../src/img-dcdr.h"

#include <plutosvg.h>

BOOL local_FastImages = FALSE;

void (*local_errprintf)(const char *s, ...);

BOOL CDECL decSvg_start (IMGINFO info, void * errfunc);
void CDECL blend_row (IMGINFO info, CHAR * row);
BOOL CDECL decSvg_read (IMGINFO info, CHAR * buffer);
void CDECL decSvg_quit (IMGINFO info);
const char* CDECL decSvg_info(void);
void CDECL decSvg_setFastImage(BOOL set_FastImages);

static UWORD svg_line_read;

/*----------------------------------------------------------------------------*/
BOOL CDECL decSvg_start (IMGINFO info, void * errfunc)
{
	BOOL                 res      = TRUE;
  plutosvg_document_t* document = NULL;
  plutovg_surface_t*   surface  = NULL;
  const char*          id       = NULL;

  svg_line_read  = 0;

  local_errprintf = errfunc;
 
  document = plutosvg_document_load_from_data((const char *)info->_priv_load, info->_priv_size, -1, -1, NULL, NULL);
    
  if (document == NULL) { local_errprintf("decSvg_start(): could not instanciate document from datas\n"); res = FALSE; goto cleanup; }

#ifdef SVG_DBG
  local_errprintf("svg_document size: %d x %d px\n", (int)plutosvg_document_get_width(document), (int)plutosvg_document_get_height(document));
#endif

  surface = plutosvg_document_render_to_surface(document, id, -1, -1, NULL, NULL, NULL);

  if (surface == NULL) { local_errprintf("decSvg_start(): could not render to surface\n"); res = FALSE; goto cleanup; }

#ifdef SVG_DBG
  local_errprintf("svg_surface size: %d x %d px\n", plutovg_surface_get_width(surface), plutovg_surface_get_height(surface));
#endif

	info->_priv_data = surface;
  info->_priv_more = document;
	info->read       = decSvg_read;
	info->quit       = decSvg_quit;
	info->ImgWidth   = plutovg_surface_get_width(surface);
	info->ImgHeight  = plutovg_surface_get_height(surface);
	info->BitDepth   = 8;
  info->NumComps   = 3;
  info->NumColors  = 0;
  //info->Palette    = NULL;
  //info->PalRpos    = 0;
  //info->PalGpos    = 0;
  //info->PalBpos    = 0;
  //info->PalStep    = 0;
	info->Alpha      = TRUE;
  info->Transp     = -1;
  //info->Interlace  = 0;
	
cleanup:
  if (!res)
  {
    if (surface)  { plutovg_surface_destroy(surface); }
    if (document) { plutosvg_document_destroy(document); }
  }

  return res;
}

/*----------------------------------------------------------------------------*/
/* Composite a row of RGBA over info->AlphaBg, in place, leaving the RGB the
 * rasterizers expect.  Most pixels are fully opaque or fully clear, so only
 * the edges pay for the multiplies.
*/
void blend_row (IMGINFO info, CHAR * row)
{
	const UWORD     r_bg = (UWORD)(info->AlphaBg >>16) & 0xFF;
	const UWORD     g_bg = (UWORD)(info->AlphaBg >> 8) & 0xFF;
	const UWORD     b_bg = (UWORD)(info->AlphaBg     ) & 0xFF;
	/* FAST_IMAGES cuts at half way, so nothing is left to blend */
	const UWORD     solid = (local_FastImages ? 128 : 255);
	const UWORD     clear = (local_FastImages ? 127 :   0);
	unsigned char * src  = (unsigned char *)row;
	unsigned char * dst  = src;
	UWORD           n    = info->ImgWidth;

	while (n--) {
		UWORD a = src[3];
		if (a >= solid) {
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
		} else if (a <= clear) {
			dst[0] = r_bg;
			dst[1] = g_bg;
			dst[2] = b_bg;
		} else {
			/* (x + (x >>8)) >>8 divides by 255, rounded, once x carries +128 */
			UWORD b = 255 - a, x;
			x = (UWORD)(src[0] * a + r_bg * b + 128); dst[0] = (x + (x >>8)) >>8;
			x = (UWORD)(src[1] * a + g_bg * b + 128); dst[1] = (x + (x >>8)) >>8;
			x = (UWORD)(src[2] * a + b_bg * b + 128); dst[2] = (x + (x >>8)) >>8;
		}
		src += 4;
		dst += 3;
	}
}

/*----------------------------------------------------------------------------*/
BOOL CDECL decSvg_read (IMGINFO info, CHAR * buffer)
{
  plutovg_surface_t *surface = (plutovg_surface_t *)info->_priv_data;
  
  if (!surface) { return FALSE; }
  if (svg_line_read >= info->ImgHeight) { return FALSE; }

  CHAR *ptr = plutovg_surface_get_data(surface);
  int   len = plutovg_surface_get_stride(surface);
  int width = plutovg_surface_get_width(surface);
  
  ptr += (svg_line_read * len);

  plutovg_convert_argb_to_rgba(buffer, ptr, width, 1, len);
  
  svg_line_read++;

	if (info->Alpha) { blend_row (info, buffer); }

  return (svg_line_read < info->ImgHeight);
}

/*----------------------------------------------------------------------------*/
void CDECL decSvg_quit (IMGINFO info)
{
  plutovg_surface_t   *surface  = (plutovg_surface_t *)info->_priv_data;
  plutosvg_document_t *document = (plutosvg_document_t *)info->_priv_more;

  if (surface)  { plutovg_surface_destroy(surface); }
  if (document) { plutosvg_document_destroy(document); }
	
  info->_priv_data = NULL;
  info->_priv_more = NULL;

  if (info->_priv_load)
  {
    ldg_Free(info->_priv_load);
    info->_priv_load = NULL;
    info->_priv_size = 0;
  }
}

/*----------------------------------------------------------------------------*/

const char* CDECL decSvg_info()
{
  return "PlutoSVG decoder (based on libplutosvg " PLUTOSVG_VERSION_STRING " and libplutovg " PLUTOVG_VERSION_STRING ")";
}

/*----------------------------------------------------------------------------*/

void CDECL decSvg_setFastImage(BOOL set_FastImages)
{
  local_FastImages = set_FastImages;
}

/*----------------------------------------------------------------------------*/

PROC LibFunc[] =
{
  {"hw_dec_open", "BOOL hw_dec_start(IMGINFO info, void * errfunc);\n", decSvg_start},
  {"hw_dec_info", "const char * hw_dec_info();\n", decSvg_info},
  {"hw_set_fast", "void hw_set_fast(BOOL set_FastImages);\n", decSvg_setFastImage},
};

LDGLIB LibLdg[] = { { 0x0001, 3, LibFunc,  "BMP decoder for HighWire", 1 } };

int main(void)
{
  ldg_init(LibLdg);
  return 0;
}
