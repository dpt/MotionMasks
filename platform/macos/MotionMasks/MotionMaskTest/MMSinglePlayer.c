//
//  MMSinglePlayer.c
//  MotionMasks
//
//  Created by David Thomas on 27/11/2012.
//  Copyright (c) 2012 David Thomas. All rights reserved.
//

#include <stddef.h>

#include "base/result.h"

#include "MMPlayer.h"

#include "MMSinglePlayer.h"

static struct
{
  MMPlayer_t *player;
  int         refs;
}
LOCALS;

result_t MMSinglePlayer_instance(MMPlayer_t **player)
{
  result_t err;

  if (LOCALS.player == NULL)
  {
    err = MMPlayer_create(&LOCALS.player);
    if (err)
      return err;
  }

  *player = LOCALS.player;
  LOCALS.refs++;

  return result_OK;
}

void MMSinglePlayer_destroy(void)
{
  if (LOCALS.refs == 0)
    return;

  if (--LOCALS.refs > 0)
    return;

  MMPlayer_destroy(LOCALS.player);
  LOCALS.player = NULL;
}
