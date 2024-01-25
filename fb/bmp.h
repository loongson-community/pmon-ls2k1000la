typedef struct bmp_color_table_entry {
    uint8_t    blue;	// brightness in blue(range:0-255)
    uint8_t    green;	// brightness in green(range:0-255)
    uint8_t    red;	// brightness in red(range:0-255)
    uint8_t    reserved;// always be 0
} __attribute__ ((packed)) bmp_color_table_entry_t;// pallette
// The number of palette structure data is determined by bit_count: bit_count=1,4,8, there are 2,16,256 table items respectively; bit_count=24, no colour table item.
/* When accessing these fields, remember that they are stored in little
   endian format, so use linux macros, e.g. le32_to_cpu(width)          */

typedef struct bmp_header {
    /* Header 14Byte*/
    char signature[2];//always be BM
    uint32_t    file_size;// size of bitmap file
    uint32_t    reserved;// always be 0
    uint32_t    data_offset;// start of bitmap data, offset of file header to bitmap, in bytes
    /* InfoHeader 40Byte*/
    uint32_t    size;// size of this structure, in bytes
    uint32_t    width;// width of bitmap, in pixels
    uint32_t    height;// height of bitmap, in pixels
    uint16_t    planes;// level of target device, always be 1
    uint16_t    bit_count;// required number of bits per pixel, must be 1 (2 colours), 4 (16 colours), 8 (256 colours) or 24 (true colour)
    uint32_t    compression;// compression type of bitmap, must be 0 (no compression), 1 (BI_RLE8), or 2 (BI_RLE4)
    uint32_t    image_size;// size of bitmap, in bytes
    uint32_t    x_pixels_per_m;// horizontal resolution of bitmap, pixels per metre
    uint32_t    y_pixels_per_m;// vertical resolution of bitmap, pixels per metre
    uint32_t    colors_used;// number of colours that bitmap actually use
    uint32_t    colors_important;// important number of colours during bitmap display
    /* ColorTable */

} __attribute__ ((packed)) bmp_header_t;

typedef struct bmp_image {
    bmp_header_t header;
    /* We use a zero sized array just as a placeholder for variable
       sized array */
    bmp_color_table_entry_t color_table[0];
} bmp_image_t;

/* Data in the bmp_image is aligned to this length */
#define BMP_DATA_ALIGN    4

/* Constants for the compression field */
#define BMP_BI_RGB    0
#define BMP_BI_RLE8    1
#define BMP_BI_RLE4    2

# define cpu_to_le16(x)		(x)
# define cpu_to_le32(x)		(x)
# define cpu_to_le64(x)		(x)
# define le16_to_cpu(x)		(x)
# define le32_to_cpu(x)		(x)
# define le64_to_cpu(x)		(x)

