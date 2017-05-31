int sobel_ori(int, int, unsigned char*, unsigned char*, int*);
int vanishing_point_detection(int, int, unsigned char *, int *, int *, int *);
int reliability_calculator(int, int, int, int, unsigned char*, int, float *, float *);
int watershed_segmentation(int, int, int, unsigned char *, int *, unsigned char *); //(threshold, input grad, output label, output label edge)
int focus_depth_estimation(int, int, unsigned char *, int *, int, unsigned char *, unsigned char *);
int edge_depth_estimation(int, int, unsigned char *, int, int *, int, unsigned char *);
int vp_depth_estimation(int, int, int *, int, int, int, unsigned char *);
int scene_change_detection(int, int, int, unsigned char *, unsigned char *, float *, float *);
int depth_difference_limiter(int, int, unsigned char *, unsigned char *, unsigned char *);
int depth_memc(int, int, unsigned char *, unsigned char *, unsigned char *, unsigned char *);
int filter_3d(int, int, int, unsigned char **, unsigned char *);
