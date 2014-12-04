/* create.c */

#include <stddef.h>
#include <stdlib.h>

#include "base/result.h"

#include "maker/make.h"

#include "impl.h"

result_t motionmaskmaker_create(motionmaskmaker_t **pmaker)
{
  motionmaskmaker_t *maker;

  *pmaker = NULL;

  maker = calloc(1, sizeof(*maker));
  if (maker == NULL)
    return result_OOM;

  *pmaker = maker;

  return result_OK;
}
