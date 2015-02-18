/*
 * jpegtran.c
 *
 * Copyright (C) 1995-2010, Thomas G. Lane, Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a command-line user interface for JPEG transcoding.
 * It is very similar to cjpeg.c, and partly to djpeg.c, but provides
 * lossless transcoding between different JPEG file formats.  It also
 * provides some lossless and sort-of-lossless transformations of JPEG data.
 */
#include "stdafx.h"

#include "cdjpeg.h" // Common decls for cjpeg/djpeg applications
#include "transupp.h" // Support routines for jpegtran

unsigned int ChangeOrientation(const wchar_t* FilenameIn, const wchar_t* FilenameOut, unsigned short OrientationFlag, unsigned char Clockwise, unsigned char Trim)
{
	struct jpeg_decompress_struct srcinfo;
	struct jpeg_compress_struct dstinfo;
	struct jpeg_error_mgr jsrcerr, jdsterr;

	jvirt_barray_ptr * src_coef_arrays;
	jvirt_barray_ptr * dst_coef_arrays;

	/* We assume all-in-memory processing and can therefore use only a
	* single file pointer for sequential input and output operation. 
	*/
	FILE * fp;

	jpeg_transform_info transformoption;
	JCOPY_OPTION copyoption = JCOPYOPT_ALL;

	switch (OrientationFlag)
	{
	case 1U:
		{
			transformoption.transform = (Clockwise == 1) ? JXFORM_ROT_90 : JXFORM_ROT_270;
		}
		break;
	case 2U:
		{
			transformoption.transform = JXFORM_FLIP_H;
		}
		break;
	case 3U:
		{
			transformoption.transform = JXFORM_ROT_180;
		}
		break;
	case 4U:
		{
			transformoption.transform = JXFORM_FLIP_V;
		}
		break;
	case 5U:
		{
			transformoption.transform = JXFORM_TRANSPOSE;
		}
		break;
	case 6U:
		{
			transformoption.transform = JXFORM_ROT_90;
		}
		break;
	case 7U:
		{
			transformoption.transform = JXFORM_TRANSVERSE;
		}
		break;
	case 8U:
		{
			transformoption.transform = JXFORM_ROT_270;
		}
		break;
	default:
		{
			transformoption.transform = JXFORM_NONE;
		}
		break;
	}
	
	transformoption.perfect = (Trim == 1) ? FALSE : TRUE;
	transformoption.trim = (Trim == 1) ? TRUE : FALSE;
	transformoption.force_grayscale = FALSE;
	transformoption.crop = FALSE;

	// Initialize the JPEG decompression object with default error handling
	srcinfo.err = jpeg_std_error(&jsrcerr);
	jpeg_create_decompress(&srcinfo);
	// Initialize the JPEG compression object with default error handling
	dstinfo.err = jpeg_std_error(&jdsterr);
	jpeg_create_compress(&dstinfo);

	// Note: we assume only the decompression object will have virtual arrays

	dstinfo.optimize_coding = TRUE;
	dstinfo.err->trace_level = 0;
	//srcinfo.mem->max_memory_to_use = dstinfo.mem->max_memory_to_use;

	// Open input file
	if (_wfopen_s(&fp, FilenameIn, L"rb") != 0)
	{
		return 110; // ERROR_OPEN_FAILED
    }

	// Specify data source for decompression
	jpeg_stdio_src(&srcinfo, fp);

	// Enable saving of extra markers that we want to copy
	jcopy_markers_setup(&srcinfo, copyoption);

	// Read file header
	(void) jpeg_read_header(&srcinfo, TRUE);

	if (!jtransform_request_workspace(&srcinfo, &transformoption))
	{
		fclose(fp);
		return 13; // ERROR_INVALID_DATA
	}

	// Read source file as DCT coefficients
	src_coef_arrays = jpeg_read_coefficients(&srcinfo);

	// Initialize destination compression parameters from source values
	jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

	/* Adjust destination parameters if required by transform options;
	* also find out which set of coefficient arrays will hold the output.
	*/
	dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo, src_coef_arrays, &transformoption);

	// Close input file
	/* Note: we assume that jpeg_read_coefficients consumed all input
	* until JPEG_REACHED_EOI, and that jpeg_finish_decompress will
	* only consume more while (! cinfo->inputctl->eoi_reached).
	* We cannot call jpeg_finish_decompress here since we still need the
	* virtual arrays allocated from the source object for processing.
	*/
	fclose(fp);

	// Open the output file
	if (_wfopen_s(&fp, FilenameOut, L"wb") != 0)
	{
		return 82; // ERROR_CANNOT_MAKE
	}

	// Set progressive mode (saves space but is slower)
	jpeg_simple_progression(&dstinfo);

	// Specify data destination for compression
	jpeg_stdio_dest(&dstinfo, fp);

	// Start compressor (note no image data is actually written here)
	jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

	// Copy to the output file any extra markers that we want to preserve
	jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);

	// Execute image transformation, if any
	jtransform_execute_transformation(&srcinfo, &dstinfo, src_coef_arrays, &transformoption);

	// Finish compression and release memory
	jpeg_finish_compress(&dstinfo);
	jpeg_destroy_compress(&dstinfo);
	(void) jpeg_finish_decompress(&srcinfo);
	jpeg_destroy_decompress(&srcinfo);

	// Close output file
    fclose(fp);

	return 0;
}
