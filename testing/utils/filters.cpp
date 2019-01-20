#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <chrono>
#define RTSEIS_LOGGING 1
#include "rtseis/utils/filters.hpp"
#include "rtseis/log.h"
#include "utils.hpp"

using namespace RTSeis::Utils::Filters;
static int readTextFile(int *npts, double *xPtr[],
                        const std::string fileName = "utils/data/gse2.txt");
static int filters_downsample_test(const int npts, const double x[]);
static int filters_medianFilter_test(const int npts, const double x[],
                                     const std::string fileName);
static int filters_sosFilter_test(const int npts, const double x[],
                                  const std::string filename);

int rtseis_test_utils_filters(void)
{
    double dt = 1.0/200.0;
    int npts;
    double *x = nullptr;
    std::string dataDir = "utils/data/";
    // Load the data
    int ierr = readTextFile(&npts, &x, dataDir + "gse2.txt");
    if (ierr != EXIT_SUCCESS)
    {
        RTSEIS_ERRMSG("%s", "Failed to read gse2 data");
        return EXIT_FAILURE;
    }
    // Apply the downsampler
    ierr = filters_downsample_test(npts, x);
    if (ierr != EXIT_SUCCESS)
    {
        RTSEIS_ERRMSG("%s", "Failed downsampler test");
        return EXIT_FAILURE;
    }
    // Apply the median filter
    ierr = filters_medianFilter_test(npts, x,
                                     dataDir + "medianFilterReference.txt");
    if (ierr != EXIT_SUCCESS)
    {
        RTSEIS_ERRMSG("%s", "Failed median filter test");
        return EXIT_FAILURE;
    }
    // Apply the second order section filter
    ierr = filters_sosFilter_test(npts, x, "gse2.txt");

    if (x != nullptr){free(x);}
    return EXIT_SUCCESS;
}
//============================================================================//
int filters_sosFilter_test(const int npts, const double x[],
                           const std::string filename)
{
    fprintf(stdout, "Testing SOS filter...\n");
    int ns = 7; // number of sections 
    const double bs7[21] = {6.37835424e-05,  6.37835424e-05,  0.00000000e+00, 
                           1.00000000e+00, -1.78848938e+00,  1.00000000e+00,
                           1.00000000e+00, -1.93118487e+00,  1.00000000e+00,
                           1.00000000e+00, -1.95799864e+00,  1.00000000e+00,
                           1.00000000e+00, -1.96671846e+00,  1.00000000e+00,
                           1.00000000e+00, -1.97011885e+00,  1.00000000e+00,
                           1.00000000e+00, -1.97135784e+00,  1.00000000e+00};
    const double as7[21] = {1.00000000e+00, -9.27054679e-01,  0.00000000e+00,
                           1.00000000e+00, -1.87008942e+00,  8.78235919e-01,
                           1.00000000e+00, -1.90342568e+00,  9.17455718e-01,
                           1.00000000e+00, -1.93318668e+00,  9.52433552e-01,
                           1.00000000e+00, -1.95271141e+00,  9.75295685e-01,
                           1.00000000e+00, -1.96423610e+00,  9.88608056e-01,
                           1.00000000e+00, -1.97157693e+00,  9.96727086e-01};
    double yref40[40] = {6.37835424e-05,  1.23511272e-04,  1.34263690e-04,
                         1.78634911e-04,  2.50312740e-04,  3.46332848e-04,
                         4.66239952e-04,  6.11416691e-04,  7.84553129e-04,
                         9.89232232e-04,  1.22960924e-03,  1.51016546e-03,
                         1.83551947e-03,  2.21028135e-03,  2.63893773e-03,
                         3.12575784e-03,  3.67471270e-03,  4.28940130e-03,
                         4.97297977e-03,  5.72809028e-03,  6.55678845e-03,
                         7.46046851e-03,  8.43978671e-03,  9.49458408e-03,
                         1.06238101e-02,  1.18254496e-02,  1.30964547e-02,
                         1.44326848e-02,  1.58288573e-02,  1.72785101e-02,
                         1.87739799e-02,  2.03063976e-02,  2.18657022e-02,
                         2.34406756e-02,  2.50189979e-02,  2.65873261e-02,
                         2.81313940e-02,  2.96361349e-02,  3.10858256e-02,
                         3.24642512e-02};
    double *impulse = new double[40];
    double *y40 = new double[40]; 
    std::fill(impulse, impulse+40, 0);
    impulse[0] = 1;
    SOSFilter sos;
    bool lrt = false;
    int ierr = sos.initialize(ns, bs7, as7, lrt, RTSEIS_DOUBLE);
    if (ierr != 0)
    {
        RTSEIS_ERRMSG("%s", "Failed to initialize sos");
        return -1;
    }
    ierr = sos.apply(40, impulse, y40);
    if (ierr != 0)
    {
        RTSEIS_ERRMSG("%s", "Failed to apply filter");
        return -1;
    }
    for (int i=0; i<40; i++)
    {
        if (std::abs(y40[i] - yref40[i]) > 1.e-8)
        {
            RTSEIS_ERRMSG("Impulse response failed %lf %lf", yref40[i], y40[i]);
            return EXIT_FAILURE;
        }
    }
    delete[] y40;
    delete[] impulse;
    // Now do a real problem

    return EXIT_SUCCESS;
    
}
int filters_medianFilter_test(const int npts, const double x[],
                              const std::string fileName)
{
    fprintf(stdout, "Testing median filter...\n");
    double xin[8] = {1, 2, 127, 4, 5, 0, 7, 8};
    double y8[8];
    double yref3[8] = {1, 2, 4, 5, 4, 5, 7, 7}; // Matlab soln; IPP has edge effect
    double yref5[8] = {1, 2, 4, 4, 5, 5, 5, 0};
    int ierr;
    MedianFilter median;
    bool lrt = false;
    ierr = median.initialize(3, lrt, RTSEIS_DOUBLE);
    if (ierr != 0)
    {
        RTSEIS_ERRMSG("%s", "Failed to initialize filter");
        return EXIT_FAILURE;
    }
    ierr = median.apply(8, xin, y8); 
    if (ierr != 0)
    {
        RTSEIS_ERRMSG("%s", "Failed to apply filter");
        return EXIT_FAILURE;
    }
    for (int i=1; i<8-1; i++)
    {
        if (std::abs(y8[i+1] - yref3[i]) > 1.e-14)
        {
            RTSEIS_ERRMSG("Failed test %lf %lf", y8[i+1], yref3[i]);
            return EXIT_FAILURE;
        }
    }
    ierr = median.initialize(5, lrt, RTSEIS_DOUBLE);
    ierr = median.apply(8, xin, y8);
    for (int i=2; i<8-2; i++)
    {
        if (std::abs(y8[i+2] - yref5[i]) > 1.e-14)
        {
            RTSEIS_ERRMSG("Failed test %lf %lf", y8[i+2], yref5[i]);
            return EXIT_FAILURE;
        }
    }
    // Load a reference solution
    double *yref = nullptr;
    int npref;
    ierr = readTextFile(&npref, &yref, fileName);
    if (ierr != 0 || npref - 11/2 != npts)
    {
        RTSEIS_ERRMSG("%s", "Failed to load reference data");
        return EXIT_FAILURE;
    }
    median.initialize(11, lrt, RTSEIS_DOUBLE);
    auto timeStart = std::chrono::high_resolution_clock::now();
    double *y = new double[npts];
    ierr = median.apply(npts, x, y);
    if (ierr != 0)
    {
        RTSEIS_ERRMSG("%s", "Failed to compute reference solution");
        return EXIT_FAILURE;
    }
    for (int i=0; i<npts; i++)
    {
        if (std::abs(y[i] - yref[i]) > 1.e-10)
        {
            RTSEIS_ERRMSG("Failed to compute reference soln %d %lf %lf",
                          i, y[i], yref[i]);
            return EXIT_FAILURE;
        }
    }
    auto timeEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> tdif = timeEnd - timeStart;
    fprintf(stdout, "Reference solution computation time %.8lf (s)\n",
            tdif.count());
    // Now do the packetized tests
    lrt = true;
    median.initialize(11, lrt, RTSEIS_DOUBLE);
    std::vector<int> packetSize({1, 2, 3, 16, 64, 100, 200, 512,
                                 1000, 1024, 1200, 2048, 4000, 4096, 5000});
    for (int job=0; job<2; job++)
    {
        for (size_t ip=0; ip<packetSize.size(); ip++)
        {
            timeStart = std::chrono::high_resolution_clock::now();
            int nxloc = 0;
            int nptsPass = 0;
            while (nxloc < npts)
            {
                nptsPass = packetSize[ip];
                if (job == 1)
                {
                     nptsPass = std::max(1, nptsPass + rand()%50 - 25);  
                }
                nptsPass = std::min(nptsPass, npts - nxloc);
                ierr = median.apply(nptsPass, &x[nxloc], &y[nxloc]);
                if (ierr != 0)
                {
                    RTSEIS_ERRMSG("%s", "Failed to apply median filter");
                    return EXIT_FAILURE;
                }
                nxloc = nxloc + nptsPass;
            }
            median.resetInitialConditions();
            auto timeEnd = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> tdif = timeEnd - timeStart;
            for (int i=0; i<npts; i++)
            {
                if (std::abs(yref[i] - y[i]) > 1.e-10)
                {
                    RTSEIS_ERRMSG("Failed to compute reference soln %d %lf %lf",
                                  i, y[i], yref[i]);
                    return EXIT_FAILURE;
                }
            }
            if (job == 0)
            {
                fprintf(stdout,
                        "Passed median filter fixed packet size %4d in %.8e (s)\n",
                        packetSize[ip], tdif.count());
            }
            else
            {
                fprintf(stdout,
                        "Passed median filter random in %.8e (s)\n", tdif.count());
            }
        }
    }   
    free(yref);
    delete[] y;
    return EXIT_SUCCESS;
}
//============================================================================//
int filters_downsample_test(const int npts, const double x[])
{
    const int nq = 7;
    const enum rtseisPrecision_enum precision = RTSEIS_DOUBLE;
    // Call this in post-processing for a couple different decimation rates
    bool lrt = false;
    srand(10245); 
    Downsample downsample;
    double *y = static_cast<double *>
                (calloc(static_cast<size_t> (npts), sizeof(double)));
    double *yref = static_cast<double *>
                   (calloc(static_cast<size_t> (npts), sizeof(double)));
    int ierr = 0;
    fprintf(stdout, "Testing downsampler...\n");
    for (int iq=1; iq<nq+1; iq++)
    {
        // Do a post-processing test
        memset(y, 0, static_cast<size_t> (npts)*sizeof(double));
        memset(yref, 0, static_cast<size_t> (npts)*sizeof(double));
        lrt = false;
        ierr = downsample.initialize(iq, lrt, precision); 
        if (ierr != 0)
        {
            RTSEIS_ERRMSG("%s", "Failed to intiialized downsample");
            return EXIT_FAILURE;
        }
        auto timeStart = std::chrono::high_resolution_clock::now();
        int ny;
        ierr = downsample.apply(npts, x, npts, &ny, y);
        if (ierr != 0)
        {
            RTSEIS_ERRMSG("%s", "Failed to call downsampler");
            return EXIT_FAILURE;
        }
        auto timeEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> tdif = timeEnd - timeStart;
        // Do a manual downsample
        int j = 0;
        for (int i=0; i<npts; i=i+iq)
        {
            if (std::abs(y[j] - x[i]) > 1.e-10)
            {
                RTSEIS_ERRMSG("%s", "Post-processing downsample failed");
                return EXIT_FAILURE;
            }
            j = j + 1;
        }
        if (j != ny)
        {
            RTSEIS_ERRMSG("%s", "Incorrect number of output points");
            return EXIT_FAILURE;
        } 
        downsample.clear();
        fprintf(stdout,
                "Post-processing execution time for nq=%d is %.8lf (s)\n",
                iq, tdif.count());
        // Make a copy of the correct answer
        int nyref = ny;
        for (int iy=0; iy<ny; iy++){yref[iy] = y[iy];}
        // Do a real-time test
        lrt = true; 
        ierr = downsample.initialize(iq, lrt, precision);
        if (ierr != 0)
        {
            RTSEIS_ERRMSG("%s", "Failed to intiialized downsample");
            return EXIT_FAILURE;
        }
        std::vector<int> packetSize({1, 2, 3, 16, 64, 100, 200, 512,
                                     1000, 1024, 1200, 2048, 4000, 4096, 5000});
        for (size_t ip=0; ip<packetSize.size(); ip++)
        {
            timeStart = std::chrono::high_resolution_clock::now();
            int nxloc = 0;
            int nyloc = 0;
            while (nxloc < npts)
            {
                int nptsPass = std::min(packetSize[ip], npts - nxloc);
                int nyDec = 0;
                ierr = downsample.apply(nptsPass, &x[nxloc],
                                        npts+1-nxloc, &nyDec, &y[nyloc]);
                if (ierr != 0)
                {
                    RTSEIS_ERRMSG("Failed to apply downsampler for iq=%d", iq);
                    return EXIT_FAILURE; 
                }
                nxloc = nxloc + nptsPass;
                nyloc = nyloc + nyDec;
            }
            auto timeEnd = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> tdif = timeEnd - timeStart;
            downsample.resetInitialConditions();
            if (nyloc != nyref)
            {
                RTSEIS_ERRMSG("%s", "Failed fixed packet size test");
                return EXIT_FAILURE;
            }
            for (int iy=0; iy<nyref; iy++)
            {
                if (std::abs(y[iy] - yref[iy]) > 1.e-10)
                {
                    RTSEIS_ERRMSG("%s", "Failed fixed packet size test");
                    return EXIT_FAILURE;
                }
            }
            if (iq == 7)
            {
                fprintf(stdout,
                   "Passed downsampler fixed packet size %4d w/ nq=%d in %.8e (s)\n",
                    packetSize[ip], iq, tdif.count());
            }
        }
        // Random packet sizes
        timeStart = std::chrono::high_resolution_clock::now();
        int nxloc = 0;
        int nyloc = 0;
        int packetLen = 100;
        while (nxloc < npts)
        {
            int nptsPass = std::min(packetLen, npts - nxloc);
            int nyDec = 0;
            ierr = downsample.apply(nptsPass, &x[nxloc],
                                    npts+1-nxloc, &nyDec, &y[nyloc]);
            if (ierr != 0)
            {
                RTSEIS_ERRMSG("Failed to apply downsampler for iq=%d", iq);
                return EXIT_FAILURE;
            }
            nxloc = nxloc + nptsPass;
            nyloc = nyloc + nyDec;
            packetLen = std::max(1, packetLen + rand()%50 - 25);
        }
        if (nyloc != nyref)
        {
            RTSEIS_ERRMSG("%s", "Failed fixed packet size test");
            return EXIT_FAILURE;
        }
        for (int iy=0; iy<nyref; iy++)
        {
            if (std::abs(y[iy] - yref[iy]) > 1.e-10)
            {
                RTSEIS_ERRMSG("%s", "Failed fixed packet size test");
                return EXIT_FAILURE;
            }
        }
        timeEnd = std::chrono::high_resolution_clock::now();
        tdif = timeEnd - timeStart;
        if (iq == 7)
        {
            fprintf(stdout,
                    "Passed downsampler random packet size w/ nq=%d in %.8e (s)\n",
                    iq, tdif.count());
        }
        // Loop 
        downsample.clear();
    }
    free(y);
    free(yref);
    fprintf(stdout, "Passed downsampler test\n");
    return EXIT_SUCCESS;
}
//============================================================================//
int readTextFile(int *npts, double *xPtr[], const std::string fileName)
{
    *xPtr = nullptr;
    *npts = 0;
    char line[64];
    double *x = nullptr;
    FILE *fl = fopen(fileName.c_str(), "r");
    *npts = 0;
    while (fscanf(fl, "%s", line) != EOF)
    {
        *npts = *npts + 1;
    }
    rewind(fl);
    if (*npts < 1)
    {
        RTSEIS_ERRMSG("%s", "No data points in file\n");
        return EXIT_FAILURE;
    }
    x = static_cast<double *> (calloc(static_cast<size_t> (*npts),
                              sizeof(double)));
    for (int i=0; i<*npts; i++)
    {
        memset(line, 0, 64*sizeof(char));
        fgets(line, 64, fl);
        sscanf(line, "%lf\n", &x[i]);
    }
    fclose(fl);
    *xPtr = x;
    return EXIT_SUCCESS;
} 