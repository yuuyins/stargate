#include <math.h>

#include "audiodsp/lib/amp.h"
#include "audiodsp/lib/interpolate-linear.h"
#include "audiodsp/lib/lmalloc.h"


/* SGFLT f_db_to_linear(SGFLT a_db)
 *
 * Convert decibels to linear amplitude
 */
SGFLT f_db_to_linear(SGFLT a_db)
{
    return pow(10.0f, (0.05f * a_db));
}

/* SGFLT f_linear_to_db(SGFLT a_linear)
 *
 * Convert linear amplitude to decibels
 */
SGFLT f_linear_to_db(SGFLT a_linear)
{
    return 20.0f * log10(a_linear);
}


/*Arrays*/

#define arr_amp_db2a_count 545
#define arr_amp_db2a_count_m1_f 544.0f

SG_THREAD_LOCAL SGFLT arr_amp_db2a[arr_amp_db2a_count] = {
0.000010, 0.000010, 0.000011, 0.000011, 0.000011, 0.000012, 0.000012, 0.000012,
0.000013, 0.000013, 0.000013, 0.000014, 0.000014, 0.000015, 0.000015, 0.000015,
0.000016, 0.000016, 0.000017, 0.000017, 0.000018, 0.000018, 0.000019, 0.000019,
0.000020, 0.000021, 0.000021, 0.000022, 0.000022, 0.000023, 0.000024, 0.000024,
0.000025, 0.000026, 0.000027, 0.000027, 0.000028, 0.000029, 0.000030, 0.000031,
0.000032, 0.000033, 0.000033, 0.000034, 0.000035, 0.000037, 0.000038, 0.000039,
0.000040, 0.000041, 0.000042, 0.000043, 0.000045, 0.000046, 0.000047, 0.000049,
0.000050, 0.000052, 0.000053, 0.000055, 0.000056, 0.000058, 0.000060, 0.000061,
0.000063, 0.000065, 0.000067, 0.000069, 0.000071, 0.000073, 0.000075, 0.000077,
0.000079, 0.000082, 0.000084, 0.000087, 0.000089, 0.000092, 0.000094, 0.000097,
0.000100, 0.000103, 0.000106, 0.000109, 0.000112, 0.000115, 0.000119, 0.000122,
0.000126, 0.000130, 0.000133, 0.000137, 0.000141, 0.000145, 0.000150, 0.000154,
0.000158, 0.000163, 0.000168, 0.000173, 0.000178, 0.000183, 0.000188, 0.000194,
0.000200, 0.000205, 0.000211, 0.000218, 0.000224,  0.000230, 0.000237,
0.000244, 0.000251, 0.000259, 0.000266, 0.000274, 0.000282, 0.000290, 0.000299,
0.000307, 0.000316, 0.000325, 0.000335, 0.000345, 0.000355, 0.000365, 0.000376,
0.000387, 0.000398, 0.000410, 0.000422, 0.000434, 0.000447, 0.000460, 0.000473,
0.000487, 0.000501, 0.000516, 0.000531, 0.000546, 0.000562, 0.000579, 0.000596,
0.000613, 0.000631, 0.000649, 0.000668, 0.000688, 0.000708, 0.000729, 0.000750,
0.000772, 0.000794, 0.000818, 0.000841, 0.000866, 0.000891, 0.000917, 0.000944,
0.000972, 0.001000, 0.001029, 0.001059, 0.001090, 0.001122, 0.001155, 0.001189,
0.001223, 0.001259, 0.001296, 0.001334, 0.001372, 0.001413, 0.001454, 0.001496,
0.001540, 0.001585, 0.001631, 0.001679, 0.001728, 0.001778, 0.001830, 0.001884,
0.001939, 0.001995, 0.002054, 0.002113, 0.002175, 0.002239, 0.002304, 0.002371,
0.002441, 0.002512, 0.002585, 0.002661, 0.002738, 0.002818,  0.002901,
0.002985, 0.003073, 0.003162, 0.003255, 0.003350, 0.003447, 0.003548, 0.003652,
0.003758, 0.003868, 0.003981, 0.004097, 0.004217, 0.004340, 0.004467, 0.004597,
0.004732, 0.004870, 0.005012, 0.005158, 0.005309, 0.005464, 0.005623, 0.005788,
0.005957, 0.006131, 0.006310, 0.006494, 0.006683, 0.006879, 0.007079, 0.007286,
0.007499, 0.007718, 0.007943, 0.008175, 0.008414, 0.008660, 0.008913, 0.009173,
0.009441, 0.009716, 0.010000, 0.010292, 0.010593, 0.010902, 0.011220, 0.011548,
0.011885, 0.012232, 0.012589, 0.012957, 0.013335, 0.013725, 0.014125, 0.014538,
0.014962, 0.015399, 0.015849, 0.016312, 0.016788, 0.017278, 0.017783, 0.018302,
0.018836, 0.019387, 0.019953, 0.020535, 0.021135, 0.021752, 0.022387, 0.023041,
0.023714, 0.024406, 0.025119, 0.025852, 0.026607, 0.027384, 0.028184, 0.029007,
0.029854, 0.030726, 0.031623, 0.032546, 0.033497, 0.034475, 0.035481, 0.036517,
0.037584, 0.038681, 0.039811, 0.040973, 0.042170, 0.043401, 0.044668, 0.045973,
0.047315, 0.048697, 0.050119, 0.051582, 0.053088, 0.054639, 0.056234, 0.057876,
0.059566, 0.061306, 0.063096, 0.064938, 0.066834, 0.068786, 0.070795, 0.072862,
0.074989, 0.077179, 0.079433, 0.081752, 0.084140, 0.086596, 0.089125, 0.091728,
0.094406, 0.097163, 0.100000, 0.102920, 0.105925, 0.109018, 0.112202, 0.115478,
0.118850, 0.122321, 0.125893, 0.129569, 0.133352, 0.137246, 0.141254, 0.145378,
0.149624, 0.153993, 0.158489, 0.163117, 0.167880, 0.172783, 0.177828, 0.183021,
0.188365, 0.193865, 0.199526, 0.205353, 0.211349, 0.217520, 0.223872, 0.230409,
0.237137, 0.244062, 0.251189, 0.258523, 0.266073, 0.273842, 0.281838, 0.290068,
0.298538, 0.307256, 0.316228, 0.325462, 0.334965, 0.344747, 0.354813,
0.365174, 0.375837, 0.386812, 0.398107, 0.409732, 0.421697, 0.434010, 0.446684,
0.459727, 0.473151, 0.486968, 0.501187, 0.515822, 0.530884, 0.546387, 0.562341,
0.578762, 0.595662, 0.613056, 0.630957, 0.649382, 0.668344, 0.687860, 0.707946,
0.728618, 0.749894, 0.771792, 0.794328, 0.817523, 0.841395, 0.865964, 0.891251,
0.917276, 0.944061, 0.971628, 1.000000, 1.029201, 1.059254, 1.090184, 1.122018,
1.154782, 1.188502, 1.223207, 1.258925,  1.295687, 1.333521, 1.372461,
1.412538, 1.453784, 1.496236, 1.539927, 1.584893, 1.631173, 1.678804, 1.727826,
1.778279, 1.830206, 1.883649, 1.938653, 1.995262, 2.053525, 2.113489, 2.175204,
2.238721, 2.304093, 2.371374, 2.440619, 2.511886, 2.585235, 2.660725, 2.738420,
2.818383, 2.900681, 2.985383, 3.072557,  3.162278, 3.254618, 3.349654,
3.447466, 3.548134, 3.651741, 3.758374, 3.868120, 3.981072, 4.097321, 4.216965,
4.340103, 4.466836, 4.597270, 4.731513, 4.869675, 5.011872, 5.158222, 5.308845,
5.463865, 5.623413, 5.787620, 5.956622, 6.130558, 6.309574, 6.493816, 6.683439,
6.878599, 7.079458,  7.286182, 7.498942,  7.717915, 7.943282, 8.175230,
8.413952, 8.659643, 8.912509, 9.172759, 9.440609,  9.716280, 10.000000,
10.292006, 10.592537, 10.901845, 11.220184, 11.547820, 11.885022, 12.232071,
12.589254, 12.956867, 13.335215, 13.724609, 14.125376, 14.537844, 14.962357,
15.399265, 15.848932, 16.311729, 16.788040, 17.278259, 17.782795, 18.302061,
18.836491, 19.386526, 19.952623, 20.535250,  21.134890, 21.752041, 22.387211,
23.040930, 23.713737, 24.406191, 25.118864, 25.852348, 26.607250, 27.384197,
28.183830, 29.006811,  29.853827, 30.725574, 31.622776, 32.546177, 33.496544,
34.474659, 35.481339, 36.517414, 37.583740, 38.681206, 39.810719, 40.973209,
42.169651, 43.401028, 44.668358, 45.972698, 47.315125, 48.696751, 50.118725,
51.582218, 53.088444, 54.638657, 56.234131, 57.876198, 59.566216, 61.305580,
63.095734
};


/* SGFLT f_db_to_linear_fast(SGFLT a_db)
 *
 * Convert decibels to linear using an approximated table lookup
 *
 * Input range:  -100 to 36
 *
 * Use the regular version if you may require more range, otherwise the values
 * will be clipped
 */
SGFLT f_db_to_linear_fast(SGFLT a_db){
    SGFLT f_result = ((a_db + 100.0f) * 4.0f) - 1.0f;

    if((f_result) > arr_amp_db2a_count_m1_f){
        f_result = arr_amp_db2a_count_m1_f;
    }

    if((f_result) < 0.0f){
        f_result = 0.0f;
    }

    return f_linear_interpolate_ptr(arr_amp_db2a, (f_result));
}


#define arr_amp_a2db_count 400
#define arr_amp_a2db_count_SGFLT 400.0f
#define arr_amp_a2db_count_SGFLT_m1 399.0f

SG_THREAD_LOCAL SGFLT arr_amp_a2db[arr_amp_a2db_count]
 = {
-100 ,-40.000000,-33.979401,-30.457575,-27.958801,-26.020601,-24.436975, -23.098040,-21.938202,-20.915152,-20.000002,-19.172148,-18.416376,
-17.721134,-17.077440,-16.478176,-15.917601,-15.391022,-14.894549, -14.424928,-13.979400,-13.555614,-13.151546,-12.765442,-12.395774,
-12.041199,-11.700532,-11.372725,-11.056839,-10.752040,-10.457576, -10.172767,-9.897001,-9.629723,-9.370423,-9.118641,-8.873952,
-8.635967,-8.404330,-8.178710,-7.958803,-7.744326,-7.535017, -7.330634,-7.130949,-6.935753,-6.744847,-6.558046,-6.375179,
-6.196082,-6.020604,-5.848600,-5.679936,-5.514486,-5.352129, -5.192750,-5.036243,-4.882507,-4.731444,-4.582963,-4.436979,
-4.293407,-4.152170,-4.013193,-3.876405,-3.741737,-3.609126, -3.478508,-3.349826,-3.223023,-3.098044,-2.974838,-2.853355,
-2.733547,-2.615370,-2.498780,-2.383733,-2.270190,-2.158113, -2.047463,-1.938205,-1.830305,-1.723728,-1.618443,-1.514419,
-1.411627,-1.310036,-1.209620,-1.110352,-1.012205,-0.915155, -0.819178,-0.724249,-0.630347,-0.537448,-0.445533,-0.354581,
-0.264571,-0.175484,-0.087302,-0.000006,0.086422,0.171998, 0.256739,0.340661,0.423781,0.506112,0.587670,0.668470,
0.748525,0.827848,0.906454,0.984355,1.061563,1.138091, 1.213951,1.289154,1.363712,1.437634,1.510934,1.583619,
1.655702,1.727191,1.798096,1.868428,1.938195,2.007405, 2.076069,2.144194,2.211788,2.278861,2.345420,2.411473,
2.477027,2.542090,2.606669,2.670772,2.734405,2.797576, 2.860290,2.922555,2.984376,3.045761,3.106715,3.167244,
3.227354,3.287051,3.346340,3.405228,3.463719,3.521819, 3.579533,3.636866,3.693822,3.750408,3.806628,3.862486,
3.917987,3.973135,4.027936,4.082393,4.136511,4.190294, 4.243746,4.296871,4.349672,4.402155,4.454323,4.506179,
4.557728,4.608972,4.659916,4.710562,4.760916,4.810978, 4.860754,4.910247,4.959459,5.008393,5.057054,5.105443,
5.153565,5.201421,5.249015,5.296350,5.343428,5.390252, 5.436825,5.483150,5.529230,5.575065,5.620661,5.666018,
5.711140,5.756028,5.800685,5.845115,5.889318,5.933297, 5.977055,6.020593,6.063915,6.107021,6.149915,6.192597,
6.235071,6.277338,6.319401,6.361260,6.402919,6.444380, 6.485643,6.526711,6.567586,6.608269,6.648763,6.689069,
6.729188,6.769124,6.808876,6.848447,6.887839,6.927053, 6.966091,7.004954,7.043644,7.082162,7.120511,7.158690,
7.196703,7.234550,7.272233,7.309753,7.347112,7.384311, 7.421351,7.458233,7.494960,7.531533,7.567952,7.604218,
7.640334,7.676301,7.712119,7.747790,7.783315,7.818696, 7.853932,7.889027,7.923980,7.958794,7.993468,8.028005,
8.062404,8.096667,8.130797,8.164793,8.198656,8.232388, 8.265988,8.299460,8.332804,8.366019,8.399108,8.432072,
8.464911,8.497626,8.530218,8.562689,8.595038,8.627269, 8.659379,8.691371,8.723247,8.755005,8.786647,8.818175,
8.849588,8.880889,8.912077,8.943153,8.974119,9.004975, 9.035722,9.066360,9.096890,9.127314,9.157631,9.187843,
9.217950,9.247953,9.277853,9.307651,9.337345,9.366940, 9.396434,9.425827,9.455122,9.484319,9.513417,9.542418,
9.571323,9.600132,9.628845,9.657465,9.685990,9.714421, 9.742761,9.771008,9.799163,9.827227,9.855201,9.883085,
9.910880,9.938586,9.966204,9.993734,10.021178,10.048535, 10.075807,10.102993,10.130094,10.157110,10.184044,10.210894,
10.237660,10.264345,10.290948,10.317470,10.343911,10.370272, 10.396553,10.422754,10.448877,10.474922,10.500889,10.526778,
10.552591,10.578327,10.603987,10.629571,10.655081,10.680515, 10.705875,10.731162,10.756374,10.781515,10.806582,10.831578,
10.856502,10.881353,10.906136,10.930846,10.955487,10.980058, 11.004560,11.028993,11.053357,11.077653,11.101882,11.126043,
11.150137,11.174164,11.198125,11.222020,11.245851,11.269614, 11.293314,11.316949,11.340520,11.364027,11.387471,11.410851,
11.434170,11.457425,11.480618,11.503750,11.526820,11.549829, 11.572777,11.595665,11.618492,11.641260,11.663968,11.686617,
11.709208,11.731739,11.754212,11.776628,11.798985,11.821285, 11.843528,11.865714,11.887844,11.909917,11.931934,11.953897,
11.975802,11.997654,12.019451};

/* SGFLT f_linear_to_db_fast(
 * SGFLT a_input  //Linear amplitude.  Typically 0 to 1
 * )
 *
 * A fast, table-lookup based linear to decibels converter.
 * The range is 0 to 4, above 4 will clip at 4.
 */
SGFLT f_linear_to_db_fast(SGFLT a_input)
{
    SGFLT f_result = (a_input  * 100.0f);

    if((f_result) >= arr_amp_a2db_count_SGFLT)
    {
        f_result = arr_amp_a2db_count_SGFLT_m1;
    }

    if((f_result) < 0.0f)
    {
        f_result = 0.0f;
    }

    return f_linear_interpolate_ptr(arr_amp_a2db, (f_result));
}


/* SGFLT f_linear_to_db_linear(SGFLT a_input)
 *
 * This takes a 0 to 1 signal and approximates it to a useful range with a logarithmic decibel curve
 * Typical use would be on an envelope that controls the amplitude of an audio signal
 */
SGFLT f_linear_to_db_linear(SGFLT a_input)
{
    SGFLT f_result = ((a_input) * 30.0f) - 30.0f;

    return f_db_to_linear_fast((f_result));
}

