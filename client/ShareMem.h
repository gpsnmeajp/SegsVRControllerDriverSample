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
#pragma once

#include <windows.h>
#include <stdio.h>
#include "E:\OpenVRDriverProj\conlib\picojson.h"

//----------���L������-----------

class SharedMemory{
private:
	DWORD SharedMemorySize = 16 * 1024; //16KB
	HANDLE SharedMemoryHandle = NULL;
	LPVOID SharedMemoryBuffer = NULL;

public:
	SharedMemory()
	{
	}

	SharedMemory(const char* Pipename)
	{
		printf(Pipename);
		open(Pipename);
	}

	~SharedMemory()
	{
		close();
	}

	LPVOID get_pointer(){
		return SharedMemoryBuffer;
	}

	bool is_open(){
		return (SharedMemoryBuffer != NULL) && (SharedMemoryHandle != NULL);
	}

	void set_size(DWORD _SharedMemorySize)
	{
		SharedMemorySize = _SharedMemorySize;
	}

	DWORD get_size()
	{
		return SharedMemorySize;
	}

	void print(const char *fmt, ...){
		va_list va;
		va_start(va, fmt);
		vsprintf_s((char*)SharedMemoryBuffer, SharedMemorySize-1, fmt, va);
		va_end(va);
	}

	void open(const char* Pipename){
		//�������ɂ��łɊJ���Ă���ꍇ����
		close();

		//�n���h���̃I�[�v��
		SharedMemoryHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SharedMemorySize, Pipename);
		//�n���h���̃I�[�v���Ɏ��s
		if (SharedMemoryHandle == NULL)
		{
			return;
		}

		//�������̃}�b�s���O
		SharedMemoryBuffer = MapViewOfFile(SharedMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		//�}�b�s���O�Ɏ��s
		if (SharedMemoryBuffer == NULL)
		{
			//��ɊJ�����n���h�������
			CloseHandle(SharedMemoryHandle);
			return;
		}
	}

	void close(){
		if (SharedMemoryBuffer != NULL){
			UnmapViewOfFile(SharedMemoryBuffer);
		}
		if (SharedMemoryHandle != NULL){
			CloseHandle(SharedMemoryHandle);
		}
	}
};

//�V�����f�[�^������܂ő҂�
void WaitForNewData(char* mem)
{
	//�f�[�^�҂��t���O�𗧂Ă�
	mem[1] = '\0';
	mem[0] = 'x';

	//�V�����f�[�^������܂ő҂�
	printf("Waiting...\n");
	while (mem[0] == 'x'){
		Sleep(1);
	}
	printf("New Data!\n");

}

//�f�[�^���҂����̂�҂�
void WaitForWaitData(char* mem)
{
	//�V�����f�[�^��҂��Ă���̂�҂�
	printf("Waiting...\n");
	while (mem[0] != 'x'){
		Sleep(1);
	}
	printf("Data waiting!\n");

}

void EPrintf(char* mem,DWORD bufsize, const char *fmt, ...){
	va_list va;
	va_start(va, fmt);
	vsprintf_s(mem, bufsize, fmt, va);
	va_end(va);
}


//----------json-----------

int GetBoolValue(bool &val, picojson::value v, std::string key)
{
	//���݃`�F�b�N
	if (!v.contains(key))
	{
		printf("%s is not found\n", key.c_str());
		return -1;
	}
	//�^�`�F�b�N
	if (!v.get(key).is<bool>())
	{
		printf("%s is not double\n", key.c_str());
		return -2;
	}
	//���o��
	val = v.get(key).get<bool>();
	return 0;
}

int GetDoubleValue(double &val, picojson::value v, std::string key)
{
	//���݃`�F�b�N
	if (!v.contains(key))
	{
		printf("%s is not found\n", key.c_str());
		return -1;
	}
	//�^�`�F�b�N
	if (!v.get(key).is <double>())
	{
		printf("%s is not double\n", key.c_str());
		return -2;
	}
	//���o��
	val = v.get(key).get<double>();
	return 0;
}
int GetDoubleArry(double arry[], unsigned int num, picojson::value v, std::string key)
{
	//���݃`�F�b�N
	if (!v.contains(key))
	{
		printf("%s is not found\n", key.c_str());
		return -1;
	}
	//�^�`�F�b�N
	if (!v.get(key).is<picojson::array>())
	{
		printf("%s is not array\n", key.c_str());
		return -2;
	}

	//�z��T�C�Y�`�F�b�N
	picojson::array vx = v.get(key).get<picojson::array>();
	if (vx.size() != num)
	{
		printf("%s unmatch array size. Result: %d vs Expected: %d\n", key.c_str(), vx.size(), num);
		return -3;
	}

	//�z��^�`�F�b�N
	for (unsigned int i = 0; i < num; i++){
		if (!vx[i].is<double>())
		{
			printf("%s[%d] unmatch type.\n", key.c_str(), i);
			return -4;
		}
	}

	//�R�s�[
	for (unsigned int i = 0; i < num; i++){
		arry[i] = vx[i].get<double>();
	}
	return 0;
}
