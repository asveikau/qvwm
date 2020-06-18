/* 

Take an animated image .ani file, as used in qvwm, on input and produce the
first frame of it as another file.

2006-03-30 09:47 Ivan Kurmanov

*/

//
// animation file format
//   'ANIM' <# of images(4B)>                       ; header
//   'DATA' <image data size(4B)> <interval(4B)>
//          <image name length(4B)> <image name(4B align)>
//          <data(4B align)>                        ; image data
//     or
//   'REFE' <frame number(4B)> <interval(4B)>       ; reference(backward only)
//
//   All value is big endian.
//   <interval> is the time during which the image is displayed.
//


#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


char *inputfile = NULL;
char *imagedata  = NULL;
char *outfile   = NULL;
char *imagefilename = NULL;
int imagesize;


void print_usage (void);
int read_input( char *file );


void print_usage (void) {

  printf( "deanimate an .ani image file\n\
\n\
Usage: deanimate file.ani [outfile]\n\
" );


}
/*
 * error -- Display an error message.
 */
void error(const char* fmt, ...)
{
  va_list args;
  char buf[256];
  
  va_start(args, fmt);
  
  sprintf(buf, "deanimate: %s\n", fmt);
  vfprintf(stderr, buf, args);

  va_end(args);
}


int GetBWord(FILE* fp, int* word) {
  unsigned char data[4];

  if (fread(data, 4, 1, fp) < 1)
    return -1;

  *word = (int)(data[3] + ((int)data[2] << 8) + ((int)data[1] << 16)
		+ ((int)data[0] << 24));
  
  return 0;
}

//
// animation file format
//   'ANIM' <# of images(4B)>                       ; header
//   'DATA' <image data size(4B)> <interval(4B)>
//          <image name length(4B)> <image name(4B align)>
//          <data(4B align)>                        ; image data
//     or
//   'REFE' <frame number(4B)> <interval(4B)>       ; reference(backward only)
//
//   All value is big endian.
//   <interval> is the time during which the image is displayed.
//

int parse_header(FILE* fp, int* num, char* filename)
{
  char id[4];

  // read and check animation file id
  if (fread(id, 4, 1, fp) < 1) {
    error("cannot read animation file: '%s'", filename);
    return -1;
  }
  if (memcmp(id, "ANIM", 4) != 0) {
    error("'%s' is not animation file", filename);
    return -1;
  }
    
  // read # of images
  if (GetBWord(fp, num) == -1) {
    error("cannot read animation image file: '%s'", filename);
    return -1;
  }
  
  return 0;
}

/* 
 FILE *fp     -- file descriptor to read
 char **data  -- the image data of the frame (pointer to pointer)
 int  *len    -- size of the image data that was read and that contains the frame (pointer to)
 char **name  -- image (frame) file name
 int  *ival   -- image (frame) display interval (pointer)
 int filesize -- size of the file fp
 char *filename -- name of the file (for error reporting)
*/
int read_image_data(FILE* fp, 
                    char** data, int* len, char **name,
                    int* ival,
                    int filesize, char* filename)
{
  char id[4];
  char pad[3];
  int namlen;

  *name = NULL;
  *data = NULL;

  // read and check the start of image section
  if (fread(id, 4, 1, fp) < 1) {
    error("cannot read image section: '%s'", filename);
    return -1;
  }

  if (memcmp(id, "DATA", 4) == 0) {
    // read image data length
    if (GetBWord(fp, len) == -1) {
      error("cannot read contained image length: '%s'", filename);
      return -1;
    }
    if (*len < 0) {
      error("contained image length is negative: '%s'", filename);
      return -1;
    }

    // read image display interval
    if (GetBWord(fp, ival) == -1) {
      error("cannot read interval: '%s'", filename);
      return -1;
    }
    if (*ival < 0) {
      error("interval is negative: '%s'", filename);
      return -1;
    }

    // read contained image name length
    if (GetBWord(fp, &namlen) == -1) {
      error("cannot read contained image name length: '%s'", filename);
      return -1;
    }
    if (namlen > 0) {
      if (ftell(fp) + namlen > filesize) {
	error("contained image name length is too large: '%s'", filename);
	return -1;
      }

      *name = malloc( namlen + 1 );
      assert( *name );

      if ((namlen & 3) != 0)
	namlen = (namlen + 3) / 4 * 4;
      
      // name is not used in qvwm now...
      if (fread(*name, namlen, 1, fp) < 1) {
	error("cannot read contained image name: '%s'", filename);
	return -1;
      }
    }
    else if (namlen < 0) {
      error("contained image name length is negative: '%s'", filename);
      return -1;
    }

    if (ftell(fp) + *len > filesize) {
      error("contained image length is too large: '%s'", filename);
      return -1;
    }

    *data = malloc( (*len)+1 ) ;
    assert( *data );

    if (fread(*data, *len, 1, fp) < 1) {
      error("cannot read contained image data: '%s'", filename);
      return -1;
    }

    // read until 4 byte alignment
    if ((*len & 3) != 0)
      fread(pad, 4 - (*len & 3), 1, fp);

  }
  else if (memcmp(id, "REFE", 4) == 0) {

    if (GetBWord(fp, len) == -1) {
      error("cannot read contained image length: '%s'", filename);
      return -1;
    }
    if (*len < 0) {
      error("contained image length is negative: '%s'", filename);
      return -1;
    }

    if (GetBWord(fp, ival) == -1) {
      error("cannot read interval: '%s'", filename);
      return -1;
    }
    if (*ival < 0) {
      error("interval is negative: '%s'", filename);
      return -1;
    }

    *data = NULL;  // stands for frame reference

  }
  else {
    error("'%s' is not animation file", filename);
    return -1;
  }

  return 0;
}


int read_input( char *file ) {

  FILE* fp;
  int ret = 0;
  int images_total;

  printf( "reading file: %s\n", file );

  if ( fp = fopen( file, "r" ) ) {
    
    ret = parse_header( fp, &images_total, file );
    if ( ret == 0 ) {
      int filesize;
      struct stat st;
      int interval;

      // get size of file, for self-control
      if (fstat(fileno(fp), &st) < 0) {
        error("cannot access animation file: '%s'", file );
        fclose(fp);
        return;
      }
      filesize = st.st_size;


      printf( "number of frames: %d\n", images_total );
      ret = read_image_data( fp, &imagedata, &imagesize, &imagefilename, 
                             &interval,
                             filesize, file );

      if ( ret == 0 ) {
        // success 
        if ( !imagedata ) {
          error( "empty frame!" );

        } else {
          // real success :-)
          printf( "read image: %s of length: %d\n", imagefilename, imagesize );

          return 0;
        }

      } else {
        error( "read image data failed!" );
      }

    }

    fclose( fp );

  } else {
    error ( "can't open file: %s", file );
  }
  return -1;
}


int write_image_data ( char *data, int size, char *filename ) {

  //// now write it!
  FILE *o;
  o = fopen( filename, "w" );
  if ( o ) {
    fwrite( data, size, 1, o );
    fclose( o );
    printf( "written %s\n", filename );
    return 0;
    
  } else {
    error( "can't open %s for writing", filename );
    return -1;
  }
}




int main( int argc, char** argv ) {

  if (argc>1) {
    inputfile = argv[1];
  } else {
    print_usage();
  }

  if (argc>2) {
    outfile = argv[2];
  }

  if ( inputfile ) {
    if ( ! read_input( inputfile ) ) {
      char * outname;
      if ( outfile ) {
        outname = outfile;
      } else {
        outname = imagefilename;
      }
      
      return write_image_data( imagedata, imagesize, outfile );
    }

    if ( imagedata )      { free( imagedata );    }
    if ( imagefilename  ) { free( imagefilename ); }
  }

  return 0;
}




