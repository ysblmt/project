#include <pic_operation.h>

/* 自定义错误处理函数：参考libjpeg中的示例程序 */
typedef struct MyErrorMgr
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
}T_MyErrorMgr, *PT_MyErrorMgr;

static void MyErrorExit(j_common_ptr ptCInfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	T_MyErrorMgr myerr = (T_MyErrorMgr) ptCInfo->err;
	
	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*ptCInfo->err->output_message) (ptCInfo);
	
	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

static int isJPGFormat(PT_FileMap ptFileMap)
{
	struct jpeg_compress_struct tDInfo;

	//struct jpeg_error_mgr tJerr; /* default */
	T_MyErrorMgr tJerr;
	int iRet;

	fseek(ptFileMap->tFp, 0, SEEK_SET); /* 重定位流上的文件指针：偏移值为0，文件开头SEEK_SET */

	/* Step 1: allocate and initialize JPEG compression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	tDInfo.err = jpeg_std_error(&tJerr.pub);
	tJerr.pub.error_exit = MyErrorExit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if(setjmp(tJerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
     	 * We need to clean up the JPEG object, close the input file, and return.
    	 */
		jpeg_destroy_decompress(&tDInfo);
     	return 0;
	}
	/* Now we can initialize the JPEG compression object. */
 	jpeg_create_compress(&tDInfo);

	/* Step 2: specify data source (eg, a file) */
	
	jpeg_stdio_src(&tDInfo, ptFileMap->tFp);

	/* Step 3: read file parameters with jpeg_read_header() */

	iRet = jpeg_read_header(&tDInfo, TRUE);
	
	/*You can abort(中止) a decompression cycle(解压循环) by calling jpeg_destroy_decompress() or
	jpeg_destroy() if you don't need the JPEG object any more, or
	jpeg_abort_decompress() or jpeg_abort() if you want to reuse the object.
	The previous discussion of aborting compression cycles applies here too.*/
	jpeg_abort_decompress(&tDInfo);

 	return (iRet == JPEG_HEADER_OK);
}

static int CovertOneLine(int iWidth, int iSrcBpp, int iDstBpp, unsigned char *pudSrcDatas, unsigned char *pudDstDatas)
{
	unsigned int dwRed;
	unsigned int dwGreen;
	unsigned int dwBlue;
	unsigned int dwColor;

	unsigned short *pwDstDatas16bpp = (unsigned short *)pudDstDatas;
	unsigned int   *pwDstDatas32bpp = (unsigned int *)pudDstDatas;

	int i;
	int pos = 0;

	if (iSrcBpp != 24)
	{
		return -1;
	}

	if (iDstBpp == 24)
	{
		memcpy(pudDstDatas, pudSrcDatas, iWidth*3);
	}
	else
	{
		for (i = 0; i < iWidth; i++)
		{
			dwRed   = pudSrcDatas[pos++];
			dwGreen = pudSrcDatas[pos++];
			dwBlue  = pudSrcDatas[pos++];
			if (iDstBpp == 32)
			{
				dwColor = (dwRed << 16) | (dwGreen << 8) | dwBlue;
				*pwDstDatas32bpp = dwColor;
				pwDstDatas32bpp++;
			}
			else if (iDstBpp == 16)
			{
				/* 565 */
				dwRed   = dwRed >> 3;
				dwGreen = dwGreen >> 2;
				dwBlue  = dwBlue >> 3;
				dwColor = (dwRed << 11) | (dwGreen << 5) | (dwBlue);
				*pwDstDatas16bpp = dwColor;
				pwDstDatas16bpp++;
			}
		}
	}
	return 0;
}


static int GetPixelDatasFrmJPG(PT_FileMap ptFileMap, PT_PixelDatas ptPixelDatas)
{
	struct jpeg_compress_struct tDInfo;
	int row_stride; 	  /* physical row width in output buffer */
	unsigned char *aucLineBuffer = NULL;
    unsigned char *pucDest;


	//struct jpeg_error_mgr tJerr; /* default */
	T_MyErrorMgr tJerr;
	int iRet;

	fseek(ptFileMap->tFp, 0, SEEK_SET); /* 重定位流上的文件指针：偏移值为0，文件开头SEEK_SET */

	/* Step 1: allocate and initialize JPEG compression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	tDInfo.err = jpeg_std_error(&tJerr.pub);
	tJerr.pub.error_exit = MyErrorExit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if(setjmp(tJerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
     	 * We need to clean up the JPEG object, close the input file, and return.
    	 */
		jpeg_destroy_decompress(&tDInfo);
		if (ptPixelDatas->aucPixelDatas)
        {
            free(ptPixelDatas->aucPixelDatas);
        }
     	return -1;
	}
	/* Now we can initialize the JPEG compression object. */
 	jpeg_create_compress(&tDInfo);

	/* Step 2: specify data source (eg, a file) */
	
	jpeg_stdio_src(&tDInfo, ptFileMap->tFp);

	/* Step 3: read file parameters with jpeg_read_header() */

	iRet = jpeg_read_header(&tDInfo, TRUE);

	// 设置解压参数,比如放大、缩小
	tDInfo.scale_num = tDInfo.scale_denom = 1;

	/* Step 5: Start decompressor */
	
	jpeg_start_decompress(&tDInfo);

	
    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
   	 * output image dimensions available, as well as the output colormap
   	 * if we asked for color quantization.
     */
    /* JSAMPLEs per row in output buffer */
  	row_stride = cinfo.output_width * cinfo.output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	aucLineBuffer = malloc(row_stride);
	assert(NULL != aucLineBuffer);
	
	ptPixelDatas->iWidth = tDInfo.output_width;
	ptPixelDatas->iHeight = tDInfo.output_height;
	ptPixelDatas->iLineBytes    = ptPixelDatas->iWidth * ptPixelDatas->iBpp / 8;
    ptPixelDatas->iTotalBytes   = ptPixelDatas->iHeight * ptPixelDatas->iLineBytes;
	ptPixelDatas->aucPixelDatas = malloc(ptPixelDatas->iTotalBytes);
	assert(NULL != ptPixelDatas->aucPixelDatas);

	pucDest = ptPixelDatas->aucPixelDatas;

	/* Step 6: while (scan lines remain to be read) */
	/*			 jpeg_read_scanlines(...); */
	
	while (tDInfo.output_scanline < tDInfo.output_height) 
	{
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could ask for
		 * more than one scanline at a time if that's more convenient.
		 */
		(void) jpeg_read_scanlines(&tDInfo, &aucLineBuffer, 1);
		// 转到ptPixelDatas去
		CovertOneLine(ptPixelDatas->iWidth, 24, ptPixelDatas->iBpp, aucLineBuffer, pucDest);
		pucDest += ptPixelDatas->iLineBytes;
	}
	
	/* Step 7: Finish decompression */
	free(aucLineBuffer);
	jpeg_finish_decompress(&tDInfo);
	jpeg_destroy_decompress(&tDInfo);
	
	return 0;
}

static int FreePixelDatasForJPG(PT_PixelDatas ptPixelDatas)
{
	free(ptPixelDatas->aucPixelDatas);
	return 0;
}


static T_PicFileParser g_tJPGParser = {
	.name           = "jpg",
	.isSupport      = isJPGFormat,
	.GetPixelDatas  = GetPixelDatasFrmJPG,
	.FreePixelDatas = FreePixelDatasForJPG,	
};

int JPGParserInit(void)
{
	return RegisterPicFileParser(&g_tJPGParser);
}


