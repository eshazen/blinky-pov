//
// extract a useful subset of ASCII characters from the 6x9 font
// (which is really a 6x8 font, top row is unused)
// output as 6-byte hex data suitable for use in javascript
//

#define GLYPH_LO 32
#define GLYPH_HI 96

#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <ftbitmap.h>

static FT_Library library;
static FT_Face face;
static FT_Error err;

int main(int argc, char **argv) {
  char *filename;
  char *outfile;
  FILE *fo;

  if (argc < 3) {
    fprintf( stderr, "usage: font2c [font] [output_file]\n");
    exit(1);
  }
  filename = argv[1];
  outfile = argv[2];

  if ((err = FT_Init_FreeType( &library ))) {
    fprintf( stderr, "error: Init_Freetype failed %d\n", err);
    exit(1);
  }
  if ((err = FT_New_Face( library, filename, 0, &face ))) {
    fprintf( stderr, "error: FT_New_Face failed %d\n", err);
    exit(1);
  }

  if( (fo = fopen( outfile, "w")) == NULL) {
    fprintf( stderr, "error:  opening output file %s\n", outfile);
    exit(1);
  }

  printf("Number of glyphs: %ld\n", face->num_glyphs);
  FT_UInt  glyph_index;
  FT_GlyphSlot  slot;

  fprintf( fo, "var font_lo = %d;\n", GLYPH_LO);
  fprintf( fo, "var font_hi = %d;\n", GLYPH_HI);
  fprintf( fo, "var font = [\n");

  for( unsigned char ch=GLYPH_LO; ch<=GLYPH_HI; ch++) {
    printf("--- Rendering ASCII code %d (0x%x) '%c'\n", ch, ch, ch);

    glyph_index = FT_Get_Char_Index( face, ch );

    if ((err = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT ))) {
      fprintf( stderr, "warning: failed FT_Load_Glyph 0x%x %d\n", ch, err);
      exit(1);
    }

    if ((err = FT_Render_Glyph( face->glyph, FT_RENDER_MODE_MONO ))) {
      fprintf( stderr, "warning: failed FT_Render_Glyph 0x%x %d\n", ch, err);
      exit(1);
    }

    slot = face->glyph;
    printf("rows: %d width: %d\n", slot->bitmap.rows, slot->bitmap.width);
    printf("pixel_mode: %s\n", slot->bitmap.pixel_mode == ft_pixel_mode_mono ? "mono" : "what?");
    printf("pitch: %d\n", slot->bitmap.pitch);

    int width = slot->bitmap.width;
    int pitch = slot->bitmap.pitch;

    unsigned char buff[40];
    memset( buff, 0, sizeof(buff));

    for( int row=0; row<slot->bitmap.rows; row++) {
      printf("row %02d: ", row);
      for( int byte=0; byte<pitch; byte++) {
	unsigned char d = slot->bitmap.buffer[row*pitch+byte];
	for( int k=0, b=0x80; k<8; k++, b>>=1) {
	  if( d & b) {
	    putchar('*');
	    buff[k] |= (0x80>>row);
	  } else {
	    putchar(' ');
	  }
	}
      }
      printf("\n");
    }

    // determine actual width of this character
    int i0 = 0;
    int i1 = width-1;
    while( buff[i0] == 0 && i0 < width)
      ++i0;
    while( buff[i1] == 0 && i1 > 0)
      --i1;

    int wid = i1-i0+1;
    if( wid < 0)
      wid = 0;
    printf("Width = %d (%d, %d)\n", wid, i0, i1);

    printf("Output data: ");
    fprintf( fo, "  %d,", wid);
    if( wid) {
      for( int i=i0; i<=i1; i++) {
	printf(" %02x", buff[i]);
	fprintf( fo, "0x%02x", buff[i]);
	if( i != i1 || ch != GLYPH_HI)
	  fprintf( fo, ",");
      }
      printf("\n");
      fprintf( fo, "  // char: %c\n", ch);
    } else {
      fprintf( fo, "\n");
    }

  }
    
  fprintf( fo, "];\n");
  fclose( fo);

  FT_Done_Face( face );
  FT_Done_FreeType( library );

  return 0;

}
