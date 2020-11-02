#ifndef SYNC_DATA_H
#define SYNC_DATA_H

enum sync_tracks {
	SYNC_TRACK_foo = 0,
	SYNC_TRACK_frameLimit = 1,
	SYNC_TRACK_frameShape = 2,
	SYNC_TRACK_frameSharpness = 3,
	SYNC_TRACK_strp_cnt = 4,
	SYNC_TRACK_strp_trnsinst = 5,
	SYNC_TRACK_tv_artifacts = 6,
	SYNC_TRACK_toblack = 7,
	SYNC_TRACK_noisemix = 8,
	SYNC_TRACK_COUNT = 9
};

static const int sync_data_offset[SYNC_TRACK_COUNT] = {
	0, /* track: foo */
	0, /* track: frameLimit */
	1, /* track: frameShape */
	3, /* track: frameSharpness */
	4, /* track: strp_cnt */
	7, /* track: strp_trnsinst */
	10, /* track: tv_artifacts */
	14, /* track: toblack */
	18, /* track: noisemix */
};

static const int sync_data_count[SYNC_TRACK_COUNT] = {
	0, /* track: foo */
	1, /* track: frameLimit */
	2, /* track: frameShape */
	1, /* track: frameSharpness */
	3, /* track: strp_cnt */
	3, /* track: strp_trnsinst */
	4, /* track: tv_artifacts */
	4, /* track: toblack */
	5, /* track: noisemix */
};

static const int sync_data_rows[] = {
	/* track: foo */
	/* track: frameLimit */
	0,
	/* track: frameShape */
	0,
	56,
	/* track: frameSharpness */
	0,
	/* track: strp_cnt */
	0,
	536,
	577,
	/* track: strp_trnsinst */
	0,
	536,
	577,
	/* track: tv_artifacts */
	0,
	32,
	56,
	536,
	/* track: toblack */
	0,
	32,
	536,
	577,
	/* track: noisemix */
	0,
	32,
	536,
	576,
	577,
};

static const float sync_data_values[] = {
	/* track: foo */
	/* track: frameLimit */
	0.300000,
	/* track: frameShape */
	30.000000,
	0.500000,
	/* track: frameSharpness */
	3.000000,
	/* track: strp_cnt */
	0.000000,
	256.000000,
	8.000000,
	/* track: strp_trnsinst */
	0.000000,
	0.300000,
	1.000000,
	/* track: tv_artifacts */
	1.000000,
	0.000000,
	0.000000,
	3.000000,
	/* track: toblack */
	1.000000,
	0.000000,
	0.000000,
	1.000000,
	/* track: noisemix */
	0.400000,
	0.200000,
	0.200000,
	0.800000,
	0.000000,
};

static const unsigned char sync_data_type[] = {
	/* track: foo */
	/* track: frameLimit */
	0,
	/* track: frameShape */
	2,
	0,
	/* track: frameSharpness */
	0,
	/* track: strp_cnt */
	0,
	2,
	0,
	/* track: strp_trnsinst */
	0,
	2,
	0,
	/* track: tv_artifacts */
	0,
	0,
	0,
	0,
	/* track: toblack */
	2,
	0,
	2,
	0,
	/* track: noisemix */
	2,
	0,
	2,
	0,
	0,
};

#endif /* !defined(SYNC_DATA_H) */
