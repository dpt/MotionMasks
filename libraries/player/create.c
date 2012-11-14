/* create.c */

#include <stddef.h>
#include <stdlib.h>

#include "base/mmerror.h"

#include "player/play.h"

#include "impl.h"

mmerror_t motionmaskplayer_create(motionmaskplayer_t **pplayer)
{
  motionmaskplayer_t *player;

  *pplayer = NULL;

  player = calloc(1, sizeof(*player));
  if (player == NULL)
    return mmerror_OOM;

  *pplayer = player;

  return mmerror_OK;
}
