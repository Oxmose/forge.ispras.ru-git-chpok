/*******************************************************************************
 * ARINC-653 RTOS Benchmark
 *
 * Developped/Ported by:
 *    Felipe Gohring de Magalhaes
 *    Jean-Baptiste Lefoul
 *    Alexy Torres Aurora Dugo
 *
 *
 * Test: Data compression program.
 *     Adopted from SPEC95 for WCET-calculation. Only compression is done on a buffer (small one) containing totally random data.
 *
 * Ported from:
 *     Mälardalen WCET Benchmarks (http://www.mrtc.mdh.se/projects/wcet/benchmarks.html)
 *
 ******************************************************************************/

#include "../../../BenchmarksTools/benc_config.h"

/*******************************************************************************
 * TESTS SETTINGS
 ******************************************************************************/

/*******************************************************************************
 * PARTITION SPECIFIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * TESTS FUNCTIONS
 ******************************************************************************/

/* MDH WCET BENCHMARK SUITE. File version $Id: compress.c 7800 2018-01-19 12:43:12Z lkg02 $ */

/*
 * Compress - data compression program
 *
 * Adopted from SPEC95 for WCET-calculation by Thomas Lundqvist, 1997-11-28.
 * Only compression is done on a buffer (small one) containing
 * totally random data. This should come closer to a worst case
 * compared to the original SPEC95-version.
 *
 * All unused code removed by Jakob Engblom, february 2000.  Cleaned
 * up for IAR compilation.
 *
 * Removed the prototype declaration of "code_int getcode();" that is
 * niether defined nor used. Christer Sandberg
 *
 * Changes:
 * JG 2005/12/20: Changed declaration of maxmaxcode to avoid warning
 * JG 2012/09/28: Comment within comment removed
 */ 

/* #define DO_TRACING */

#ifdef DO_TRACING   /* ON PC */

#include <stdio.h>
#define TRACE(x) trace((x))
#undef TEST                   /* finished testing! */

/*
void trace(char *s)
{
    printf("%s\n",s);
}
*/

#else               /* ON TARGET */

#define TRACE(x)
#undef TEST

#endif


#define BUFFERSIZE  20480
#define IN_COUNT    BUFFERSIZE

#define HSIZE   257     /* 95% occupancy */
#define BITS 16
#define INIT_BITS 9         /* initial number of bits/code */



/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

#define min(a,b)    ((a>b) ? b : a)

/*
 * Set USERMEM to the maximum amount of physical user memory available
 * in bytes.  USERMEM is used to determine the maximum BITS that can be used
 * for compression.
 *
 * SACREDMEM is the amount of physical memory saved for others; compress
 * will hog the rest.
 */
/* For SPEC95 use, SACREDMEM automatically set to 0.
    Jeff Reilly, 1/15/95                */

#define SACREDMEM   0

/* For SPEC95 use, USERMEM automatically set to 450000.
    Jeff Reilly, 1/15/95                */
# define USERMEM    450000  /* default user memory */

#ifdef interdata        /* (Perkin-Elmer) */
#define SIGNED_COMPARE_SLOW /* signed compare is slower than unsigned */
#endif

/* For SPEC95 use, PBITS and BITS automatically set to 16.
    Jeff Reilyy, 1/15/95                */
#define PBITS   16
#define BITS 16
#define HSIZE   257     /* 95% occupancy was 69001 */


/*
 * a code_int must be able to hold 2**BITS values of type int, and also -1
 */
#if BITS > 15
typedef long int    code_int;
#else
typedef int     code_int;
#endif

#ifdef SIGNED_COMPARE_SLOW
typedef unsigned long int count_int;
typedef unsigned short int count_short;
#else
typedef long int      count_int;
#endif

typedef unsigned char   char_type;

/* Defines for third byte of header */
#define BIT_MASK    0x1f
#define BLOCK_MASK  0x80
/* Masks 0x40 and 0x20 are free.  I think 0x20 should mean that there is
   a fourth header byte (for expansion).
*/

/* Global variables */
int n_bits;             /* number of bits/code */
int maxbits = BITS;         /* user settable max # bits/code */
code_int maxcode;           /* maximum code, given n_bits */
#if BITS > 15
code_int maxmaxcode = 1L << BITS;   /* should NEVER generate this code */
#else
code_int maxmaxcode = 1 << BITS;    /* should NEVER generate this code */
#endif

# define MAXCODE(n_bits)    ((1 << (n_bits)) - 1)


#define htabof(i)   htab[i]
#define codetabof(i)    codetab[i]

code_int hsize = HSIZE;         /* for dynamic table sizing */
count_int fsize;

/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**BITS characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i) codetabof(i)
#define tab_suffixof(i) ((char_type *)(htab))[i]
#define de_stack        ((char_type *)&tab_suffixof(1<<BITS))

code_int free_ent = 0;          /* first unused entry */
int exit_stat = 0;

int nomagic = 1;    /* Use a 3-byte magic number header, unless old file */
int zcat_flg = 0;   /* Write output on stdout, suppress messages */
int quiet = 1;      /* don't tell me about compression */

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
int block_compress = BLOCK_MASK;
int clear_flg = 0;
long int ratio = 0;
#define CHECK_GAP 10000 /* ratio check interval */
count_int checkpoint = CHECK_GAP;
/*
 * the next two codes should not be changed lightly, as they must not
 * lie within the contiguous general code space.
 */
#define FIRST   257 /* first free entry */
#define CLEAR   256 /* table clear output code */

int force = 0;
char ofname [100];
int InCnt;
int apsim_InCnt;
unsigned char *InBuff;
unsigned char *OutBuff;

char orig_text_buffer[BUFFERSIZE];
char comp_text_buffer[BUFFERSIZE+5];

count_int htab [HSIZE];
unsigned short codetab [HSIZE];
char buf[BITS];



/*---------------------------------------------------- */
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/


/*----------------------------------------- Prototypes */
void initbuffer(int* seed);
void compress(void);
void cl_hash(count_int hsize);      /* reset code table */
unsigned int getbyte(void);
void putbyte( char c );
void cl_block (void);
void output( code_int code );
void writebytes( char *buf, int n );

void* compress_thread(void)
{
    RETURN_CODE_TYPE      ret_type;
    PARTITION_STATUS_TYPE pr_stat;

    GET_PARTITION_STATUS(&pr_stat, &ret_type);
    if(ret_type != NO_ERROR)
    {
        OUTPUT("[COMPRESS] Cannot get partition status [%d]\n", ret_type);
        return (void*)1;
    }

    while(1)
    {
        OUTPUT("Compressing text of %u elements\n", IN_COUNT);


        compress();

        PERIODIC_WAIT(&ret_type);
        if(ret_type != NO_ERROR)
        {
            OUTPUT("[COMPRESS] Cannot achieve periodic wait [%d]\n", ret_type);
            return (void*)1;
        }
    }

    return (void*)0;
}



void initbuffer(int* seed)
{
   int i;

   for (i = 0 ; i < BUFFERSIZE ; i++) {
      /* Generates random integers between 0 and 8095 */
      *seed = ((*seed * 133) + 81) % 8095;

      orig_text_buffer[i] = (*seed % 94) + 33;
   }
}



static int offset;
long int in_count = 1;          /* length of input */
long int bytes_out;         /* length of compressed output */
long int out_count = 0;         /* # of codes output (for debugging) */


void compress(void)
{
   register long fcode;
   register code_int i = 0;
   register int c;
   register code_int ent;
   register int disp;
   register code_int hsize_reg;
   register int hshift;


   offset = 0;
   bytes_out = 3;       /* includes 3-byte header mojo */
   out_count = 0;
   clear_flg = 0;
   ratio = 0;
   in_count = 1;
   checkpoint = CHECK_GAP;
   maxcode = MAXCODE(n_bits = INIT_BITS);
   free_ent = ((block_compress) ? (FIRST) : (256) );

   ent = getbyte ();

   hshift = 0;
   for ( fcode = (long) hsize;  fcode < 65536L; fcode *= 2L )
   {
      hshift++;
   }

   hshift = 8 - hshift;     /* set hash code range bound */

   hsize_reg = hsize;
   cl_hash( (count_int) hsize_reg);     /* clear hash table */


   while ( InCnt > 0 )           /* apsim_loop 11 0 */
   {
      int apsim_bound111 = 0;

      c = getbyte();   /* decrements InCnt */

      in_count++;
      fcode = (long) (((long) c << maxbits) + ent);
      i = ((c << hshift) ^ ent);    /* xor hashing */

      if ( htabof (i) == fcode ) {
         ent = codetabof (i);
         continue;
      } else if ( (long)htabof (i) < 0 ) { /* empty slot */
         goto nomatch;
      }


      disp = hsize_reg - i;     /* secondary hash (after G. Knott) */
      if ( i == 0 ) {
         disp = 1;
      }

probe:

      if ( (i -= disp) < 0 ) {  /* apsim_loop 111 11 */
         i += hsize_reg;
      }

      if ( htabof (i) == fcode ) {
         ent = codetabof (i);
         continue;
      }

      if ( (long)htabof (i) > 0 && (++apsim_bound111 < in_count) )
         goto probe;
nomatch:

      out_count++;
      ent = c;
      if ( free_ent < maxmaxcode ) {
         codetabof (i) = free_ent++;            /* apsim_unknown codetab */
         htabof (i) = fcode;            /* apsim_unknown htab */
      } else if ( ((count_int)in_count >= checkpoint) && (block_compress) ) {
         cl_block ();
      }

   }
   if(bytes_out > in_count) { /* exit(2) if no savings */
      exit_stat = 2;
   }

   return;
}


void cl_block (void)        /* table clear for block compress */
{
   register long int rat;

   checkpoint = in_count + CHECK_GAP;

   if(in_count > 0x007fffff) {  /* shift will overflow */

      rat = bytes_out >> 8;
      if(rat == 0) {        /* Don't divide by zero */
         rat = 0x7fffffff;
      } else {
         rat = in_count / rat;
      }
   } else {
      rat = (in_count << 8) / bytes_out;    /* 8 fractional bits */
   }
   if ( rat > ratio ) {
      ratio = rat;
   } else {
      ratio = 0;
      cl_hash ( (count_int) hsize );


      free_ent = FIRST;
      clear_flg = 1;
      output ( (code_int) CLEAR );
   }
}

void cl_hash(count_int hsize)       /* reset code table */
{
   register count_int *htab_p = htab+hsize;
   register long i;
   register long m1 = -1;

   i = hsize - 16;
   do {             /* might use Sys V memset(3) here */

      *(htab_p-16) = m1;
      *(htab_p-15) = m1;
      *(htab_p-14) = m1;
      *(htab_p-13) = m1;
      *(htab_p-12) = m1;
      *(htab_p-11) = m1;
      *(htab_p-10) = m1;
      *(htab_p-9) = m1;
      *(htab_p-8) = m1;
      *(htab_p-7) = m1;
      *(htab_p-6) = m1;
      *(htab_p-5) = m1;
      *(htab_p-4) = m1;
      *(htab_p-3) = m1;
      *(htab_p-2) = m1;
      *(htab_p-1) = m1;
      htab_p -= 16;
   } while ((i -= 16) >= 0);
   for ( i += 16; i > 0; i-- ) {
      *--htab_p = m1;
   }
}



unsigned int getbyte(void)
{
   if( InCnt > 0 && (apsim_InCnt-- > 0)) {
      InCnt--;
      return( (unsigned int)*InBuff++ );
   } else {
      return( -1 );
   }
}

void putbyte( char c )
{
   *OutBuff++ = c;               /* apsim_unknown comp_text_buffer */
}


void writebytes( char *buf, int n )
{
   int i;
   for( i=0; (i<n) && /*apsim*/ (i < BITS) ; i++ ) {
      *OutBuff++ = buf[i];               /* apsim_unknown comp_text_buffer */
   }
}

/* apsim_rel 111 < 112 */

char_type lmask[9] = {0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};
char_type rmask[9] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

void output( code_int code )
{
   /*
   * On the VAX, it is important to have the register declarations
   * in exactly the order given, or the asm will break.
   */
   register int r_off = offset, bits= n_bits;
   register char * bp = buf;

   if ( code >= 0 ) {
      /*
      * byte/bit numbering on the VAX is simulated by the following code
      */
      /*
      * Get to the first byte.
      */
      bp += (r_off >> 3);
      r_off &= 7;
      /*
      * Since code is always >= 8 bits, only need to mask the first
      * hunk on the left.
      */
      *bp = ((*bp & rmask[r_off]) | (code << r_off)) & lmask[r_off];  /* apsim_unknown buf */
      bp++;
      bits -= (8 - r_off);
      code >>= 8 - r_off;
      /* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
      if ( bits >= 8 ) {

         *bp++ = code;  /* apsim_unknown buf */
         code >>= 8;
         bits -= 8;
      }

      /* Last bits. */
      if(bits) {
         *bp = code;             /* apsim_unknown buf */
      }

      offset += n_bits;
      if ( offset == (n_bits << 3) ) {
         bp = buf;
         bits = n_bits;
         bytes_out += bits;
         do {
            putbyte(*bp++);
         } while(( --bits) && ((bp - buf < BITS)));
         offset = 0;
      }
      /*
      * If the next entry is going to be too big for the code size,
      * then increase it, if possible.
      */
      if ( free_ent > maxcode || ((clear_flg > 0))) {
         /*
         * Write the whole buffer, because the input side won't
         * discover the size increase until after it has read it.
         */
         if ( offset > 0 ) {
            writebytes( buf, n_bits );
            bytes_out += n_bits;
         }
         offset = 0;
         if ( clear_flg ) {
            maxcode = MAXCODE (n_bits = INIT_BITS);
            clear_flg = 0;
         } else {
            n_bits++;
            if ( n_bits == maxbits )
            {
               maxcode = maxmaxcode;
            }
            else
            {
               maxcode = MAXCODE(n_bits);
            }
         }
      }
   } else {
      /*
      * At EOF, write the rest of the buffer.
      */
      if ( offset > 0 )
      {
         writebytes( buf, ((offset + 7) / 8) );
      }
      bytes_out += (offset + 7) / 8;
      offset = 0;
   }
}

/*******************************************************************************
 * TESTS MAIN
 ******************************************************************************/
int main()
{
    PROCESS_ID_TYPE        thread_compress;
    PROCESS_ATTRIBUTE_TYPE th_attr_compress;

    RETURN_CODE_TYPE       ret_type;

    /* Used to initialize input text with random values*/
    int seed = 1;

    /* Set COMPRESS manipulation process */
    th_attr_compress.ENTRY_POINT   = compress_thread;
    th_attr_compress.DEADLINE      = HARD;
    th_attr_compress.PERIOD        = 10000000000;
    th_attr_compress.STACK_SIZE    = 4096;
    th_attr_compress.TIME_CAPACITY = 10000000000;
    th_attr_compress.BASE_PRIORITY = 1;
    memcpy(th_attr_compress.NAME, "COMPRESS_A653\0", 14 * sizeof(char));

    OUTPUT("Init P0 partition\n");

    int count = IN_COUNT;

      initbuffer(&seed);

      /*if(maxbits < INIT_BITS) maxbits = INIT_BITS;*/
      /* With our setting, maxbits = 16,
                           INIT_BITS = 9 */
      /*if (maxbits > BITS) maxbits = BITS;*/
      maxbits = BITS;
      maxmaxcode = 1 << maxbits;

      InCnt = count;
      apsim_InCnt = IN_COUNT + 3;
      InBuff = (unsigned char *)orig_text_buffer;

      OutBuff = (unsigned char *)comp_text_buffer;

    /* Create processes */
    CREATE_PROCESS(&th_attr_compress, &thread_compress, &ret_type);
    if(ret_type != NO_ERROR)
    {
        OUTPUT("Cannot create COMPRESS process [%d]\n", ret_type);
        return -1;
    }

    /* Start all processes */
    START(thread_compress, &ret_type);
    if(ret_type != NO_ERROR)
    {
        OUTPUT("Cannot start COMPRESS process[%d]\n", ret_type);
        return -1;
    }

    /* Partition has been initialized, now switch to normal mode */
    OUTPUT("P0 partition switching to normal mode\n");
    SET_PARTITION_MODE(NORMAL, &ret_type);
    if(ret_type != NO_ERROR)
    {
        OUTPUT("Cannot switch P0 partition to NORMAL state[%d]\n", ret_type);
        return -1;
    }

    STOP_SELF();

    return 0;
}
