#define _CRT_SECURE_NO_WARNINGS

#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>

/*
    Returns byte array of loaded bitmap or NULL if there were errors.
    Assumes that fread and fseek functions won't fail.
    Free allocated memory!
*/

unsigned char * load_bitmap( const char * file )
{
    FILE *                  f;
    bitmap_file_header_t    bmp_file_hdr;
    bitmap_info_header_t    bmp_info_hdr;
    unsigned char *         img;

    if( !( f = fopen( file, "rb" ) ) ) {
        return NULL;
    }

    fread( &bmp_file_hdr, sizeof( bitmap_file_header_t ), 1, f );

    if( bmp_file_hdr.f_type != 0x4D42 ) {
        fclose( f );
        return NULL;
    }

    fread( &bmp_info_hdr, sizeof( bitmap_info_header_t ), 1, f );
    fseek( f, bmp_file_hdr.offset, SEEK_SET );

    if( !( img = (unsigned char *)malloc( bmp_info_hdr.bitmap_size ) ) ) {
        fclose( f );
        return NULL;
    }

    fread( img, 1, bmp_info_hdr.bitmap_size, f );

    fclose( f );

    return img;
}
