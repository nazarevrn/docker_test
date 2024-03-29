/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2006 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Stanislav Malyshev <stas@php.net>                           |
   +----------------------------------------------------------------------+
 */

/* $Id: gd_ctx.c,v 1.22.2.5 2006/01/27 13:36:29 pajoye Exp $ */

#include "php_gd.h"

#define CTX_PUTC(c,ctx) ctx->putC(ctx, c)

static void _php_image_output_putc(struct gdIOCtx *ctx, int c)
{
	/* without the following downcast, the write will fail
	 * (i.e., will write a zero byte) for all
	 * big endian architectures:
	 */
	unsigned char ch = (unsigned char) c;
	TSRMLS_FETCH();
	php_write(&ch, 1 TSRMLS_CC);
}

static int _php_image_output_putbuf(struct gdIOCtx *ctx, const void* buf, int l)
{
	TSRMLS_FETCH();
	return php_write((void *)buf, l TSRMLS_CC);
}

static void _php_image_output_ctxfree(struct gdIOCtx *ctx)
{
	if(ctx) {
		efree(ctx);
	}
}

/* {{{ _php_image_output_ctx */
static void _php_image_output_ctx(INTERNAL_FUNCTION_PARAMETERS, int image_type, char *tn, void (*func_p)())
{
	zval **imgind, **file, **quality, **basefilter;
	gdImagePtr im;
	char *fn = NULL;
	FILE *fp = NULL;
	int argc = ZEND_NUM_ARGS();
	int q = -1, i;
	int f = -1;
	gdIOCtx *ctx;

	/* The third (quality) parameter for Wbmp stands for the threshold when called from image2wbmp().
	 * The third (quality) parameter for Wbmp and Xbm stands for the foreground color index when called
	 * from imagey<type>().
	 */

	if (argc < 2 && image_type == PHP_GDIMG_TYPE_XBM) {
		WRONG_PARAM_COUNT;
	}

	if (argc < 1 || argc > 4 || zend_get_parameters_ex(argc, &imgind, &file, &quality, &basefilter) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	ZEND_FETCH_RESOURCE(im, gdImagePtr, imgind, -1, "Image", phpi_get_le_gd());

	if (argc > 1) {
		convert_to_string_ex(file);
		fn = Z_STRVAL_PP(file);
		if (argc >= 3) {
			convert_to_long_ex(quality);
			q = Z_LVAL_PP(quality);/* or colorindex for foreground of BW images (defaults to black) */
			if (argc == 4) {
				convert_to_long_ex(basefilter);
				f = Z_LVAL_PP(basefilter);
			}
		}
	}

	if ((argc == 2) || (argc > 2 && Z_STRLEN_PP(file))) {

		PHP_GD_CHECK_OPEN_BASEDIR(fn, "Invalid filename");

		fp = VCWD_FOPEN(fn, "wb");
		if (!fp) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open '%s' for writing", fn);
			RETURN_FALSE;
		}

		ctx = gdNewFileCtx(fp);
	} else {
		ctx = emalloc(sizeof(gdIOCtx));
		ctx->putC = _php_image_output_putc;
		ctx->putBuf = _php_image_output_putbuf;
#if HAVE_LIBGD204
		ctx->gd_free = _php_image_output_ctxfree;
#else
		ctx->free = _php_image_output_ctxfree;
#endif

#if APACHE && defined(CHARSET_EBCDIC)
		/* XXX this is unlikely to work any more thies@thieso.net */
		/* This is a binary file already: avoid EBCDIC->ASCII conversion */
		ap_bsetflag(php3_rqst->connection->client, B_EBCDIC2ASCII, 0);
#endif
	}

	switch(image_type) {
		case PHP_GDIMG_CONVERT_WBM:
			if(q<0||q>255) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid threshold value '%d'. It must be between 0 and 255", q);
			}
		case PHP_GDIMG_TYPE_JPG:
			(*func_p)(im, ctx, q);
			break;
		case PHP_GDIMG_TYPE_PNG:
			(*func_p)(im, ctx, q, f);
			break;
		case PHP_GDIMG_TYPE_XBM:
		case PHP_GDIMG_TYPE_WBM:
			if (argc < 3) {
				for(i=0; i < gdImageColorsTotal(im); i++) {
					if(!gdImageRed(im, i) && !gdImageGreen(im, i) && !gdImageBlue(im, i)) break;
				}
				q = i;
			}
			if (image_type == PHP_GDIMG_TYPE_XBM) {
				(*func_p)(im, fn, q, ctx);
			} else {
				(*func_p)(im, q, ctx);
			}
			break;
		default:
			(*func_p)(im, ctx);
			break;
	}

#if HAVE_LIBGD204
	ctx->gd_free(ctx);
#else
	ctx->free(ctx);
#endif

	if(fp) {
		fflush(fp);
		fclose(fp);
	}

    RETURN_TRUE;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
