#include <iostream>
#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <cstring>
#include <vector>

#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

const int   SIZEINT   = (int) sizeof(int);
const int   SIZEFLOAT = (int) sizeof(float);
const float MIN_DIST  = 0.00005F;
const float MAX_DIST  = 0.1f;

timeval g_start_time;
timeval g_end_time;

// -----------------------------------------------------------------------------
void create_dir(					// create directory
	char *path)							// input path
{
	int len = (int) strlen(path);
	for (int i = 0; i < len; ++i) {
		if (path[i] == '/') {
			char ch = path[i + 1];
			path[i + 1] = '\0';

			int ret = access(path, F_OK);
			if (ret != 0) {
				ret = mkdir(path, 0755);
				if (ret != 0) printf("Could not create %s\n", path);
			}
			path[i + 1] = ch;
		}
	}
}

// -----------------------------------------------------------------------------
int read_bin_data(					// read bin data from disk
	const char *fname,					// address of data set
	std::vector<float> &min_list,		// min list (return)
	std::vector<float> &max_list,		// max list (return)
	std::vector<std::vector<float> > &data)	// data (return)
{
    gettimeofday(&g_start_time, NULL);
	int   n   = (int) data.size();
    int   d   = (int) data[0].size();
	float val = -1.0f;

	// -------------------------------------------------------------------------
	//  read bin data
	// -------------------------------------------------------------------------
	FILE *fp = fopen(fname, "rb");
	if (!fp) { printf("Could not open %s\n", fname); return 1; }

	int i = 0;
	while (!feof(fp) && i < n) {
		fread(data[i++].data(), SIZEFLOAT, d, fp);
	}
	fclose(fp);

	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < d; ++j) {
			if (i == 0 || data[i][j] < min_list[j]) min_list[j] = data[i][j];
			if (i == 0 || data[i][j] > max_list[j]) max_list[j] = data[i][j];
		}
	}
	for (int j = 0; j < d; ++j) {
		std::cout << "min[" << j << "] = " << min_list[j] << ", "
		          << "max[" << j << "] = " << max_list[j] << std::endl;
	}

	// -------------------------------------------------------------------------
	//  shift data by the position of center
	// -------------------------------------------------------------------------
	std::vector<float> center(d, 0.0f);
	for (int i = 0; i < d; ++i) {
		center[i] = (min_list[i] + max_list[i]) / 2.0f;
	}

	float max_norm = -1.0f;
	for (int i = 0; i < n; ++i) {
		float norm = 0.0f;
		for (int j = 0; j < d; ++j) {
			val = data[i][j] - center[j];
			data[i][j] = val; norm += val * val;
		}
		norm = sqrt(norm);
		if (max_norm < norm) max_norm = norm;
	}

	// -------------------------------------------------------------------------
	//  max normalization: scale the data by the max l2-norm
	// -------------------------------------------------------------------------
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < d; ++j) {
			val = data[i][j] / max_norm;

			data[i][j] = val;
			if (i == 0 || val < min_list[j]) min_list[j] = val;
			if (i == 0 || val > max_list[j]) max_list[j] = val;
		}
	}
	gettimeofday(&g_end_time, NULL);
	float running_time = g_end_time.tv_sec - g_start_time.tv_sec + 
		(g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0f;
	printf("Read Data: %f Seconds\n", running_time);

	return 0;
}

// -----------------------------------------------------------------------------
int write_bin_data(                // write binary data to disk
    bool sign,                          // data or query
    char *fname,                        // output file name
    const std::vector<std::vector<float> > &data) // output data
{
    gettimeofday(&g_start_time, NULL);
    int n = (int) data.size();
    int d = (int) data[0].size();
    
    // -------------------------------------------------------------------------
	//  write binary data
	// -------------------------------------------------------------------------
    FILE *fp = fopen(fname, "wb");
	if (!fp) { printf("Could not open %s\n", fname); return 1; }
	for (int i = 0; i < n; ++i) {
		fwrite(data[i].data(), SIZEFLOAT, d, fp);
	}
	fclose(fp);

    gettimeofday(&g_end_time, NULL);
	float running_time = g_end_time.tv_sec - g_start_time.tv_sec + 
		(g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0f;

	if (sign) printf("Write Bin Data: %f Seconds\n", running_time);
    else printf("Write Bin Query: %f Seconds\n", running_time);

    return 0;
}

// -----------------------------------------------------------------------------
int write_txt_data(                // write text data to disk
    bool sign,                          // data or query
    char *fname,                        // output file name
    const std::vector<std::vector<float> > &data) // output data
{
    gettimeofday(&g_start_time, NULL);
    int n = (int) data.size();
    int d = (int) data[0].size();
    
    // -------------------------------------------------------------------------
	//  write text data
	// -------------------------------------------------------------------------
    std::ofstream fout;
	fout.open(fname, std::ios::trunc);
	for (int i = 0; i < n; ++i) {
		fout << i+1;
		for (int j = 0; j < d; ++j) fout << " " << data[i][j];
		fout << "\n";
	}
	fout.close();

    gettimeofday(&g_end_time, NULL);
	float running_time = g_end_time.tv_sec - g_start_time.tv_sec + 
		(g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0f;
	
	if (sign) printf("Write Text Data: %f Seconds\n", running_time);
    else printf("Write Text Query: %f Seconds\n", running_time);

    return 0;
}

// -----------------------------------------------------------------------------
float uniform(						// r.v. from Uniform(min, max)
	float min,							// min value
	float max)							// max value
{
	int   num  = rand();
	float base = (float) RAND_MAX - 1.0F;
	float frac = ((float) num) / base;

	return (max - min) * frac + min;
}

// -----------------------------------------------------------------------------
float calc_dot_plane(				// calc dist from a point to a hyperplane
	int   d,							// dimension of input data object
	const std::vector<float> &data,		// input data object
	const std::vector<float> &query)	// input query
{
	float ip = 0.0f, norm = 0.0f;
	for (int i = 0; i < d; ++i) {
		ip   += data[i]  * query[i];
		norm += query[i] * query[i];
	}
	ip += query[d];

	return fabs(ip) / sqrt(norm);
}

// -----------------------------------------------------------------------------
void generate_query(                // generate query
	const char* fname,					// file name
    const std::vector<float> &min_list,	// min value in d-dimensions
    const std::vector<float> &max_list,	// max value in d-dimensions
    const std::vector<std::vector<float> > &data, // input data
    std::vector<std::vector<float> > &query) // query (return)
{
	gettimeofday(&g_start_time, NULL);
    int n  = (int) data.size();
    int d  = (int) data[0].size();
    int qn = (int) query.size();

    // -------------------------------------------------------------------------
    //  generate query set
    // -------------------------------------------------------------------------
	float ip, val, rnd, dist, min_dist;
    for (int i = 0; i < qn; ++i) {
		do {
			std::cout << fname << " " << i+1 << ": "; 
			ip = 0.0f;
			for (int j = 0; j < d; ++j) {
				val = uniform(min_list[j], max_list[j]);
				rnd = uniform(min_list[j], max_list[j]);
				
				query[i][j] = val; ip += val * rnd;
			}
			query[i][d] = -ip;

			min_dist = -1.0f;
			for (int j = 0; j < n; ++j) {
				dist = calc_dot_plane(d, data[j], query[i]);
				if (min_dist < 0 || dist < min_dist) min_dist = dist;
			}
			std::cout << "min_dist = " << min_dist << ", ip = " << ip << std::endl;
		} while (min_dist < MIN_DIST || min_dist > MAX_DIST);
	}
    gettimeofday(&g_end_time, NULL);
	float running_time = g_end_time.tv_sec - g_start_time.tv_sec + 
		(g_end_time.tv_usec - g_start_time.tv_usec) / 1000000.0f;

	printf("Generate Query: %f Seconds\n", running_time);
}

// -----------------------------------------------------------------------------
int main(int nargs, char** args)
{
	// -------------------------------------------------------------------------
	//  read parameters
	// -------------------------------------------------------------------------
	srand(666);                    	// set up random seed 
	int  n  = atoi(args[1]);     	// cardinality
	int  d  = atoi(args[2]);     	// dimensionality
    int  qn = atoi(args[3]);		// number of queries

	char input_file[200];
	char bin_folder[200];
    char txt_folder[200];

	strncpy(input_file, args[4], sizeof(input_file));
	strncpy(bin_folder, args[5], sizeof(bin_folder));
	strncpy(txt_folder, args[6], sizeof(txt_folder));
	
	create_dir(bin_folder); 
	create_dir(txt_folder); 

	printf("n          = %d\n", n);
	printf("d          = %d\n", d);
	printf("qn         = %d\n", qn);
	printf("input file = %s\n", input_file);
	printf("bin folder = %s\n", bin_folder);	
	printf("txt folder = %s\n", txt_folder);
	printf("\n");

	char bin_fname[200];
	char txt_fname[200];

	std::vector<float> min_list(d, -1.0f);
	std::vector<float> max_list(d, -1.0f);
    std::vector<std::vector<float> > data(n, std::vector<float>(d));

    // -------------------------------------------------------------------------
    //  read dataset
    // -------------------------------------------------------------------------
	if (read_bin_data(input_file, min_list, max_list, data) == 1) return 0;

    // -------------------------------------------------------------------------
    //  write data to disk
    // -------------------------------------------------------------------------
    strcpy(bin_fname, bin_folder); strcat(bin_fname, ".ds");
    write_bin_data(true, bin_fname, data);
    
    strcpy(txt_fname, txt_folder); strcat(txt_fname, ".ds");
    write_txt_data(true, txt_fname, data);

    // -------------------------------------------------------------------------
    //  generate query
    // -------------------------------------------------------------------------
    std::vector<std::vector<float> > query(qn, std::vector<float>(d + 1));
    generate_query(input_file, min_list, max_list, data, query);

    // -------------------------------------------------------------------------
    //  write query to disk
    // -------------------------------------------------------------------------
    strcpy(bin_fname, bin_folder); strcat(bin_fname, ".q");
    write_bin_data(false, bin_fname, query);
    
    strcpy(txt_fname, txt_folder); strcat(txt_fname, ".q");
    write_txt_data(false, txt_fname, query);
	printf("\n");

    return 0;
}
