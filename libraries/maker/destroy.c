/* destroy.c */

#include <stdlib.h>

#include "maker/make.h"

#include "impl.h"

void motionmaskmaker_destroy(motionmaskmaker_t *doomed)
{
  if (doomed == NULL)
    return;
  
  free(doomed->frames);
  free(doomed->offsets);
  free(doomed->data);
  
  free(doomed);
}
