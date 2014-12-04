/* create.c */

#include <stddef.h>
#include <stdlib.h>

#include "base/result.h"

#include "player/play.h"

#include "impl.h"

result_t motionmaskplayer_create(motionmaskplayer_t **pplayer)
{
  motionmaskplayer_t *player;

  *pplayer = NULL;

  player = calloc(1, sizeof(*player));
  if (player == NULL)
    return result_OOM;

  *pplayer = player;

  return result_OK;
}
