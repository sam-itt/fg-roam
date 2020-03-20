#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include "misc.h"

// normalize the value to be in a range between [min, max[
double normalize_periodicd(double min, double max, double value)
{
    double range = max - min;
    if (range < DBL_MIN)
      return min;
    double normalized = value - range*floor((value - min)/range);
    // two security checks that can only happen due to roundoff
    if (normalized <= min)
      return min;
    if (max <= normalized)
      return min;
    return normalized;
}

char *get_sized_unit_text(size_t amount)
{
	if(amount >= GB_AMOUNT )
		return("GB");
	if(amount >= MB_AMOUNT )
		return("MB");
	if(amount >= KB_AMOUNT )
		return("KB");
	return("Bytes");
}

double get_sized_unit_value(size_t amount)
{
	if(amount >= GB_AMOUNT ){
		return(amount/GB_AMOUNT);
	}
	if(amount >= MB_AMOUNT ){
		return(amount/MB_AMOUNT);
	}
	if(amount >= KB_AMOUNT ){
		return(amount/KB_AMOUNT);
	}
	return(amount);
}


#ifdef ENABLE_TEST
int main(int argc, char *argv[])
{


	exit(EXIT_SUCCESS);
}
#endif

