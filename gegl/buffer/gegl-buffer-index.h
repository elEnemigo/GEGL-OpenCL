#ifndef __GEGL_BUFFER_INDEX_H
#define __GEGL_BUFFER_INDEX_H
/* File format building blocks 

GeglBuffer on disk representation
=================================

*/


/* Increase this number when the structures change.*/
#define GEGL_FILE_SPEC_REV     23
#define GEGL_MAGIC             {'G','E','G','L'}

#define GEGL_FLAG_TILE         1
#define GEGL_FLAG_FREE_TILE    0xf+2

/* a VOID message, indicating that the specified tile has been rewritten */
#define GEGL_FLAG_INVALIDATED  2

/* these flags are used for the header, the lower bits of the
 * header store the revision
 */
#define GEGL_FLAG_LOCKED       (0xff*(1<<0))
#define GEGL_FLAG_FLUSHED      (0xff*(1<<1))
#define GEGL_FLAG_IS_HEADER    (0xff*(1<<3))

/* The default header we expect to see on a file is that it is
 * flushed, and has the revision the file conforms to written
 * to it.
 */
#define  GEGL_FLAG_HEADER    (GEGL_FLAG_FLUSHED  |\
                              GEGL_FLAG_IS_HEADER|\
                              GEGL_FILE_SPEC_REV)

/*
 * This header is the first 256 bytes of the GEGL buffer.
 */
typedef struct {
  gchar   magic[4];    /* - a 4 byte identifier */
  guint32 flags;       /* the header flags is used to encode state and revision 
                          */
  guint64 next;        /* offset to first GeglBufferBlock */

  guint32 tile_width;       
  guint32 tile_height;
  guint16 bytes_per_pixel;

  gchar   description[64]; /* GEGL stores the string of the babl format
                            * here, as well as after the \0 a debug string
                            * describing the buffer.
                            */

  /* the ROI could come as a separate block */
  gint32  x;               /* indication of bounding box for tiles stored. */
  gint32  y;               /* this isn't really needed as each GeglBuffer as */
  guint32 width;           /* represented on disk doesn't really have any */
  guint32 height;          /* dimension restriction. */

  guint32 rev;             /* if it changes on disk it means the index has changed */

  gint32  padding[36];     /* Pad the structure to be 256 bytes long */
} GeglBufferHeader;

/* the revision of the format is stored in the flags of the header in the
 * lower 8 bits
 */
#define gegl_buffer_header_get_rev(header)  (((GeglBufferHeader*)(header))->flags&0xff)

/* The GeglBuffer index is written to the file as a linked list of
 * GeglBufferBlock's, each block encodes it's own length and the offset
 * of the file which the next block can be found. The last block in the
 * list has the next offset set to 0.
 */

typedef struct {
  guint32  length; /* the length of this block */
  guint32  flags;  /* flags (can be used to handle different block types
                    * differently
                    */
  guint64  next;   /*next block element in file */
} GeglBufferBlock;

/* The header is followed by entries describing tiles stored in the swap,
 */
typedef struct {
  GeglBufferBlock block; /* The block definition for this tile entry   */
  guint64 offset;        /* offset into file for this tile             */
  gint32  x;             /* upperleft of tile % tile_width coordinates */
  gint32  y;           

  gint32  z;             /* mipmap subdivision level of tile (0=100%)  */

  /** used for shared buffers can be ignored for normal use */
  guint32 rev;           /* revision */
} GeglBufferTile;

/* The following structs are sketches for collaborative use of the b

typedef struct {
  GeglBufferBlock block; /* The block definition for this tile entry   */
  guint32 rev;           /* buffer rev this change belongs to */
  gint32  x;             /* upperleft of tile % tile_width coordinates */
  gint32  y;           

  gint32  z;             /* mipmap subdivision level of tile (0=100%)  */
} GeglInvalidated;

#define MAX_CLIENTS

typedef struct {
  GeglBufferBlock block; /* The block definition for this tile entry   */
  gchar    lock;         /* mmap like lock, we poll and sync to get it  */
} GeglMaster;

typedef struct {
  GeglBufferBlock block; /* The block definition for this tile entry   */
  guchar  id[64];        /* a string identifying the client */
  guint32  timestamp; 
  guint64  invalidated;  /* local storage for the client to build it's
                          * invalidated list
                          */
  guint32  startreserved;
  guint32  endreserved;
} GeglClient;

/* A convenience union to allow quick and simple casting */
typedef union {
  guint32          length;
  GeglBufferBlock  block;
  GeglBufferHeader header;
  GeglBufferTile   tile;
} GeglBufferItem;

/* functions to initialize data structures */
GeglBufferTile * gegl_tile_entry_new (gint x,
                                      gint y,
                                      gint z);

/* intializing the header causes the format to be written out
 * as well as a hidden comment after the zero terminated format
 * with additional human readable information about the header.
 */
void gegl_buffer_header_init (GeglBufferHeader *header,
                              gint              tile_width,
                              gint              tile_height,
                              gint              bpp,
                              Babl*             format);

void gegl_tile_entry_destroy (GeglBufferTile *entry);

GeglBufferItem *gegl_buffer_read_header(GInputStream *i,
                                        goffset      *offset);
GList          *gegl_buffer_read_index (GInputStream *i,
                                        goffset      *offset);

#define struct_check_padding(type, size) \
  if (sizeof (type) != size) \
    {\
      g_warning ("%s %s is %i bytes, should be %i padding is off by: %i bytes %i ints", G_STRFUNC, #type,\
         (int) sizeof (type),\
         size,\
         (int) sizeof (type) - size,\
         (int)(sizeof (type) - size) / 4);\
      return;\
    }
#define GEGL_BUFFER_STRUCT_CHECK_PADDING \
  {struct_check_padding (GeglBufferBlock, 16);\
  struct_check_padding (GeglBufferHeader,   256);}
#define GEGL_BUFFER_SANITY {static gboolean done=FALSE;if(!done){GEGL_BUFFER_STRUCT_CHECK_PADDING;done=TRUE;}}

#endif
