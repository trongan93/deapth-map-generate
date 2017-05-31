#include <stdio.h>
#include <stdlib.h>
#include <cmath>

int vanishing_point_detection(int width, int height, unsigned char *y, int *Ori, int *xx, int *yy)
{
	int i, j, k, l, a, b;
	/*--------Soble with Ori information--------*/
	/*--------LDOS--------*/
	// 32*32 blocks
	int *LDOS;
	LDOS = new int[32 * 32 * 18];
	int block_counter = 0;
	int his[19];
	int last_max_ori;
	int max;
	int max_ori;
	/*-----------Dynamic Programming----------*/
	int mask[8] = { -33, -32, -31, -1, 1, 31, 32, 33 };

	int new_pos[32 * 32];
	int new_pos_count = 0;

	int pos_temp[32 * 32];
	int pos_temp_count = 0;

	int D1;
	int D2;
	float s;

	int last_blocks[32 * 32] = { 0 };
	int connected_blocks[32 * 32];
	int CB_counter;

	/*==========Main program=========*/
	/*--------LDOS--------*/
	// 32*32 blocks

	for (j = 0; j < height; j = j + height / 32) {
		for (i = 0; i < width; i = i + width / 32) {
			for (l = 0; l < 19; l++)
				his[l] = 0;
			for (l = 0; l < height / 32; l++) {
				for (k = 0; k < width / 32; k++) {
					his[Ori[j * width + i + l * width + k]] = his[Ori[j * width + i + l * width + k]] + y[j * width + i + l * width + k];
				}
			}
			/*--------------Sorting--------------*/
			last_max_ori = 18;
			for (a = 0; a < 18; a++) {
				max = 0;
				max_ori = 18;
				for (b = 0; b < 18; b++) {
					if (max <= his[b]) {
						max = his[b];
						max_ori = b;
					}
				}
				if (max != 0) {
					LDOS[block_counter * 18 + a] = max_ori;
					last_max_ori = max_ori;
					his[max_ori] = 0;
				}
				else {
					LDOS[block_counter * 18 + a] = last_max_ori;
				}
			}
			block_counter++;
		}
	}


	/*-----------Dynamic Programming----------*/
	for (i = 0; i < 32 * 32; i++) {																//Start from outer blocks
		if (i / 32 == 0 ||
			i / 32 == 31 ||
			i % 32 == 0 ||
			i % 32 == 31) {
			new_pos[new_pos_count] = i;
			new_pos_count++;
		}
		for (j = 0; j < 32 * 32; j++)
			connected_blocks[j] = 0;
		connected_blocks[i] = 1;

		for (;;) {
			if (new_pos_count == 0)
				break;

			for (j = 0; j < new_pos_count; j++) {
				for (a = 0; a < 8; a++) {
					if (new_pos[j] / 32 == 0 && (a == 0 || a == 1 || a == 2));
					else if (new_pos[j] / 32 == 31 && (a == 5 || a == 6 || a == 7));
					else if (new_pos[j] % 32 == 0 && (a == 0 || a == 3 || a == 5));
					else if (new_pos[j] % 32 == 31 && (a == 2 || a == 4 || a == 7));
					else {
						if (connected_blocks[new_pos[j] + mask[a]] != 1 && LDOS[(new_pos[j]) * 18 + 0] != 18) {
							D1 = abs(LDOS[new_pos[j] * 18 + 0] - LDOS[(new_pos[j] + mask[a]) * 18 + 0]);
							D2 = abs(LDOS[new_pos[j] * 18 + 1] - LDOS[(new_pos[j] + mask[a]) * 18 + 1]);
							if (D1 > 8)
								D1 = 17 - D1;
							if (D2 > 8)
								D2 = 17 - D2;
							if (LDOS[(new_pos[j] + mask[a]) * 18 + 0] != 18)
								s = D1 * 0.7 + D2 * 0.3;
							else
								s = 2.1;
							if (s < 1) {
								connected_blocks[new_pos[j] + mask[a]] = 1;
								pos_temp[pos_temp_count] = new_pos[j] + mask[a];
								pos_temp_count++;
							}
						}
					}
				}
			}
			//Updata pos buffer
			if (pos_temp_count != 0) {
				for (a = 0; a < pos_temp_count; a++)
					new_pos[a] = pos_temp[a];
			}
			else {
				for (a = 0; a < new_pos_count; a++) {
					last_blocks[new_pos[a]] = 1;
				}
			}
			new_pos_count = pos_temp_count;
			pos_temp_count = 0;
		}

	}

	/*------- Vanishing point location-------*/
	double x_std = 0;
	double x_sqr = 0;
	double y_std = 0;
	double y_sqr = 0;

	double x_V_point = 0;
	double y_V_point = 0;

	int point_counter = 0;

	for (i = 0; i < 32 * 32; i++) {
		if (i / 32 != 0 &&
			i / 32 != 31 &&
			i % 32 != 0 &&
			i % 32 != 31) {
			if (last_blocks[i] == 1) {
				x_std = x_std + (i % 32) * (i % 32);
				x_sqr = x_sqr + (i % 32);
				y_std = y_std + (i / 32) * (i / 32);
				y_sqr = y_sqr + (i / 32);
				point_counter++;
			}
		}
	}
	printf("point_counter = %d\n", point_counter);

	x_sqr = x_sqr / point_counter;
	y_sqr = y_sqr / point_counter;
	x_std = sqrt(x_std / point_counter - x_sqr * x_sqr);
	y_std = sqrt(y_std / point_counter - y_sqr * y_sqr);

	point_counter = 0;

	for (i = 0; i < 32 * 32; i++) {
		if (i / 32 != 0 &&
			i / 32 != 31 &&
			i % 32 != 0 &&
			i % 32 != 31) {
			if (last_blocks[i] == 1) {
				*xx = i % 32;
				*yy = i / 32;
				if (abs(*xx - x_sqr) < x_std && abs(*yy - y_sqr) < y_std) {
					x_V_point = x_V_point + *xx;
					y_V_point = y_V_point + *yy;
					point_counter++;
				}
			}
		}
	}
	x_V_point = x_V_point / point_counter;
	y_V_point = y_V_point / point_counter;

	x_V_point = (x_V_point + 0.5) / 32 * 1024;
	y_V_point = (y_V_point + 0.5) / 32 * 768;

	if (point_counter != 0) {
		*xx = int(x_V_point);
		*yy = int(y_V_point);
	}
	else {
		*xx = -1;
		*yy = -1;
	}
	/*--------End--------*/
	delete(LDOS);
	return 0;
}