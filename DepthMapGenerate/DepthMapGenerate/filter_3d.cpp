#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define	full	(width*height)
#define	m_size	5
#define	z_w	2.0
int filter_3d(int width, int height, int f, unsigned char **set_d, unsigned char *output_d)
{
	int i, j, k;
	int x, y, z;

	double w_mask[m_size][m_size][m_size];
	double total_w;
	double total_value;

	double mean = 0;
	double st_dev = 0;
	double dis;

	for (z = 0; z < m_size; z++) {
		for (y = 0; y < m_size; y++) {
			for (x = 0; x < m_size; x++) {
				mean += sqrt(1.0 * z * z + y * y + x * x);
			}
		}
	}
	mean = mean / m_size / m_size / m_size;
	for (z = 0; z < m_size; z++) {
		for (y = 0; y < m_size; y++) {
			for (x = 0; x < m_size; x++) {
				dis = sqrt(1.0 * (z * z_w) * (z * z_w) + y * y + x * x);
				st_dev += (dis - mean) * (dis - mean);
			}
		}
	}
	st_dev = sqrt(st_dev / m_size / m_size / m_size);
	//	printf("mean = %lf\nstd = %lf\n", mean, st_dev);

	for (z = 0; z < m_size; z++) {
		for (y = 0; y < m_size; y++) {
			for (x = 0; x < m_size; x++) {
				dis = sqrt(1.0 * (z * z_w) * (z * z_w) + y * y + x * x);
				w_mask[z][y][x] = exp(-1.0 * dis * dis / (2 * st_dev * st_dev));
				//				printf("(%d,%d,%d)w = %lf\n", z, y, x, w_mask[z][y][x]);
			}
		}
	}

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			total_value = 0;
			total_w = 0;
			for (z = 0; z < m_size; z++) {
				for (y = -m_size + 1; y < m_size; y++) {
					for (x = -m_size + 1; x < m_size; x++) {
						if (z <= f &&
							i + y >= 0 && i + y < height &&
							j + x >= 0 && j + x < width) {
							if (f > 4) {
								dis = 0;
							}
							total_value += 1.0 * set_d[z][(i + y) * width + j + x] * w_mask[z][abs(y)][abs(x)];
							total_w += w_mask[z][abs(y)][abs(x)];
						}
					}
				}
			}
			output_d[i * width + j] = unsigned char(1.0 * total_value / total_w + 0.5);
		}
	}

	return 0;
}