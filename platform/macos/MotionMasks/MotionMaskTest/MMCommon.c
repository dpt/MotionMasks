//
//  MMCommon.c
//  MotionMasks
//
//  Created by David Thomas on 27/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <stddef.h>

#include "base/mmerror.h"

#include "MMPlayer.h"

#include "MMCommon.h"

static MMPlayer_t *player;
static int         refs;

mmerror_t MMCommon_Player_instance(MMPlayer_t **pplayer)
{
  mmerror_t err;

  if (player == NULL)
  {
    err = MMPlayer_create(&player);
    if (err)
      return err;
  }

  *pplayer = player;
  refs++;

  return mmerror_OK;
}

void MMCommon_Player_destroy(void)
{
  if (refs == 0)
    return;

  if (--refs > 0)
    return;

  MMPlayer_destroy(player);
  player = NULL;
}
