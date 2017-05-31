#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define full (width*height)

int sobel(int width, int height, unsigned char *buffer, unsigned char *buffer2);

int focus_depth_estimation(int width, int height, unsigned char *grad, int *label_map, int label_count, unsigned char *edge, unsigned char *result)
{
	int i, j, k, m, n;
	unsigned char temp;

	unsigned char *grad_2;
	grad_2 = new unsigned char[full];

	//fwrite(grad, full, 1, O);
	/*=====Sobel 2=====*/
	sobel(width, height, grad, grad_2);

	//fwrite(grad_2, full, 1, O);
	/*=====Label Information Search=====*/
	int *label_size;
	label_size = new int[label_count];

	int *label_b_rate;
	label_b_rate = new int[label_count];

	for (i = 0; i < label_count; i++) {
		label_size[i] = 0;
		label_b_rate[i] = 0;
	}

	for (i = 0; i < width*height; i++) {
		label_size[label_map[i]]++;
		if (i % width == 0 || i % width == width - 1 || i / width == 0 || i / width == height - 1)
			label_b_rate[label_map[i]]++;
	}
	/*
	for (i = 0; i < full; i++){
	if (label_size[label_map[i]] > 300)
	temp = 255;
	else
	temp = 0;
	fwrite(&temp, 1, 1, O);
	}
	for (i = 0; i < full; i++){
	if (label_b_rate[label_map[i]] > 0.6 * (width + height))
	temp = 255;
	else
	temp = 0;
	fwrite(&temp, 1, 1, O);
	}
	*/
	/*=====label edge strength search=====*/
	int *label_edge;
	label_edge = new int[label_count];
	int *label_edge_count;
	label_edge_count = new int[label_count];

	unsigned char *edge_add_map;
	edge_add_map = new unsigned char[full];

	int m_x, m_y;

	for (i = 0; i < full; i++) {
		edge_add_map[i] = 0;
	}

	for (i = 0; i < label_count; i++) {
		label_edge[i] = 0;
		label_edge_count[i] = 0;
	}

	int m_label_buf[25];
	int m_label_count;
	int m_label_check;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			if (edge[i * width + j] == 255) {
				m_label_count = 0;
				for (m_y = -2; m_y < 2; m_y++) {
					for (m_x = -2; m_x <= 2; m_x++) {
						if (m_y + i >= 0 && m_y + i < height && m_x + j >= 0 && m_x + j < width) {
							if (label_map[(i + m_y) * width + m_x + j] != -1 && edge[(i + m_y) * width + m_x + j] != 255 && label_size[label_map[(i + m_y) * width + m_x + j]] > 300) {
								if (edge_add_map[i] == 0) {
									label_edge[label_map[(i + m_y) * width + m_x + j]] += grad_2[(i + m_y) * width + m_x + j];
									edge_add_map[(i + m_y) * width + m_x + j] = 255;
									label_edge_count[label_map[(i + m_y) * width + m_x + j]]++;
									grad_2[(i + m_y) * width + m_x + j] = 0;
								}
							}
						}
					}
				}
			}
		}
	}
	/*=====depth generation=====*/
	double *label_avg_edge;
	label_avg_edge = new double[label_count];

	double max = 0;
	double min = 255;


	for (i = 0; i < label_count; i++) {
		if (label_edge_count[i] != 0 && label_b_rate[i] < 0.9 * (width + height)) {
			label_avg_edge[i] = 1.0 * label_edge[i] / label_edge_count[i];
			if (label_avg_edge[i] > max)
				max = label_avg_edge[i];
			if (label_avg_edge[i] < min)
				min = label_avg_edge[i];
		}
		else
			label_avg_edge[i] = -1;
		//printf("label %d => %lf\n", i, label_avg_edge[i]);
	}
	printf("%lf, %lf\n", min, max);

	for (i = 0; i < full; i++) {
		temp = unsigned char(1.0 * 255 * label_avg_edge[label_map[i]] / max);
		if (label_avg_edge[label_map[i]] == -1)
			temp = 0;
	}
	/*=====Neighbor Region Search=====*/
	struct q_label_information {
		int q_label_count;
		int q_es_count;
		int q[100];
	};

	q_label_information *q_label;
	q_label = new q_label_information[label_count]; //{ num, not -1 num, label 1, label 2.... }

	int mask_label[9];
	int mask_label_count;
	int mask_label_check;
	int q_counter;

	for (i = 0; i < label_count; i++) {
		q_label[i].q_label_count = 0;
		q_label[i].q_es_count = 0;
	}

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			mask_label_count = 0;
			for (m_y = -1; m_y < 1; m_y++) {
				for (m_x = -1; m_x <= 1; m_x++) {
					if (m_y * m_x == 0) {
						if (m_y + i >= 0 && m_y + i < height && m_x + j >= 0 && m_x + j < width) {
							mask_label_check = 0;
							for (k = 0; k < mask_label_count; k++) {
								if (mask_label[k] == label_map[(i + m_y) * width + m_x + j]) {
									mask_label_check = 1;
									break;
								}
							}
							if (mask_label_check == 0) {
								mask_label[mask_label_count] = label_map[(i + m_y) * width + m_x + j];
								mask_label_count++;
							}
						}
					}
				}
			}
			for (m = 0; m < mask_label_count; m++) {
				for (n = 0; n < mask_label_count; n++) {
					if (n != m) {
						mask_label_check = 0;
						for (k = 0; k < q_label[mask_label[m]].q_label_count; k++) {
							if (q_label[mask_label[m]].q[k] == mask_label[n]) {
								mask_label_check = 1;
								break;
							}
						}
						if (mask_label_check == 0 && q_label[mask_label[m]].q_label_count < 100) {
							q_label[mask_label[m]].q[q_label[mask_label[m]].q_label_count] = mask_label[n];
							q_label[mask_label[m]].q_label_count++;
						}
					}
				}
			}
		}
	}
	/*=====Small Region ES Filling=====*/
	double *sum_es;
	sum_es = new double[label_count];
	for (;;) {
		int max_q_es_num = 0;
		//updata q_es number
		for (i = 0; i < label_count; i++) {
			q_label[i].q_es_count = 0;
			if (label_b_rate[i] < 0.6 * (width + height)) {
				for (j = 0; j < q_label[i].q_label_count; j++) {
					if (label_avg_edge[q_label[i].q[j]] != -1) {
						q_label[i].q_es_count++;
					}
				}
			}
			if (label_avg_edge[i] == -1)
				if (max_q_es_num < q_label[i].q_es_count)
					max_q_es_num = q_label[i].q_es_count;
		}

		if (max_q_es_num == 0)
			break;
		//Filling the small region with max_q_es_num
		for (i = 0; i < label_count; i++) {
			sum_es[i] = -1;
			if (q_label[i].q_es_count == max_q_es_num && label_avg_edge[i] == -1) {
				for (j = 0; j < q_label[i].q_label_count; j++) {
					if (label_avg_edge[q_label[i].q[j]] != -1) {
						if (sum_es[i] == -1)
							sum_es[i] += label_avg_edge[q_label[i].q[j]] + 1;
						else
							sum_es[i] += label_avg_edge[q_label[i].q[j]];
					}
				}
			}
		}
		for (i = 0; i < label_count; i++) {
			if (sum_es[i] != -1) {
				label_avg_edge[i] = 1.0 * sum_es[i] / q_label[i].q_es_count;
			}
		}

		for (i = 0; i < full; i++) {
			temp = unsigned char(1.0 * 255 * label_avg_edge[label_map[i]] / max);
			if (label_avg_edge[label_map[i]] == -1)
				temp = 0;
		}
	}
	//Filling the last small regions' ES with 0, only background has ES value, -1.
	for (i = 0; i < full; i++) {
		if (label_avg_edge[label_map[i]] == -1 && label_b_rate[label_map[i]] < 0.6 * (width + height)) {
			label_avg_edge[label_map[i]] = 0;
		}
	}
	//depth output
	for (i = 0; i < full; i++) {
		if (label_avg_edge[label_map[i]] == -1)
			temp = 0;
		else
			temp = unsigned char(1.0 * 255 * label_avg_edge[label_map[i]] / max);
	}
	/*=====ES Smooth=====*/
	double *label_avg_edge_temp;
	label_avg_edge_temp = new double[label_count];

	for (k = 0; k < 10; k++) {
		//updata q_es number for every region
		for (i = 0; i < label_count; i++) {
			q_label[i].q_es_count = 0;
			label_avg_edge_temp[i] = -1;
			for (j = 0; j < q_label[i].q_label_count; j++) {
				if (label_avg_edge[q_label[i].q[j]] != -1) {
					q_label[i].q_es_count++;
				}
			}
		}
		//ES sharing from NB regions
		int		large_check;
		double	ES_avg;
		double	var;
		for (i = 0; i < label_count; i++) {
			if (label_avg_edge[i] != -1) {
				large_check = 1;
				ES_avg = 0;
				var = 0;
				if (q_label[i].q_es_count != 0) {
					for (j = 0; j < q_label[i].q_label_count; j++) {
						if (label_avg_edge[q_label[i].q[j]] != -1) {
							if (label_avg_edge[q_label[i].q[j]] < label_avg_edge[i])
								large_check = 0;
							ES_avg += label_avg_edge[q_label[i].q[j]];
						}
					}
					ES_avg = ES_avg / q_label[i].q_es_count;
					for (j = 0; j < q_label[i].q_label_count; j++) {
						if (label_avg_edge[q_label[i].q[j]] != -1)
							var += (label_avg_edge[q_label[i].q[j]] - ES_avg) * (label_avg_edge[q_label[i].q[j]] - ES_avg);
					}
					var = var / q_label[i].q_es_count;
					label_avg_edge_temp[i] = ES_avg;
				}
			}
		}
		for (i = 0; i < label_count; i++)
			if (label_avg_edge_temp[i] != -1)
				label_avg_edge[i] = label_avg_edge_temp[i];
		float final_max = 0;
		for (i = 0; i < full; i++)
			if (label_avg_edge[label_map[i]] > final_max)
				final_max = label_avg_edge[label_map[i]];
		for (i = 0; i < full; i++) {
			result[i] = unsigned char(1.0 * 255 * (label_avg_edge[label_map[i]]) / final_max);
			if (label_avg_edge[label_map[i]] == -1) {
				result[i] = 0;
			}
		}
	}

	delete(grad_2);
	delete(label_size);
	delete(label_b_rate);
	delete(label_edge);
	delete(label_edge_count);
	delete(edge_add_map);
	delete(label_avg_edge);
	delete(q_label);
	delete(sum_es);
	delete(label_avg_edge_temp);
	return 0;
}

int sobel(int width, int height, unsigned char *buffer, unsigned char *buffer2)
{
	int i;
	int Gx;
	int Gy;
	int total = 0;
	int total_count = 0;
	float G, G_u, G_v;
	float max, min;
	float *result;
	result = new float[full];
	max = -1;
	min = sqrt(1.0 * 255 * 4 * 255 * 4 * 2);

	for (i = 0; i < width*height; i++)
	{
		if (i%width == 0 || i%width == width - 1 || i / width == 0 || i / width == height - 1) {
		}
		else {
			Gx = (-1) * buffer[i - width - 1] + 1 * buffer[i - width + 1] + (-2) * buffer[i - 1] + 2 * buffer[i + 1]
				+ (-1) * buffer[i + width - 1] + 1 * buffer[i + width + 1];
			Gy = (-1) * buffer[i - width - 1] + 1 * buffer[i + width - 1] + (-2) * buffer[i - width]
				+ 2 * buffer[i + width] + (-1) * buffer[i - width + 1] + 1 * buffer[i + width + 1];
			G = Gx * Gx + Gy * Gy;
			G = sqrt(G);
			if (G < min)
				min = G;
			if (G > max)
				max = G;
			result[i] = G;
		}

	}

	for (i = 0; i < width*height; i++)
	{
		if (i%width == 0 || i%width == width - 1 || i / width == 0 || i / width == height - 1) {
			buffer2[i] = 0;
		}
		else {
			buffer2[i] = unsigned char(1.0 * 255 * (result[i] - min) / (max - min));
		}
		if (buffer2[i] != 0) {
			total = total + buffer2[i];
			total_count++;
		}
	}

	total = int(1.0 * total / total_count + 0.5);

	delete(result);
	return total;
}