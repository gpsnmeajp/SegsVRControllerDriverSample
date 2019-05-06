/*
BSD 3-Clause License

Copyright (c) 2019, gpsnmeajp
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "E:\OpenVRDriverProj\conlib\ShareMem.h"
#include "E:\OpenVRDriverProj\conlib\picojson.h"


int main(void){
	SharedMemory comm("pip1");
	if (!comm.is_open())
	{
		return -1;
	}

	char *SharedRam = (char*)comm.get_pointer();

	SharedRam[0] = '\0';
	int i = 0;
	double x,y,z;
	x = 0; y = 0; z = 0;
	while (!_kbhit())
	{
		WaitForWaitData(SharedRam); //ƒf[ƒ^‘Ò‚¿‚É‚È‚é‚Ü‚Å‘Ò‚Â

		int c = _getche();
		switch (c)
		{
		case 'q':x += 0.01; break;
		case 'a':x -= 0.01; break;
		case 'w':y += 0.01; break;
		case 's':y -= 0.01; break;
		case 'e':z += 0.01; break;
		case 'd':z -= 0.01; break;
		}
		
		comm.print("{\"id\":0,\"v\":[%lf,%lf,%lf],\"vd\":[0,0,0],\"vdd\":[0,0,0],\"r\":[0,0,0,0],\"rd\":[0,0,0],\"rdd\":[0,0,0],\"Valid\":true}",x,y,z);
		printf("->%s\n", SharedRam);
		i++;
	}


	return 0;
}
