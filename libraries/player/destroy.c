/* destroy.c */

#include <stdlib.h>

#include "player/play.h"

#include "impl.h"

void motionmaskplayer_destroy(motionmaskplayer_t *doomed)
{
  if (doomed == NULL)
    return;
  
  free(doomed->frames);
  free(doomed->offsets);
  free(doomed->data);
  
  free(doomed);
}
