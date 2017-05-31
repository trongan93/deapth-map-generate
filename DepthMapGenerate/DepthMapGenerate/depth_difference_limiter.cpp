#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define	full	(width*height)
#define max_dif 20

int depth_difference_limiter(int width, int height, unsigned char *pre_output_d, unsigned char *ori_d, unsigned char *output_d)
{
	int i, j;
	int *dif;
	dif = new int[full];
	int max = 0;

	for (i = 0; i < full; i++) {
		dif[i] = ori_d[i] - pre_output_d[i];
		if (abs(dif[i]) > max)
			max = abs(dif[i]);
	}
	for (i = 0; i < full; i++) {
		output_d[i] = unsigned char(pre_output_d[i] + 1.0 * dif[i] / max * max_dif + 0.5);
		//printf("%d\t%f\t%d\n", pre_output_d[i], 1.0 * dif[i] / max * max_dif + 0.5, output_d[i]);
	}
	delete[] dif;
	return 0;
}