#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#define full (width*height)
#define edge_rate 0.4

#define size_th 1000
#define cub_th 0.6
#define TB_range 10
#define HZ_range 200
#define PI 3.14159265

int vp_depth_estimation(int width, int height, int *label, int label_count, int V_x, int V_y, unsigned char *depth_map)
{
	int f, i, j, k;
	unsigned char r;
	/*=====region information=====*/
	struct reg_inf
	{
		int top;
		int bot;
		int size;
		int cub_size;
		unsigned int sum_dep;
	};

	struct reg_inf *reg;
	reg = new struct reg_inf[label_count];


	/*=====region information=====*/
	printf("region information.....");
	for (i = 0; i < label_count; i++) {
		reg[i].top = -1;
		reg[i].bot = 0;
		reg[i].size = 0;
		reg[i].cub_size = 0;
		reg[i].sum_dep = 0;
	}

	for (i = 0; i < width * height; i++) {
		if (label[i] != 0) {
			if (reg[label[i]].top == -1) {
				reg[label[i]].top = i;
			}
			reg[label[i]].bot = i;
			reg[label[i]].size++;
		}
	}

	printf("Done\n");
	/*=====Initial depth generation=====*/
	int cur_dep_x, cur_dep_y;
	int dis_x, dis_y;
	int cur_dep;
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			dis_x = i - V_x;
			dis_y = j - V_y;
			if (dis_x >= 0) {
				cur_dep_x = dis_x * 1.0 / (1024 - V_x) * 255;
			}
			else {
				cur_dep_x = (-1.0) * dis_x / V_x * 255;
			}
			if (dis_y >= 0) {
				cur_dep_y = dis_y * 1.0 / (768 - V_y) * 255;
			}
			else {
				//Outdoor
				cur_dep_y = 0;
				//Indoor
				//cur_dep_y = (-1.0) * dis_y / V_y * 255;
			}
			if (cur_dep_x > cur_dep_y)
				depth_map[i + j * width] = cur_dep_x;
			else
				depth_map[i + j * width] = cur_dep_y;
		}
	}
	printf("Depth assign.....");

	double w_x, w_y;
	float a, b;

	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			cur_dep = 0;
			if (label[i + j * width] != 0) {
				if (reg[label[i + j * width]].size > size_th) {		//Region Size: BIG
					if (i < V_x) {
						if (j < V_y) {
							a = 1.0 * (V_y - j) / (V_x - i);
							b = 1.0 * V_y / V_x;
						}
						else {
							a = 1.0 * (j - V_y) / (V_x - i);
							b = 1.0 * (height - V_y) / V_x;
						}
					}
					else {
						if (j < V_y) {
							a = 1.0 * (V_y - j) / (i - V_x);
							b = 1.0 * V_y / (width - V_x);
						}
						else {
							a = 1.0 * (j - V_y) / (i - V_x);
							b = 1.0 * (height - V_y) / (width - V_x);
						}
					}
					if (a < b)
						reg[label[i + j * width]].cub_size++;
				}
				else {											//Region Size: SMALL
					reg[label[i + j * width]].sum_dep = reg[label[i + j * width]].sum_dep + depth_map[i + j * width];
					//					printf("%d\t%d\t%d\n", dis_x, dis_y,cur_dep);
					//					system("pause");
				}
			}
		}
	}

	for (i = 0; i < width*height; i++) {
		if (label[i] != 0) {
			if (reg[label[i]].size > size_th) {
				if (abs(reg[label[i]].top / width - V_y) < HZ_range && reg[label[i]].bot / width > (V_y + HZ_range)) {			//Under
					if (reg[label[i]].bot / width > height - TB_range) {
						dis_y = i / width - V_y;
						if (dis_y >= 0) {
							depth_map[i] = (dis_y * 1.0 / (768 - V_y) * 255);
						}
						else {
							depth_map[i] = ((-1.0) * dis_y / V_y * 255);
						}
					}
					else {
						if (1.0 * reg[label[i]].cub_size / reg[label[i]].size > cub_th) {
							if ((i%width) < V_x) {
								dis_x = V_x - (i%width);
								cur_dep = 1.0 * dis_x / V_x * 255;
							}
							else {
								dis_x = (i%width) - V_x;
								cur_dep = 1.0 * dis_x / (width - V_x) * 255;
							}
						}
						else {
							cur_dep = depth_map[reg[label[i]].bot % width + (reg[label[i]].bot / width) * width];
						}
						depth_map[i] = cur_dep;
					}
				}
				else if (abs(reg[label[i]].bot / width - V_y) < HZ_range && reg[label[i]].top / width < (V_y - HZ_range)) {		//Above
					if (reg[label[i]].top / width < TB_range) {
						depth_map[i] = 0;
						//						dis_y = i / width - V_y;
						//						if (dis_y >= 0){
						//							depth_map[i] = (dis_y * 1.0 / (768 - V_y) * 255);
						//						}
						//						else{
						//							depth_map[i] = ((-1.0) * dis_y / V_y * 255);
						//						}

					}
					else {
						if (1.0 * reg[label[i]].cub_size / reg[label[i]].size > cub_th) {
							if ((i%width) < V_x) {
								dis_x = V_x - (i%width);
								cur_dep = 1.0 * dis_x / V_x * 255;
							}
							else {
								dis_x = (i%width) - V_x;
								cur_dep = 1.0 * dis_x / (width - V_x) * 255;
							}
						}
						else {
							cur_dep = depth_map[reg[label[i]].bot % width + (reg[label[i]].bot / width) * width];
						}
						depth_map[i] = cur_dep;
					}
				}
				else {
					if (1.0 * reg[label[i]].cub_size / reg[label[i]].size > cub_th) {
						if ((i%width) < V_x) {
							dis_x = V_x - (i%width);
							cur_dep = 1.0 * dis_x / V_x * 255;
						}
						else {
							dis_x = (i%width) - V_x;
							cur_dep = 1.0 * dis_x / (width - V_x) * 255;
						}
					}
					else {
						cur_dep = depth_map[reg[label[i]].bot % width + (reg[label[i]].bot / width) * width];
					}
					depth_map[i] = cur_dep;
				}
			}
			else {
				r = reg[label[i]].sum_dep / reg[label[i]].size;
				depth_map[i] = r;
			}
		}
		else
			depth_map[i] = 0;
	}
	printf("Done\n");
	/*=====End=====*/
	return 0;
}