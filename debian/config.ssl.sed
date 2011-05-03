s/\(SYS_LIBRARIES.*\)/\1 -lssl -lcrypto/
s/\(MYCFLAGS.*\)/\1 -I\/usr\/include\/openssl/
s/#undef USE_SSL/#define USE_SSL/
