#include <stdio.h>
#include <stdlib.h>
#include <cmath>
int watershed_segmentation(int width, int height, int threshold, unsigned char *buffer2, int *label, unsigned char *edge) //(threshold, input grad, output label, output label edge)
{
	int i, j, k;
	//sorting
	struct pixel
	{
		int y;				// y value
		int pos;			// position
	};

	struct pixel* Sorting;

	Sorting = (struct pixel *) malloc(width * height * sizeof(struct pixel));

	/*=====Put pixel back and label=====*/
	int *label_list;
	label_list = (int *)malloc(width * height * sizeof(int));

	/*=====Sorting=====*/
	//printf("Sorting.....");

	int S_Count = 0;
	int Y_Count;

	for (Y_Count = 0; Y_Count < 256; Y_Count++)
		for (i = 0; i < width*height; i++) {
			int a = buffer2[i];
			if (a == Y_Count) {
				Sorting[S_Count].y = Y_Count;
				Sorting[S_Count].pos = i;
				S_Count++;
			}
		}
	//printf("Done\n");
	/*=====Put pixel back and label=====*/
	//	printf("Put pixel back and label.....");

	for (i = 0; i < width * height; i++)
		edge[i] = 0;

	unsigned int label_count = 1;
	int mask_label;

	int label_check;
	int label_buffer[8];
	int label_min;

	for (i = 0; i < width*height; i++) {
		label[i] = -1;
		label_list[i] = i;
	}

	for (i = 0; i < width*height; i++) {
		if (Sorting[i].pos%width != 0 && Sorting[i].pos%width != width - 1 && Sorting[i].pos / width != 0 && Sorting[i].pos / width != height - 1) {
			label_min = -1;
			label_check = 0;
			mask_label = -1;
			// Check 3*3 mask label;
			if (label[Sorting[i].pos - width - 1] != -1 && label_list[label[Sorting[i].pos - width - 1]] != mask_label) {
				mask_label = label_list[label[Sorting[i].pos - width - 1]];
				if (label_min == -1 || label_min > mask_label)
					label_min = mask_label;
				label_buffer[label_check] = mask_label;
				label_check++;
			}
			if (label[Sorting[i].pos - width] != -1 && label_list[label[Sorting[i].pos - width]] != mask_label) {
				mask_label = label_list[label[Sorting[i].pos - width]];
				if (label_min == -1 || label_min > mask_label)
					label_min = mask_label;
				label_buffer[label_check] = mask_label;
				label_check++;
			}
			if (label[Sorting[i].pos - width + 1] != -1 && label_list[label[Sorting[i].pos - width + 1]] != mask_label) {
				mask_label = label_list[label[Sorting[i].pos - width + 1]];
				if (label_min == -1 || label_min > mask_label)
					label_min = mask_label;
				label_buffer[label_check] = mask_label;
				label_check++;
			}
			if (label[Sorting[i].pos - 1] != -1 && label_list[label[Sorting[i].pos - 1]] != mask_label) {
				mask_label = label_list[label[Sorting[i].pos - 1]];
				if (label_min == -1 || label_min > mask_label)
					label_min = mask_label;
				label_buffer[label_check] = mask_label;
				label_check++;
			}
			if (label[Sorting[i].pos + 1] != -1 && label_list[label[Sorting[i].pos + 1]] != mask_label) {
				mask_label = label_list[label[Sorting[i].pos + 1]];
				if (label_min == -1 || label_min > mask_label)
					label_min = mask_label;
				label_buffer[label_check] = mask_label;
				label_check++;
			}
			if (label[Sorting[i].pos + width - 1] != -1 && label_list[label[Sorting[i].pos + width - 1]] != mask_label) {
				mask_label = label_list[label[Sorting[i].pos + width - 1]];
				if (label_min == -1 || label_min > mask_label)
					label_min = mask_label;
				label_buffer[label_check] = mask_label;
				label_check++;
			}
			if (label[Sorting[i].pos + width] != -1 && label_list[label[Sorting[i].pos + width]] != mask_label) {
				mask_label = label_list[label[Sorting[i].pos + width]];
				if (label_min == -1 || label_min > mask_label)
					label_min = mask_label;
				label_buffer[label_check] = mask_label;
				label_check++;
			}
			if (label[Sorting[i].pos + width + 1] != -1 && label_list[label[Sorting[i].pos + width + 1]] != mask_label) {
				mask_label = label_list[label[Sorting[i].pos + width + 1]];
				if (label_min == -1 || label_min > mask_label)
					label_min = mask_label;
				label_buffer[label_check] = mask_label;
				label_check++;
			}

			// Label
			if (label_check == 0) {								// No label around pos i
				label[Sorting[i].pos] = label_count;
				label_count++;
			}
			else if (label_check == 1) {							// 1 label around pos i
				label[Sorting[i].pos] = label_list[label_min];
			}
			else {												// 2+ label around pos i
				if (Sorting[i].y < threshold) {					// Y is lower than threshold // th_map[i]
					for (j = 0; j < label_check; j++) {
						for (k = 0; k < label_count; k++) {
							if (label_list[k] == label_buffer[j])
								label_list[k] = label_min;
						}
					}
					label[Sorting[i].pos] = label_min;
				}
				else {
					label[Sorting[i].pos] = label_min;
					edge[Sorting[i].pos] = 255;
				}
			}
		}
		else {
			edge[Sorting[i].pos] = 255;
		}
	}
	//printf("Done\n");
	/*=====label path=====*/
	//	printf("label updataing.....");
	for (i = 0; i < width*height; i++)
	{
		if (label[i] != -1) {
			label[i] = label_list[label[i]];
		}
		else {
			label[i] = -1;
		}
	}
	label[0] = label[width + 1];
	label[width - 1] = label[width - 1 + width - 1];
	label[width * height - 1] = label[width * height - 1 - width - 1];
	label[width * (height - 1)] = label[width * (height - 1) - width + 1];
	for (i = 1; i < width - 1; i++) {
		label[i] = label[i + width];
		label[width * (height - 1) + i] = label[width * (height - 1) + i - width];
	}

	for (i = 1; i < height - 1; i++) {
		label[i * width] = label[i * width + 1];
		label[i * width + width - 1] = label[i * width + width - 2];
	}

	//	printf("Done\n");
	/*=====label restack=====*/
	int new_label_count = 0;
	for (i = 0; i < label_count; i++) {
		label_list[i] = -1;
	}
	for (i = 0; i < width*height; i++)
	{
		if (label_list[label[i]] == -1) {
			label_list[label[i]] = new_label_count;
			new_label_count++;
		}
		label[i] = label_list[label[i]];
	}

	delete(Sorting);
	delete(label_list);

	return new_label_count;
}