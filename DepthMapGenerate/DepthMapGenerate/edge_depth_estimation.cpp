#include <stdio.h>
#include <stdlib.h>
#include <cmath> 

#define full (width*height)
#define pi 3.14159265359

int edge_depth_estimation(int width, int height, unsigned char *buffer2, int edge_th, int *label_map, int label_count, unsigned char *depth_map)
{

	int i, j, k, l;
	double s = 45;

	int vp_x = 0;
	int vp_y = 0;

	int *result;
	result = new int[180 * 2560]; //result[1280][180]

	unsigned char *result_map;
	result_map = new unsigned char[180 * 2560]; //result[1280][180]

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
						dis = -1 * int((1.0 * tan(1.0 * deg * pi / 180) * i - j + vp_y - tan(1.0 * deg * pi / 180) * vp_x) / sqrt(tan(1.0 * deg * pi / 180) * tan(1.0 * deg * pi / 180) + 1));
					}
					else if (deg > 90) {
						dis = int((1.0 * tan(1.0 * deg * pi / 180) * i - j + vp_y - tan(1.0 * deg * pi / 180) * vp_x) / sqrt(tan(1.0 * deg * pi / 180) * tan(1.0 * deg * pi / 180) + 1));
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
	printf("%d\t%d\n", max, min);
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
	int *seed;
	seed = new int[(2560 / 20) * (180 / 6)];
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

	int *final_seed;
	final_seed = new int[(2560 / 20) * (180 / 6)];
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
	/*	fprintf(log, "Main Line\n");
	for (i = 0; i < seed_count; i++){
	fprintf(log, "%d, %d\n", final_seed[i]/180 - 1280, final_seed[i]%180);
	}
	*/

	/*=====Line Feature=====*/
	//Vertical Line Feature
	int Ver_line_count = 0;
	double left_var = 0;
	double right_var = 0;
	for (i = 0; i < seed_count; i++) {
		if (abs(final_seed[i] % 180 - 90) < 20) {
			left_var = (abs(final_seed[i] / 180 - 1280) - 0) * (abs(final_seed[i] / 180 - 1280) - 0);
			right_var = (abs(final_seed[i] / 180 - 1280) - 1024) * (abs(final_seed[i] / 180 - 1280) - 1024);
			Ver_line_count++;
		}
	}
	if (Ver_line_count != 0) {
		left_var = sqrt(left_var / Ver_line_count);
		right_var = sqrt(right_var / Ver_line_count);
	}
	else {
		left_var = -1;
		right_var = -1;
	}
	//	fprintf(log, "Var\n%lf\t%lf\n", left_var, right_var);


	for (i = 0; i < 2560 * 180; i++) {
		result_map[i] = 0;

		if (result[i] > th)
			result_map[i] = unsigned char(1.0 * (abs(result[i]) - min) / (max - min) * 255);
		else
			result_map[i] = 0;
		result_map[i] = 0;

	}
	for (i = 0; i < (2560 / 20) * (180 / 6); i++) {
		if (seed[i] != -1) {
			result_map[seed[i]] = 255;
		}
	}
	/*=====Depth Asign=====*/
	//weighting decision
	float Wx, Wy;
	if (Ver_line_count == 0) {
		Wx = 0;
	}
	else if (left_var > right_var) {
		Wx = (1 - right_var / left_var) * 0.5;
	}
	else {
		Wx = -1.0 * (1 - left_var / right_var) * 0.5;
	}
	Wy = 1 - abs(Wx);
	//	fprintf(log, "Wx\tWy\n%f\t%f\n", left_var, right_var);

	//depth asign	
	int *label_x, *label_y, *label_size;
	label_x = new int[label_count];
	label_y = new int[label_count];
	label_size = new int[label_count];

	for (i = 0; i < label_count; i++) {
		label_x[i] = 0;
		label_y[i] = 0;
		label_size[i] = 0;
	}

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			label_x[label_map[i * width + j]] += j;
			label_y[label_map[i * width + j]] += i;
			label_size[label_map[i * width + j]]++;
		}
	}

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			if (Wx < 0) {
				depth_map[i * width + j] = unsigned char(1.0 * Wy * 255 * (1.0 * label_y[label_map[i * width + j]] / label_size[label_map[i * width + j]] / height) - 1.0 * Wx * 255 * (1 - 1.0 * label_x[label_map[i * width + j]] / label_size[label_map[i * width + j]] / width));
			}
			else {
				depth_map[i * width + j] = unsigned char(1.0 * Wy * 255 * (1.0 * label_y[label_map[i * width + j]] / label_size[label_map[i * width + j]] / height) + 1.0 * Wx * 255 * (1.0 * label_x[label_map[i * width + j]] / label_size[label_map[i * width + j]] / width));
			}
		}
	}

	//system("pause");
	delete(result);
	delete(result_map);
	delete(seed);
	delete(final_seed);
	delete(label_x);
	delete(label_y);
	delete(label_size);
	return 0;
}