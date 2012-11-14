/* destroy.c */

#include <stdlib.h>

#include "player/play.h"

#include "impl.h"

void motionmaskplayer_destroy(motionmaskplayer_t *doomed)
{
  free(doomed);
}
