#ifndef _SENDFILE_H
#define _SENDFILE_H

#include "../hardware/link.hpp"

LINK_ERR SendFile(const CALC* lpCalc, const char * lpszFileName, SEND_FLAG Destination);

#endif