#if (__ARM_FEATURE_PAC_DEFAULT & 0x2) == 0
#    error "FEAT_PAUTH B-key signing not enabled!"
#endif
