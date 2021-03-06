/*

  VSEARCH: a versatile open source tool for metagenomics

  Copyright (C) 2014-2015, Torbjorn Rognes, Frederic Mahe and Tomas Flouri
  All rights reserved.

  Contact: Torbjorn Rognes <torognes@ifi.uio.no>,
  Department of Informatics, University of Oslo,
  PO Box 1080 Blindern, NO-0316 Oslo, Norway

  This software is dual-licensed and available under a choice
  of one of two licenses, either under the terms of the GNU
  General Public License version 3 or the BSD 2-Clause License.


  GNU General Public License version 3

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.


  The BSD 2-Clause License

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

*/

#include "vsearch.h"

unsigned long arch_get_memused()
{
#if defined (__APPLE__) || (__MACH__)
    /* Mac: ru_maxrss gives the size in bytes */
    struct rusage r_usage;
    getrusage(RUSAGE_SELF, & r_usage);
    
    return r_usage.ru_maxrss;
#elif (linux) || (__linux) || (__linux__) || (__unix__) || (__unix)
    /* Linux: ru_maxrss gives the size in kilobytes  */
    struct rusage r_usage;
    getrusage(RUSAGE_SELF, & r_usage);
    return r_usage.ru_maxrss * 1024;
#elif defined (_WIN32)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return (size_t)(status.ullTotalPhys - status.ullAvailPhys);
#else
    fatal("Cannot determine amount of RAM used");
	return 0;
#endif
}

unsigned long arch_get_memtotal()
{
#if defined(__APPLE__)
    
    int mib [] = { CTL_HW, HW_MEMSIZE };
    int64_t ram = 0;
    size_t length = sizeof(ram);
    if(sysctl(mib, 2, &ram, &length, NULL, 0) == -1)
        fatal("Cannot determine amount of RAM");
    return ram;
    
#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
    
    long phys_pages = sysconf(_SC_PHYS_PAGES);
    long pagesize = sysconf(_SC_PAGESIZE);
    if ((phys_pages == -1) || (pagesize == -1))
        fatal("Cannot determine amount of RAM");
    return pagesize * phys_pages;
#elif defined (_WIN32)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return (size_t)status.ullTotalPhys;
#else
    
    struct sysinfo si;
    if (sysinfo(&si))
        fatal("Cannot determine amount of RAM");
    return si.totalram * si.mem_unit;
    
    
#endif
}
