#include <stdio.h>
#include <stdlib.h>
#include <cmath>

int sobel_ori(int w, int h, unsigned char *p, unsigned char *p2, int *Ori)	//sobel(Ori, Y data, w, h, threshold)
{
	int i;
	float Gx;
	float Gy;
	float G;
	int avg = 0;
	int avg_count = 0;
	int max = 0;

	int* G_map;
	G_map = new int[w * h];

	for (i = 0; i < w * h; i++)
	{
		if (i%w == 0 || i%w == w - 1 || i / w == 0 || i / w == h - 1) {
			p2[i] = 0;
			Ori[i] = 18;
		}
		else {
			Gx = (-1) * p[i - w - 1] + 1 * p[i - w + 1] + (-2) * p[i - 1] + 2 * p[i + 1]
				+ (-1) * p[i + w - 1] + 1 * p[i + w + 1];
			Gy = (-1) * p[i - w - 1] + 1 * p[i + w - 1] + (-2) * p[i - w]
				+ 2 * p[i + w] + (-1) * p[i - w + 1] + 1 * p[i + w + 1];
			G = Gx * Gx + Gy * Gy;
			G = sqrt(G);
			if (G > max)
				max = G;
			G_map[i] = G;
			if (Gx == 0)
				Ori[i] = 8;
			else if (Gy / Gx >= 0) {
				if (Gy / Gx < 0.17)
					Ori[i] = 0;
				else if (Gy / Gx < 0.36)
					Ori[i] = 1;
				else if (Gy / Gx < 0.58)
					Ori[i] = 2;
				else if (Gy / Gx < 0.84)
					Ori[i] = 3;
				else if (Gy / Gx < 1.19)
					Ori[i] = 4;
				else if (Gy / Gx < 1.73)
					Ori[i] = 5;
				else if (Gy / Gx < 2.75)
					Ori[i] = 6;
				else if (Gy / Gx < 5.67)
					Ori[i] = 7;
				else
					Ori[i] = 8;

			}
			else {
				if (Gy / Gx < -5.67)
					Ori[i] = 9;
				else if (Gy / Gx < -2.75)
					Ori[i] = 10;
				else if (Gy / Gx < -1.73)
					Ori[i] = 11;
				else if (Gy / Gx < -1.19)
					Ori[i] = 12;
				else if (Gy / Gx < -0.84)
					Ori[i] = 13;
				else if (Gy / Gx < -0.58)
					Ori[i] = 14;
				else if (Gy / Gx < -0.36)
					Ori[i] = 15;
				else if (Gy / Gx < -0.17)
					Ori[i] = 16;
				else
					Ori[i] = 17;
			}
		}
	}


	for (i = 0; i < w * h; i++) {
		p2[i] = unsigned char(1.0 * G_map[i] * 255 / max);
		if (p2[i] >= 0) {
			avg += p2[i];
			avg_count++;
		}
	}

	avg = int(1.0 * avg / avg_count + 0.5);

	printf("avg = %d\n", avg);
	for (i = 0; i < w * h; i++)
		if (p2[i] < avg)
			Ori[i] = 18;
	delete(G_map);
	return avg;
}