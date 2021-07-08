#pragma once

#ifdef __cplusplus
extern "C" {
#endif
size_t getPeakRSS( );
size_t getCurrentRSS( );

size_t getPeakPSS();
size_t getCurrentPSS();
#ifdef __cplusplus
}
#endif
