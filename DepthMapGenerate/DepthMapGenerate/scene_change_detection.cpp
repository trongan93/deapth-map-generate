#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#define	full	(width*height)

float MAFD(int w, int h, unsigned char *pre_frame, unsigned char *cur_frame);
float FV(int w, int h, float MAFD, unsigned char *cur_frame);
int HISTEQ(int w, int h, unsigned char *y_data, unsigned char *out_y_data);

int scene_change_detection(int width, int height, int f, unsigned char *pre_frame, unsigned char *cur_frame, float *pre_data, float *cur_data)
{
	//data[5] = {cur_mafd, cur_mafd_HE, cur_sdmafd_HE, cur_FV_HE, cur_adfv_HE};
	int i, j, k;

	unsigned char *pre_frame_HE;
	pre_frame_HE = new unsigned char[full];
	unsigned char *cur_frame_HE;
	cur_frame_HE = new unsigned char[full];

	//HISTEQ
	HISTEQ(width, height, cur_frame, cur_frame_HE);
	HISTEQ(width, height, pre_frame, pre_frame_HE);

	if (f > 0) {
		//MAFD
		cur_data[0] = MAFD(width, height, pre_frame, cur_frame);
		//MAFD_HE
		cur_data[1] = MAFD(width, height, pre_frame_HE, cur_frame_HE);
		//FV
		cur_data[3] = FV(width, height, cur_data[1], cur_frame_HE);
		if (f > 1) {
			//SDMAFD_HE
			cur_data[2] = cur_data[1] - pre_data[1];
			//ADFV_HE
			cur_data[4] = abs(cur_data[3] - pre_data[3]);
		}
	}

	delete(pre_frame_HE);
	delete(cur_frame_HE);
	if (f > 1) {
		if (cur_data[0] < 14)
			return 0;
		else if (cur_data[1] < 40)
			return 0;
		else {
			if (cur_data[1] < 100 && cur_data[0] > 58 && cur_data[4] > 23)
				return 1;
			else if (cur_data[4]  < 2 || cur_data[2] < 5)
				return 0;
			else if (cur_data[1] > 50 && cur_data[0] > 35 && cur_data[2] < 50 && cur_data[4] > 50)
				return 1;
			else
				return 0;
		}
	}
	return 0;
}

float MAFD(int w, int h, unsigned char *pre_frame, unsigned char *cur_frame)
{
	int i, SAD = 0;
	float final_value;

	for (i = 0; i < w * h; i++)
		SAD += abs(cur_frame[i] - pre_frame[i]);

	final_value = 1.0 * SAD / (w * h);

	return final_value;
}

float FV(int w, int h, float MAFD, unsigned char *cur_frame)
{
	int i, SAD = 0;
	float final_value;

	for (i = 0; i < w * h; i++)
		SAD += abs(cur_frame[i] - MAFD);

	final_value = 1.0 * SAD / (w * h);

	return final_value;
}

int HISTEQ(int w, int h, unsigned char *y_data, unsigned char *out_y_data)
{
	/*
	FILE *data_his;
	data_his = fopen("D:\\C_program_data\\scene_change_detection\\color_pro_nor.txt", "w");
	fprintf(data_his, "%f\n",his[i]);
	*/

	int i;
	int his[256] = { 0 };
	int sum = 0;
	int min = 255;
	int max = 0;
	//histogram
	for (i = 0; i < w * h; i++) {
		his[y_data[i]]++;
	}
	//normalized to [0,1]
	for (i = 0; i < 256; i++) {
		sum += his[i];
		if (his[i] != 0) {
			his[i] = sum;
			if (i > max)
				max = i;
			if (i < min)
				min = i;
		}
		else
			his[i] = sum;
	}
	//HISTEQ
	for (i = 0; i < w * h; i++)
		out_y_data[i] = unsigned char(1.0 * (his[y_data[i]] - his[min]) / (his[max] - his[min]) * 255 + 0.5);

	return 0;
}
