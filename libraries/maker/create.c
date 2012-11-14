/* create.c */

#include <stddef.h>
#include <stdlib.h>

#include "base/error.h"

#include "maker/make.h"

#include "impl.h"

mmerror_t motionmaskmaker_create(motionmaskmaker_t **pmaker)
{
  motionmaskmaker_t *maker;

  *pmaker = NULL;

  maker = calloc(1, sizeof(*maker));
  if (maker == NULL)
    return mmerror_OOM;

  *pmaker = maker;

  return mmerror_OK;
}
