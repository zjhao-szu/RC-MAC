#ifndef UTILITIES_H
#define UTILITIES_H

#include <cstdint>
#include <string.h>
#include <numeric>
#include <gnuradio/expj.h>
#include <volk/volk.h>

namespace gr{
    namespace RC_MAC {
        inline void build_ref_upchirps(gr_complex * chirp,uint32_t id,uint8_t sf,uint8_t os_factor = 1){
            double N =  (1 << sf);
            int n_fold = N * os_factor - id * os_factor;
            for(int n = 0;n < N * os_factor;n++){
                if(n < n_fold){
                    chirp[n] = gr_complex(1.0,0.0) * gr_expj(2.0*M_PI *(n*n/(2*N)/pow(os_factor,2)+(id/N-0.5)*n/os_factor));
                }else{
                    chirp[n] = gr_complex(1.0,0.0) * gr_expj(2.0*M_PI *(n*n/(2*N)/pow(os_factor,2)+(id/N-1.5)*n/os_factor));
                }
            }

        };
        inline void build_chirps(gr_complex* upchirp, gr_complex* downchirp, uint8_t sf, uint8_t os_factor = 1){
            double N = (1 << sf);
            build_ref_upchirps(upchirp,0,sf,os_factor);
            volk_32fc_conjugate_32fc(&downchirp[0], &upchirp[0], N*os_factor);
        };
        inline void build_downchirp(gr_complex * chirp,uint32_t id,uint8_t sf,uint8_t os_factor = 1){
            double N =  (1 << sf);
            int n_fold = N * os_factor - id * os_factor;
            for(int n = 0;n < N * os_factor;n++){
                if(n < n_fold){
                    chirp[n] = gr_complex(1.0,0.0) * gr_expj(2.0*M_PI * -1 * n*n/(2*N)/pow(os_factor,2));
                }else{
                    chirp[n] = gr_complex(1.0,0.0) * gr_expj(2.0*M_PI *((id/N)*n/os_factor) - n*n/(2*N)/pow(os_factor,2));
                }
            }
        };
    }
}



#endif
