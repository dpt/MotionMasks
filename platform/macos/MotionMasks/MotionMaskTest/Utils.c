//
//  Utils.c
//  MotionMasks
//
//  Created by David Thomas on 19/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <dirent.h>
#include <regex.h>

/* DPTLib */

#include "base/result.h"
#include "utils/array.h"

#include "Utils.h"

void memset32(uint32_t *dest, uint32_t val, int len)
{
  while (len--)
    *dest++ = val;
}

static int filenamecompare(const void *a, const void *b)
{
  const char *const *sa = a;
  const char *const *sb = b;

  return strcmp(*sa, *sb);
}

result_t findfilesbyregexp(const char **dirs,
                           int          ndirs,
                           const char  *pattern,
                           char      ***filenames,
                           int         *nfilenames,
                           char       **buffer)
{
  result_t    err;
  regex_t     regex;
  const char *dirname;
  char       *buf            = NULL;
  int         buf_allocated  = 0;
  int         buf_used       = 0;
  char      **ptrs           = NULL;
  int         ptrs_allocated = 0;
  int         ptrs_used      = 0;

  *filenames  = NULL;
  *nfilenames = 0;
  *buffer     = NULL;

  // TODO: Investigate fnmatch() to see if it's more appropriate.
  if (regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB))
    return result_BAD_ARG;

  while (ndirs--)
  {
    size_t         dirnamelen;
    DIR           *dir;
    struct dirent *entry;

    dirname    = *dirs++;
    dirnamelen = strlen(dirname);

    dir = opendir(dirname);
    if (dir == NULL)
    /* Consider this a transient error and fail at the end if no files are
     * found. */
      continue;

    while ((entry = readdir(dir)))
    {
      if (!regexec(&regex, entry->d_name, 0, NULL, 0))
      {
        int buf_need;

        /* account for separator and terminator */
        buf_need = (int) dirnamelen + 1 + entry->d_namlen + 1;

        if (array_grow((void **) &buf,
                       sizeof(*buf),
                       buf_used,
                       &buf_allocated,
                       buf_need,
                       256 /* minimum */))
        {
          err = result_OOM;
          goto failure;
        }

        if (array_grow((void **) &ptrs,
                       sizeof(*ptrs),
                       ptrs_used,
                       &ptrs_allocated,
                       1 /* need */,
                       8 /* minimum */))
        {
          err = result_OOM;
          goto failure;
        }

        {
          char *nextbuf;

          nextbuf = &buf[buf_used];

          memcpy(nextbuf, dirname, dirnamelen);
          nextbuf[dirnamelen] = '/';
          memcpy(nextbuf + dirnamelen + 1, entry->d_name, entry->d_namlen);
          nextbuf[buf_need - 1] = '\0';
          buf_used += buf_need;

          /* We can't store pointers here as we may realloc the
           * buffer at any point invalidating those pointers. Instead
           * store deltas from the buffer start and fix them up in a
           * pass at the end of the routine. */
          ptrs[ptrs_used++] = (char *) (nextbuf - buf);
        }
      }
    }

    closedir(dir);
  }

  regfree(&regex);

  /* If we didn't find any files return an error. */
  if (ptrs_used == 0)
  {
    err = result_NOT_FOUND;
    goto failure;
  }

  /* Fix up the deltas into pointers. */
  {
    int i;

    for (i = 0; i < ptrs_used; i++)
      ptrs[i] = ptrs[i] + (intptr_t) buf;
  }

  qsort(ptrs, ptrs_used, sizeof(*ptrs), filenamecompare);

  *filenames  = ptrs;
  *nfilenames = (int) ptrs_used;
  *buffer     = buf;
  
  return result_OK;
  
  
failure:
  
  free(ptrs);
  free(buf);
  
  return err;
}
