#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#define full (width*height)
#define pi 3.14159265359

#define focus_SAD_max 120000
#define vp_max 200
#define vp_min 70

int reliability_calculator(int width, int height, int vp_x, int vp_y, unsigned char* buffer2, int edge_th, float *vp_rel, float *foc_rel)
{
	int i, j, k, l, t;
	double s = 45;

	int mean;

	int *result;
	result = new int[180 * 2560]; //result[1280][180]
	unsigned char *result_map;
	result_map = new unsigned char[180 * 2560]; //result[1280][180]
	int *seed;
	seed = new int[(2560 / 20) * (180 / 6)];
	int *final_seed;
	final_seed = new int[(2560 / 20) * (180 / 6)];

	int m_x, m_y, b_value, b_count;
	int low_b = 0;

	for (i = 0; i < 32; i++) {
		for (j = 0; j < 32; j++) {
			if (i == 0 || i == 31 || j == 0 || j == 31) {
				b_value = 0;
				b_count = 0;
				for (m_y = 0; m_y < height / 32; m_y++) {
					for (m_x = 0; m_x < width / 32; m_x++) {
						b_value += buffer2[(i * height / 32 + m_y) * width + j * width / 32 + m_x];
						if (buffer2[(i * height / 32 + m_y) * width + j * width / 32 + m_x] > edge_th)
							b_count++;
					}
				}
				b_value = int(1.0 * b_value / (width / 32 * height / 32) + 0.5);
				//printf("%d, %d\nb_value = %d\nb_count = %d\n", j, i, b_value, b_count);
				//if (b_value < 10)
				if (b_count < 1.0 * width / 32 * height / 32 * 0.1)
					low_b++;
			}
		}
	}
	//printf("th = %d, high_b = %d\n", edge_th, 124 - low_b);
	//system("pause");
	for (t = 0; t < 1; t++) {
		if (t == 1) {
			vp_x = 290;
			vp_y = 512;
		}
		else if (t == 2) {
			vp_x = 0;
			vp_y = 0;
		}
		else if (t == 3) {
			vp_x = width - 1;
			vp_y = 0;
		}
		else if (t == 4) {
			vp_x = 0;
			vp_y = height - 1;
		}
		else if (t == 5) {
			vp_x = width - 1;
			vp_y = height - 1;
		}

		if (vp_x != -1 && vp_y != -1) {
			/*===== Hough Transform=====*/
			int deg;
			int dis;
			for (i = 0; i < 180 * 2560; i++) {
				result[i] = 0;
			}
			for (i = 0; i < height; i++) {
				for (j = 0; j < width; j++) {
					if (buffer2[i * width + j] > edge_th) {
						for (deg = 0; deg < 180; deg++) {
							if (deg < 90) {
								dis = -1 * int((1.0 * i - tan(1.0 * deg * pi / 180) * j - vp_y + tan(1.0 * deg * pi / 180) * vp_x) / sqrt(tan(1.0 * deg * pi / 180) * tan(1.0 * deg * pi / 180) + 1));
							}
							else if (deg > 90) {
								dis = int((1.0 * i - tan(1.0 * deg * pi / 180) * j - vp_y + tan(1.0 * deg * pi / 180) * vp_x) / sqrt(tan(1.0 * deg * pi / 180) * tan(1.0 * deg * pi / 180) + 1));
							}
							else {
								dis = vp_x - j;
							}
							if (deg == 45 || deg == 0 || deg == 135 || dis == 0)
								;
							else
								//result[(dis + 1280) * 180 + deg]++;
								result[(dis + 1280) * 180 + deg] += buffer2[i * width + j];
						}
					}
				}
			}

			int max = 0;
			int min = abs(sqrt(1.0 * 1024 * 1024 + 768 * 768));
			int total = 0;
			for (i = 0; i < 2560 * 180; i++) {
				if (result[i] > 0)
					total++;
				if (abs(result[i]) > max)
					max = abs(result[i]);
				if (abs(result[i]) < min)
					min = abs(result[i]);
			}
			for (i = 0; i < 2560 * 180; i++) {
				result_map[i] = unsigned char(1.0 * (abs(result[i]) - min) / (max - min) * 255 + 0.5);
			}
			/*=====find local maximun=====*/
			//Find th
			int counter = 0;
			int th;
			th = 256;
			while (counter < 0.001 * total) {
				th--;
				counter = 0;
				for (i = 0; i < 2560; i++) {
					for (j = 0; j < 180; j++) {
						if (result_map[i * 180 + j] >= th)
							counter++;
					}
				}
			}
			//set initial seed dis:20 deg:6
			for (i = 0; i < 2560 / 20; i++) {
				for (j = 0; j < 180 / 6; j++) {
					seed[i * (180 / 6) + j] = (i * 20 + 10) * 180 + 6 * j + 3;
				}
			}
			//seed shifting
			int change = 1;
			int large;
			int large_pos;
			while (change == 1) {
				change = 0;
				//fwrite(result_map, 1, 2560*180, output);
				for (i = 0; i < (2560 / 20) * (180 / 6); i++) {
					if (seed[i] != -1) {
						large = 0;
						large_pos = -1;
						for (j = -10; j < 10; j++) {
							for (k = -3; k <3; k++) {
								if (seed[i] / 180 + j > 0 && seed[i] / 180 + j < 2560 && seed[i] % 180 + k > 0 && seed[i] % 180 + k < 180) {
									if (result[seed[i] + j * 180 + k] > large && result_map[seed[i] + j * 180 + k] > th) {
										large_pos = seed[i] + j * 180 + k;
										large = result[seed[i] + j * 180 + k];
									}
								}
							}
						}
						if (large_pos != seed[i]) {
							change = 1;
							seed[i] = large_pos;
						}
					}
				}
			}
			int seed_count = 0;
			for (i = 0; i < (2560 / 20) * (180 / 6); i++) {
				if (seed[i] != -1) {
					for (j = 0; j < seed_count; j++)
						if (final_seed[j] == seed[i])
							break;
					if (j == seed_count) {
						final_seed[seed_count] = seed[i];
						seed_count++;
					}
				}
			}
			for (i = 0; i < seed_count; i++)
				printf("%d %d\n", i, abs(final_seed[i] / 180 - 1280));
			/*=====Find the mean Dis. of main line=====*/
			mean = 0;
			for (i = 0; i < seed_count; i++) {
				mean += abs(final_seed[i] / 180 - 1280);
			}
			mean = int(1.0 * mean / seed_count + 0.5);
		}
	}
	if (vp_x != -1 && vp_y != -1 && (124 - low_b) > 20)
		if (mean > vp_max)
			*vp_rel = 0;
		else if (mean < vp_min)
			*vp_rel = 1;
		else
			*vp_rel = 1.0 - 1.0 * (mean - vp_min) / (vp_max - vp_min);
	else {
		mean = -1;
		*vp_rel = 0;
	}
	/*================================*/
	/*===== defocus and in-focus =====*/
	/*================================*/
	int his[256] = { 0 };
	int focus[256] = { 0 };
	int SAD = 0;

	for (i = 1; i < height - 1; i++)
		for (j = 1; j < width - 1; j++)
			his[buffer2[i * width + j]]++;
	/*
	FILE *focus_his;
	focus_his = fopen("focus_cue_his.txt","rt");

	for (i = 0; i < 40; i++){
	fscanf(focus_his, "%d", &focus[15 + i]);
	}

	for (i = 0; i < 256; i++){
	if (focus[i] != 0)
	SAD += his[i] - focus[i];
	}

	if (SAD < 0)
	*foc_rel = 1;
	else if (SAD < focus_SAD_max)
	*foc_rel = 1.0 - 1.0 * SAD / focus_SAD_max;
	else
	*foc_rel = 0;
	*/

	/*
	for (i = 0; i < 256; i++)
	printf("%d\n", focus[i]);
	system("pause");
	*/
	double hi_rate = 0;

	for (i = 0; i < 256; i++) {
		if (i >= 15)
			hi_rate += his[i];
	}
	hi_rate = hi_rate / width / height;

	if (hi_rate < 6)
		*foc_rel = 1;
	else if (hi_rate < 18)
		*foc_rel = 1.0 - 1.0 * (hi_rate - 6) / 12;
	else
		*foc_rel = 0;

	printf("%d\t%f\n", mean, *vp_rel);
	printf("%d\t%f\n", SAD, *foc_rel);

	delete(result);
	delete(result_map);
	delete(seed);
	//fclose(focus_his);
	return 0;
}