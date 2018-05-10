#include "tools.h"
#include "bitmap.h"

color_t     color_pallete[ 256 ];
map_entry_t map[ NUM_MAP_ENTRIES ];
int         num_colors;

/*void colors_to_mem( FILE * f, unsigned long addr )
{
    unsigned int i;

    for( i = 0; i < 256; i++ ) {
        fprintf( f, "\t\t%lu =>\tx\"", addr );

        if( i < num_colors ) {
            fprintf( f, "00%.2X%.2X%.2X", color_pallete[ i ].b, color_pallete[ i ].g, color_pallete[ i ].r );
        } else {
            fprintf( f, "00000000" );
        }

        if( i < num_colors ) {
            fprintf( f, "\", -- R: %d G: %d B: %d\n", color_pallete[ i ].r, color_pallete[ i ].g, color_pallete[ i ].b );
        } else {
            fprintf( f, "\", -- Unused\n" );
        }

        addr++;
    }
}

char * color_to_string( unsigned char r, unsigned char g, unsigned char b )
{
    static char     str[ 3 ];
    unsigned int    i;
    unsigned int    mask;
    unsigned char   bit;
    unsigned char   found;

    memset( str, '?', 2 );

    found = 0;

    for( i = 0; i < num_colors; i++ ) {
        if( color_pallete[ i ].r == r && color_pallete[ i ].g == g && color_pallete[ i ].b == b ) {
            sprintf( str, "%.2X", i );
            found = 1;
            break;
        }
    }

    // Color is not in pallete but there's still free space, so add it
    if( !found && num_colors < 256 ) {
        color_pallete[ num_colors ].r = r;
        color_pallete[ num_colors ].g = g;
        color_pallete[ num_colors ].b = b;

        sprintf( str, "%.2X", num_colors );

        num_colors++;
    } else {
        if( !found ) {
            printf( "Cannot add color: %d %d %d, pallete is full!\n", r, g, b );
        }
    }

    str[ 2 ] = '\0';

    return str;
}

void image_to_mem( FILE * f, unsigned long addr, unsigned char * img, unsigned char type, char * comment )
{
    unsigned char n;
    unsigned char i;
    unsigned char k;
    unsigned char pixel;

    n = ( type == IMG_8x8 ) ? 8 : 16;

    img += ( n * n - 1 ) * 3;

    for( i = 0; i < n; i++ ) {
        img -= ( n - 1 ) * 3;

        for( k = 0; k < n / 4; k++ ) {
            fprintf( f, "\t\t%lu =>\tx\"", addr );

            // 4 color pallete indexes
            for( pixel = 0; pixel < 4; pixel++ ) {
                fprintf( f, color_to_string( img[ 2 ], img[ 1 ], img[ 0 ] ) );
                img += 3;
            }

            if( !i && !k ) {
                fprintf( f, "\", -- %s\n", comment );
            } else {
                fprintf( f, "\",\n" );
            }

            addr++;
        }

        img -= ( n + 1 ) * 3;
    }
}

void process_images( const char * dir, FILE * mem_file, FILE * def_file, unsigned long * base_addr, unsigned char type )
{
    char            search_dir[ MAX_PATH ];
    char            file_path[ MAX_PATH ];
    char            def_name[ 128 ];
    unsigned char * img;
    WIN32_FIND_DATA find_data;
    HANDLE          find;

    sprintf( search_dir, ( type == IMG_8x8 ) ? "%s\\8x8\\*.bmp" : "%s\\16x16\\*.bmp", dir );

    if( !( find = FindFirstFile( search_dir, &find_data ) ) ) {
        printf( "FindFirstFile failed.\n" );
        return;
    }

    do {
        if( !( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {
            sprintf( file_path, ( type == IMG_8x8 ) ? "%s\\8x8\\%s" : "%s\\16x16\\%s", dir, find_data.cFileName );

            if( !( img = load_bitmap( file_path ) ) ) {
                printf( "Failed to open: %s\n", file_path );
                continue;
            }

            sprintf( def_name, ( type == IMG_8x8 ) ? "IMG_8x8_%s" : "IMG_16x16_%s", find_data.cFileName );

            // Remove .bmp extension
            def_name[ strlen( def_name ) - 4 ] = '\0';

            fprintf( def_file, "#define %s\t\t\t0x%.4X\n", def_name, *base_addr );

            image_to_mem( mem_file, *base_addr, img, type, def_name );

            // Each image row gets split into 4 byte parts in order to fit memory size.
            *base_addr += ( type == IMG_8x8 ) ? 8 * 2 : 16 * 4;

            free( img );
        }
    } while( FindNextFile( find, &find_data ) );

    FindClose( find );
}

void create_test_map( )
{
    unsigned int    x;
    unsigned int    y;
    char            tmp;
    FILE *          f;

    if( !( f = fopen( "mapa.map", "r" ) ) ) {
        printf( "Couldn't open 'mapa.map' file!\n" );
        return;
    }

    x = 0;

    while( ( tmp = fgetc( f ) ) != EOF ) {
        if( tmp >= '0' && tmp <= '9' ) {
            map[ x ].rot = 0;

            if( tmp == '0' ) {
                map[ x ].z = 0;
                map[ x++ ].ptr = 0x0400; // Null object
            } else if( tmp == '1' ) {
                map[ x ].z = 0;
                map[ x++ ].ptr = 0x03A0; // Blank
            } else if( tmp == '2' ) {
                map[ x ].z = 0;
                map[ x++ ].ptr = 0x0400; // Null object
            } else if( tmp == '3' ) {
                map[ x ].z = 1;
                map[ x++ ].ptr = 0x03B0; // Brick
            } else if( tmp == '4' ) {
                map[ x ].z = 2;
                map[ x++ ].ptr = 0x03C0; // Grass
            } else if( tmp == '5' ) {
                map[ x ].z = 1;
                map[ x++ ].ptr = 0x03E0; // Iron
            } else if( tmp == '6' ) {
                map[ x ].z = 0;
                map[ x++ ].ptr = 0x0420; // Water
            } else if( tmp == '7' ) {
                map[ x ].z = 0;
                map[ x++ ].ptr = 0x03D0; // Ice
            }
        }
    }

    fclose( f );
}

void map_to_mem( FILE * mem_file, FILE * def_file, FILE * hdr_file, unsigned long * base_addr )
{
    unsigned int i;

    fprintf( def_file, "#define MAP_BASE_ADDRESS\t\t\t0x%.4X", *base_addr );

    fprintf( hdr_file, "map_entry_t map1[ 4800 ] = {\n" );

    for( i = 0; i < NUM_MAP_ENTRIES; i++ ) {
        fprintf( mem_file, "\t\t%lu =>\tx\"%.2X%.2X%.4X\", -- z: %d rot: %d ptr: %d\n", *base_addr,
                                                                                         map[ i ].z,
                                                                                         map[ i ].rot,
                                                                                         map[ i ].ptr,
                                                                                         map[ i ].z,
                                                                                         map[ i ].rot,
                                                                                         map[ i ].ptr );

        fprintf( hdr_file, ( i == NUM_MAP_ENTRIES - 1 ) ? "    { %u, %u, 0x%.4X, 0 }  // x: %u y: %u\n"
                                                        : "    { %u, %u, 0x%.4X, 0 }, // x: %u y: %u\n",
                            map[ i ].z,
                            map[ i ].rot,
                            map[ i ].ptr,
                            i % 80,
                            i / 80 );

        *base_addr += 1;
    }

    fprintf( hdr_file, "};\n" );
}*/
