/*

  VSEARCH: a versatile open source tool for metagenomics

  Copyright (C) 2014-2015, Torbjorn Rognes, Frederic Mahe and Tomas Flouri
  All rights reserved.

  Contact: Torbjorn Rognes <torognes@ifi.uio.no>,
  Department of Informatics, University of Oslo,
  PO Box 1080 Blindern, NO-0316 Oslo, Norway

  This software is dual-licensed and available under a choice
  of one of two licenses, either under the terms of the GNU
  General Public License version 3 or the BSD 2-Clause License.


  GNU General Public License version 3

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.


  The BSD 2-Clause License

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

*/

#include "vsearch.h"

static const char * progress_prompt;
static unsigned long progress_next;
static unsigned long progress_size;
static unsigned long progress_chunk;
static const unsigned long progress_granularity = 200;

void progress_init(const char * prompt, unsigned long size)
{
  if (! opt_quiet)
    {
      progress_prompt = prompt;
      progress_size = size;
      progress_chunk = size < progress_granularity ? 
        1 : size / progress_granularity;
      progress_next = 0;
      fprintf(stderr, "%s %.0f%%", prompt, 0.0);
    }
}

void progress_update(unsigned long progress)
{
  if (! opt_quiet)
    if (progress >= progress_next)
      {
        if (progress_size > 0)
          fprintf(stderr, "  \r%s %.0f%%", progress_prompt,
                  100.0 * progress / progress_size);
        else
          fprintf(stderr, "  \r%s ?%%", progress_prompt);
        progress_next = progress + progress_chunk;
      }
}

void progress_done()
{
  if (! opt_quiet)
    fprintf(stderr, "  \r%s %.0f%%\n", progress_prompt, 100.0);
}

long gcd(long a, long b)
{
  if (b == 0)
  {
    return a;
  }
  else
  {
    return gcd(b, a % b);
  }
}

void  ATTR_NORETURN fatal(const char * msg)
{
  fprintf(stderr, "\n\n");
  fprintf(stderr, "Fatal error: %s\n", msg);

  if (opt_log)
    {
      fprintf(fp_log, "\n\n");
      fprintf(fp_log, "Fatal error: %s\n", msg);
    }

  exit(EXIT_FAILURE);
}

void  ATTR_NORETURN fatal(const char * format,
                                      const char * message)
{
  fprintf(stderr, "\n\n");
  fprintf(stderr, format, message);
  fprintf(stderr, "\n");

  if (opt_log)
    {
      fprintf(fp_log, "\n\n");
      fprintf(fp_log, format, message);
      fprintf(fp_log, "\n");
    }

  exit(EXIT_FAILURE);
}

void * xmalloc(size_t size)
{
  const size_t alignment = 16;
  void * t = 0;

#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux) || (__linux__) || (__unix__) || (__unix)
  if (posix_memalign(& t, alignment, size))
    fatal("Unable to allocate enough memory.");
#else
	t = _aligned_malloc(size, alignment);
#endif

  if (!t)
    fatal("Unable to allocate enough memory.");
  return t;
}

void * xrealloc(void *ptr, size_t size)
{
  if (size == 0)
    size = 1;
  void * t = realloc(ptr, size);
  if (!t)
    fatal("Unable to allocate enough memory.");
  return t;
}

char * xstrdup(const char *s)
{
  size_t len = strlen(s);
  char * p = (char*) xmalloc(len+1);
  return strcpy(p, s);
}


char * xstrchrnul(char *s, int c)
{
  char * r = strchr(s, c);

  if (r)
    return r;
  else
    return (char *)s + strlen(s);
}

unsigned long hash_cityhash64(char * s, unsigned long n)
{
  return CityHash64((const char*)s, n);
}

long getusec(void)
{
    struct timeval tv;
    
#if defined (_WIN32)
    static const unsigned __int64 epoch = (uint64_t)(116444736000000000);
    FILETIME file_time;
    SYSTEMTIME system_time;
    ULARGE_INTEGER ularge;
    
    GetSystemTime(&system_time);
    if ((SystemTimeToFileTime(&system_time, &file_time)) == 0) { return 0; } //failed
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;
    
    tv.tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
    tv.tv_usec = (long) (system_time.wMilliseconds * 1000);
    return tv.tv_sec;
    
    
    
#else
    if(gettimeofday(&tv,0) != 0) return 0;
    return tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}

void show_rusage()
{
#if 0
  struct rusage r_usage;
  getrusage(RUSAGE_SELF, & r_usage);
  
  fprintf(stderr, "Time: %.3fs (user)", r_usage.ru_utime.tv_sec * 1.0 + (double) r_usage.ru_utime.tv_usec * 1.0e-6);
  fprintf(stderr, " %.3fs (sys)", r_usage.ru_stime.tv_sec * 1.0 + r_usage.ru_stime.tv_usec * 1.0e-6);

  fprintf(stderr, " Memory: %luMB\n",  arch_get_memused() / 1024 / 1024);

  if (opt_log)
    {
      fprintf(fp_log, "Time: %.3fs (user)", r_usage.ru_utime.tv_sec * 1.0 + (double) r_usage.ru_utime.tv_usec * 1.0e-6);
      fprintf(fp_log, " %.3fs (sys)", r_usage.ru_stime.tv_sec * 1.0 + r_usage.ru_stime.tv_usec * 1.0e-6);
      
      fprintf(fp_log, " Memory: %luMB\n",  arch_get_memused() / 1024 / 1024);
    }
#endif
}

void reverse_complement(char * rc, char * seq, long len)
{
  /* Write the reverse complementary sequence to rc.
     The memory for rc must be long enough for the rc of the sequence
     (identical to the length of seq + 1. */

  for(long i=0; i<len; i++)
    rc[i] = chrmap_complement[(int)(seq[len-1-i])];
  rc[len] = 0;
}

void random_init()
{
  /* initialize pseudo-random number generator */
  unsigned int seed = opt_randseed;
  if (seed == 0)
    {
		char buffer[sizeof(seed)];
		std::ifstream in;
		in.open("/dev/urandom", std::ios_base::in);
      if (!in)
        fatal("Unable to open /dev/urandom");
	  in.read(buffer, sizeof(seed));
      if (in.gcount() < 0)
        fatal("Unable to read from /dev/urandom");
	  in.close();
    }
#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux) || (__linux__) || (__unix__) || (__unix)
    srandom(seed);
#else
    srand(seed);
#endif
}

long random_int(long n)
{
  /*
    Generate a random integer in the range 0 to n-1, inclusive.
    n must be > 0
    The random() function returns a random number in the range
    0 to 2147483647 (=2^31-1=RAND_MAX), inclusive.
    We should avoid some of the upper generated numbers to
    avoid modulo bias.
  */

  long random_max = RAND_MAX;
  long limit = random_max - (random_max + 1) % n;
  long r = 1;
#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux) || (__linux__) || (__unix__) || (__unix)
    r = random();
    while (r > limit) {  r = random();  }
#else
  	r = rand();
    while (r > limit) {  r = rand();  }
#endif
  return r % n;
}

unsigned long random_ulong(unsigned long n)
{
  /*
    Generate a random integer in the range 0 to n-1, inclusive,
    n must be > 0
  */
    
    
    unsigned long random_max = ULONG_MAX;
    unsigned long limit = random_max - (random_max - n + 1) % n;
    unsigned long r_long = 1;
    
#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux) || (__linux__) || (__unix__) || (__unix)

  unsigned long r = ((random() << 48) ^ (random() << 32) ^
                     (random() << 16) ^ (random()));
  while (r > limit)
    r = ((random() << 48) ^ (random() << 32) ^
         (random() << 16) ^ (random()));
    
    r_long = r;
#else
    
  long long r = ((rand() << 48) ^ (rand() << 32) ^
                     (rand() << 16) ^ (rand()));
  while (r > limit)
    r = ((rand() << 48) ^ (rand() << 32) ^
         (rand() << 16) ^ (rand()));
    
    
    r_long = (unsigned long) r;
    
#endif
    
  return r_long % n;
}

void string_normalize(char * normalized, char * s, unsigned int len)
{
  /* convert string to upper case and replace U by T */
  char * p = s;
  char * q = normalized;
  for(unsigned int i=0; i<len; i++)
    *q++ = chrmap_normalize[(int)(*p++)];
  *q = 0;
}

void fprint_hex(FILE * fp, unsigned char * data, int len)
{
  for(int i=0; i<len; i++)
    fprintf(fp, "%02x", data[i]);
}

void SHA1(const unsigned char * d, unsigned long n, unsigned char * md)
{
  if (!md)
    fatal("Error in computing SHA1 digest");
  SHA1_CTX c;
  SHA1_Init(&c);
  SHA1_Update(&c, d, n);
  SHA1_Final(&c, md);
}

void MD5(void * d, unsigned long n, unsigned char * md)
{
  if (!md)
    fatal("Error in computing MD5 digest");
  MD5_CTX c;
  MD5_Init(&c);
  MD5_Update(&c, d, n);
  MD5_Final(md, &c);
}

static const char hexdigits[] = "0123456789abcdef";

void get_hex_seq_digest_sha1(char * hex, char * seq, int seqlen)
{
  /* Save hexadecimal representation of the SHA1 hash of the sequence.
     The string array digest must be large enough (LEN_HEX_DIG_SHA1).
     First normalize string by uppercasing it and replacing U's with T's. */

  char * normalized = (char*) xmalloc(seqlen+1);
  string_normalize(normalized, seq, seqlen);

  unsigned char digest[LEN_DIG_SHA1];

  SHA1((const unsigned char*) normalized, (size_t) seqlen, digest);

  free(normalized);

  for(int i=0; i<LEN_DIG_SHA1; i++)
    {
      hex[2*i+0] = hexdigits[digest[i] >> 4];
      hex[2*i+1] = hexdigits[digest[i] & 15];
    }
  hex[2*LEN_DIG_SHA1] = 0;
}

void get_hex_seq_digest_md5(char * hex, char * seq, int seqlen)
{
  /* Save hexadecimal representation of the MD5 hash of the sequence.
     The string array digest must be large enough (LEN_HEX_DIG_MD5).
     First normalize string by uppercasing it and replacing U's with T's. */

  char * normalized = (char*) xmalloc(seqlen+1);
  string_normalize(normalized, seq, seqlen);

  unsigned char digest[LEN_DIG_MD5];

  MD5(normalized, (size_t) seqlen, digest);

  free(normalized);

  for(int i=0; i<LEN_DIG_MD5; i++)
    {
      hex[2*i+0] = hexdigits[digest[i] >> 4];
      hex[2*i+1] = hexdigits[digest[i] & 15];
    }
  hex[2*LEN_DIG_MD5] = 0;
}


void fprint_seq_digest_sha1(FILE * fp, char * seq, int seqlen)
{
  char digest[LEN_HEX_DIG_SHA1];
  get_hex_seq_digest_sha1(digest, seq, seqlen);
  fprintf(fp, "%s", digest);
}

void fprint_seq_digest_md5(FILE * fp, char * seq, int seqlen)
{
  char digest[LEN_HEX_DIG_MD5];
  get_hex_seq_digest_md5(digest, seq, seqlen);
  fprintf(fp, "%s", digest);
}


