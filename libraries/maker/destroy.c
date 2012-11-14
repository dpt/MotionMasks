/* destroy.c */

#include <stdlib.h>

#include "maker/make.h"

#include "impl.h"

void motionmaskmaker_destroy(motionmaskmaker_t *doomed)
{
  free(doomed);
}
