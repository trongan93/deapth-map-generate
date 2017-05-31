#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <string.h>
#include <Windows.h>
#include "sub_programs.h"

using namespace std;

//#define width 1024
//#define height 768
#define full (width*height)
//#define frame 10
#define start_f 0

int main(int argc, char* argv[])
{
	if (argc < 2 || (strcmp(argv[1], "-r") != 0 && strcmp(argv[1], "-rs") != 0))
		return 0;

	int frame, width, height;

	frame = atoi(argv[2]);
	width = atoi(argv[3]);
	height = atoi(argv[4]);

	char O_test[100], O_dep[100], O_fd[100], O_log[100];

	strcpy(O_test, argv[5]);
	strcat(O_test, "_test.yuv");
	strcpy(O_dep, argv[5]);
	strcat(O_dep, "_ori_depth.yuv");
	strcpy(O_fd, argv[5]);
	strcat(O_fd, "_final_depth.yuv");
	strcpy(O_log, argv[5]);
	strcat(O_log, "_log.txt");


	DWORD t_b, t_n, t_r, t_s, t_e;

	t_b = GetTickCount();

	int f, i, j, k, l, t;
	double s = 45;

	int vp_x = 0;
	int vp_y = 0;

	FILE *input;
	input = fopen(argv[5], "rb");
	FILE *test;
	test = fopen(O_test, "wb");
	FILE *ori_output;
	ori_output = fopen(O_dep, "wb");
	FILE *final_output;
	final_output = fopen(O_fd, "wb");
	FILE *log;
	log = fopen(O_log, "w");

	int force_cue = -1;
	if (strcmp(argv[1], "-rs") == 0 && argc == 7) {
		force_cue = atoi(argv[6]);
	}


	unsigned char *input_y;
	input_y = new unsigned char[full];
	unsigned char *input_u;
	input_u = new unsigned char[full / 4];
	unsigned char *input_v;
	input_v = new unsigned char[full / 4];

	unsigned char *grad_1;
	grad_1 = new unsigned char[full];
	int *ori;
	ori = new int[full];

	int label_num;
	int *label_map;
	label_map = new int[full];
	unsigned char *edge_map;
	edge_map = new unsigned char[full];

	unsigned char *ori_d;
	ori_d = new unsigned char[full];
	unsigned char *output_d;
	output_d = new unsigned char[full];
	unsigned char *uv_d;
	uv_d = new unsigned char[full / 2];

	//>>>AUO depth buffer//
	unsigned char *ori_d_vp;
	ori_d_vp = new unsigned char[full];
	unsigned char *ori_d_fo;
	ori_d_fo = new unsigned char[full];
	unsigned char *ori_d_mlsd;
	ori_d_mlsd = new unsigned char[full];
	//<<<AUO depth buffer//

	for (i = 0; i < full / 2; i++)
		uv_d[i] = 128;

	unsigned char *pre_input_y;
	pre_input_y = new unsigned char[full];
	unsigned char *pre_ori_d;
	pre_ori_d = new unsigned char[full];
	unsigned char *pre_output_d;
	pre_output_d = new unsigned char[full];
	unsigned char *temp_pointer;

	unsigned char **set_d;
	set_d = new unsigned char*[5];
	for (i = 0; i < 5; i++) {
		set_d[i] = new unsigned char[full];
	}

	float cur_color_data[5];
	float pre_color_data[5];
	float cur_depth_data[5];
	float pre_depth_data[5];

	int cue_rate[3] = { 0 };

	for (f = 0; f < start_f; f++) {
		fread(input_y, 1, full, input);
		fread(input_u, 1, full / 4, input);
		fread(input_v, 1, full / 4, input);
	}

	for (f = 0; f < frame; f++) {
		fread(input_y, 1, full, input);
		fread(input_u, 1, full / 4, input);
		fread(input_v, 1, full / 4, input);
		printf("frame %d\n", f);

		//===== 1st Gradient and VP detection =====//
		int edge_th;
		edge_th = sobel_ori(width, height, input_y, grad_1, ori);
		fwrite(grad_1, 1, full, test);
		vanishing_point_detection(width, height, grad_1, ori, &vp_x, &vp_y);
		printf(" vp = %d,%d\n", vp_x, vp_y);
		//test output
		/*
		if (vp_x != -1 && vp_y != -1){
		unsigned char temp_255 = 255;
		unsigned char temp_0 = 0;
		for (i = 0; i < full; i++){
		if (abs(i%width - vp_x) < 10 && abs(i/width - vp_y) < 10)
		fwrite(&temp_255, 1, 1, test);
		else if (abs(i%width - vp_x) < 12 && abs(i/width - vp_y) < 12)
		fwrite(&temp_0, 1, 1, test);
		else
		fwrite(&input_y[i], 1, 1, test);
		}
		}*/
		//===== Cue Selection =====//
		float vp_rel;
		float foc_rel;
		int cue;
		reliability_calculator(width, height, vp_x, vp_y, grad_1, edge_th, &vp_rel, &foc_rel);
		if (vp_rel > 0.4 || foc_rel > 0.4)
			if (vp_rel > foc_rel)
				cue = 0;
			else
				cue = 1;
		else
			cue = 2;
		//test output
		printf(" vp_rel\tfoc_rel\n %lf\t%lf\n", vp_rel, foc_rel);
		cue_rate[cue]++;

		if (force_cue != -1)
			cue = force_cue;

		t_r = GetTickCount();
		printf("re time = %lf s\n", 1.0 * (t_r - t_b) / 1000);
		//===== Depth Estiamtion =====//
		label_num = watershed_segmentation(width, height, edge_th, grad_1, label_map, edge_map);
		unsigned char temp;
		/*
		for (i = 0; i < width*height; i++)
		{
		if (label_map[i] != 0){
		temp = label_map[i]%255;
		}
		else{
		temp = 0;
		}
		//fwrite(&temp, 1, 1, test);
		}
		*/

		//==========Original==========//

		switch (cue) {
		case 0:
			vp_depth_estimation(width, height, label_map, label_num, vp_x, vp_y, ori_d);
			break;
		case 1:
			focus_depth_estimation(width, height, grad_1, label_map, label_num, edge_map, ori_d);
			break;
		case 2:
			edge_depth_estimation(width, height, grad_1, edge_th, label_map, label_num, ori_d);
			break;
		}
		/*
		//==========Fusion AUO Ver.==========//
		vp_depth_estimation(width, height, label_map, label_num, vp_x, vp_y, ori_d_vp);
		focus_depth_estimation(width, height, grad_1, label_map, label_num, edge_map, ori_d_fo);
		edge_depth_estimation(width, height, grad_1, edge_th , label_map, label_num, ori_d_mlsd);

		// weighting assign
		float w_vp, w_fo, w_mlsd;
		if (vp_rel + foc_rel > 0.8)
		w_mlsd = 0;
		else
		w_mlsd = 1.0 * (vp_rel + foc_rel) / 0.8;
		w_vp = (1.0 - w_mlsd) * vp_rel / (vp_rel + foc_rel);
		w_fo = (1.0 - w_mlsd) * foc_rel / (vp_rel + foc_rel);

		for (i = 0; i < full; i++){																				//2-way fusion
		ori_d[i] = unsigned char(1.0 * (ori_d_vp[i] + ori_d_fo[i] + ori_d_mlsd[i]) / 3);					//Fusion by Avg.
		//ori_d[i] = unsigned char(w_vp * ori_d_vp[i] + w_fo * ori_d_fo[i] + w_mlsd * ori_d_mlsd[i]);		//Fusion by Weighting.
		}
		*/

		t_e = GetTickCount();
		printf("ES time = %lf s\n", 1.0 * (t_e - t_r) / 1000);

		//test output
		//===== Color and Depth Scene Change Detection =====//
		int color_SC;
		int depth_SC;
		int SAD = 0;
		if (f > 0) {
			for (i = 0; i < full; i++)
				SAD += abs(pre_input_y[i] - input_y[i]);
			SAD = 1.0 * SAD / full;
			printf("MAFD = %d\n", SAD);
		}
		color_SC = scene_change_detection(width, height, f, pre_input_y, input_y, pre_color_data, cur_color_data);
		depth_SC = scene_change_detection(width, height, f, pre_ori_d, ori_d, pre_depth_data, cur_depth_data);
		//test output
		printf("color_SC = %d\n", color_SC);
		printf("depth_SC = %d\n", depth_SC);
		//===== Depth Stability Enhancement=====//
		if (f != 0) {
			if (color_SC == 1) {
				/*
				for (i = 0; i < full; i++)
				output_d[i] = 0;
				*/
			}
			else {
				if (depth_SC == 1)
					depth_memc(width, height, pre_input_y, input_y, pre_output_d, output_d);
				else
					depth_difference_limiter(width, height, pre_output_d, ori_d, output_d);
			}
		}
		else {
			for (i = 0; i < full; i++)
				output_d[i] = ori_d[i];
		}
		//fwrite(output_d, 1, full, final_output);
		for (i = 0; i < full; i++)
			set_d[0][i] = output_d[i];
		filter_3d(width, height, f, set_d, output_d);

		//===== depth output=====//
		fwrite(ori_d, 1, full, ori_output);
		fwrite(uv_d, 1, full / 2, ori_output);
		fwrite(output_d, 1, full, final_output);
		fwrite(uv_d, 1, full / 2, final_output);
		//===== data shift "current -> previous" =====//
		//y
		temp_pointer = pre_input_y;
		pre_input_y = input_y;
		input_y = temp_pointer;
		//d
		temp_pointer = pre_ori_d;
		pre_ori_d = ori_d;
		ori_d = temp_pointer;
		//output d
		temp_pointer = pre_output_d;
		pre_output_d = output_d;
		output_d = temp_pointer;
		//SCD data
		for (i = 0; i < 5; i++) {
			//printf("%f ", cur_color_data[i]);
			pre_color_data[i] = cur_color_data[i];
			pre_depth_data[i] = cur_depth_data[i];
		}
		//d_set
		temp_pointer = set_d[4];
		set_d[4] = set_d[3];
		set_d[3] = set_d[2];
		set_d[2] = set_d[1];
		set_d[1] = set_d[0];
		set_d[0] = temp_pointer;
		printf("\n");
	}
	t_n = GetTickCount();
	printf("DSES time = %.2lf s\n", 1.0 * (t_n - t_e) / 1000);
	printf("total time = %.2lf s\n", 1.0 * (t_n - t_b) / 1000);
	printf("s/frame = %.2lf s\n\n", 1.0 * (t_n - t_b) / 1000 / frame);
	printf("Cue vp\t: %.2f\n", 1.0 * cue_rate[0] / frame * 100);
	printf("Cue focue\t: %.2f\n", 1.0 * cue_rate[1] / frame * 100);
	printf("Cue MLsD\t: %.2f\n", 1.0 * cue_rate[2] / frame * 100);

	fprintf(log, "DSES time \t= %.2lf s\n", 1.0 * (t_n - t_e) / 1000);
	fprintf(log, "total time\t= %.2lf s\n", 1.0 * (t_n - t_b) / 1000);
	fprintf(log, "s/frame   \t= %.2lf s\n\n", 1.0 * (t_n - t_b) / 1000 / frame);
	fprintf(log, "Cue vp   \t: %.2f\n", 1.0 * cue_rate[0] / frame * 100);
	fprintf(log, "Cue focue\t: %.2f\n", 1.0 * cue_rate[1] / frame * 100);
	fprintf(log, "Cue MLsD \t: %.2f\n", 1.0 * cue_rate[2] / frame * 100);

	system("pause");
	printf("Press any key to continue...\n");
	return 0;
}