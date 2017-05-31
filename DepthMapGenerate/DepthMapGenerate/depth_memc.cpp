#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define	full	(width*height)
#define b_size 8
#define s_range 16

int depth_memc(int width, int height, unsigned char *pre_input_y, unsigned char *input_y, unsigned char *pre_output_d, unsigned char *output_d)
{
	int i, j;
	int s_x, s_y;
	int b_x, b_y;

	for (i = 0; i < height; i += b_size) {
		for (j = 0; j < width; j += b_size) {
			//Color ME
			int min = 255 * b_size * b_size;
			int mv_x, mv_y;
			//printf("(%.4d,%.4d)\n", i, j);
			for (s_y = -s_range; s_y < s_range; s_y++) {
				for (s_x = -s_range; s_x < s_range; s_x++) {
					//printf(" (%.4d,%.4d)\t", i + s_y, j + s_x);
					if (i + s_y >= 0 && i + s_y + b_size < height && j + s_x >= 0 && j + s_x + b_size < width) {
						int SAD = 0;
						for (b_y = 0; b_y < b_size; b_y++) {
							for (b_x = 0; b_x < b_size; b_x++) {
								//printf("(%.4d,%.4d)\t(%.4d,%.4d)\n", i + b_y, j + b_x, i + s_y + b_y, j + s_x + b_x);
								SAD += abs(input_y[(i + b_y) * width + j + b_x] - pre_input_y[(i + s_y + b_y) * width + j + s_x + b_x]);
							}
						}
						if (SAD < min) {
							min = SAD;
							mv_y = s_y;
							mv_x = s_x;
						}
					}
				}
			}
			//Depth MC
			for (b_y = 0; b_y < b_size; b_y++) {
				for (b_x = 0; b_x < b_size; b_x++) {
					output_d[(i + b_y) * width + j + b_x] = pre_output_d[(i + mv_y + b_y) * width + j + mv_x + b_x];
				}
			}
		}
	}

	return 0;
}