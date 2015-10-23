// Lotsa_dicts.cpp: ���������� ����� ����� ��� ����������� ����������.
//
//#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "windows.h"
#include <iostream>
#include <string>
#include <map>
#include <memory>

using namespace std;

struct s_params
{
	string inputDir;
	string outputDir;
	HANDLE stopEvent;
	//HANDLE dirChangedEvent;
};
void chop_file(string filename, string inputDir, string outputDir);

DWORD WINAPI make_dicts(void* params) //����������� ����� � ����� � �� ���������
{
	s_params* parameters = (s_params*)params;
	//��� ������ ��������
	string filenamePattern = parameters->inputDir + "*.txt";
	WIN32_FIND_DATA fileInfo;
	HANDLE fileSearch = FindFirstFile(filenamePattern.c_str(), &fileInfo);
	while (fileSearch != INVALID_HANDLE_VALUE &&
		WaitForSingleObject(parameters->stopEvent, 0) != WAIT_OBJECT_0)
		//WaitForSingleObject � ���� ������ ���������,
		//��������� �� ����� � ���������� ���������
		//WaitForMultipleObjects ��� ���������� �������
		//FindNextChangeNotification ��� ��������� ��������� �����
	{
		string filename = fileInfo.cFileName;
		chop_file(filename, parameters->inputDir, parameters->outputDir);
		if (FindNextFile(fileSearch, &fileInfo) == false)
			break;
	}
	FindClose(fileSearch);

	HANDLE dirChangedEvent = FindFirstChangeNotification(
		parameters->inputDir.c_str(),
		false,
		FILE_NOTIFY_CHANGE_FILE_NAME);

	HANDLE events[2];
	events[0] = parameters->stopEvent;
	events[1] = dirChangedEvent;

	while (WaitForMultipleObjects(2, events, false, INFINITE) != 0)
	{
		
		fileSearch = FindFirstFile(filenamePattern.c_str(), &fileInfo);
		while (fileSearch != INVALID_HANDLE_VALUE)
			//&& WaitForSingleObject(parameters->stopEvent, 0) != WAIT_OBJECT_0)
			//�� �������� - ����� ������ ������������ � �������� ��������, �� WaitForMultipleObjects ������� �� ���������
		{
			string filename = fileInfo.cFileName;
			chop_file(filename, parameters->inputDir, parameters->outputDir);
			if (FindNextFile(fileSearch, &fileInfo) == false)
				break;
		} 
		FindClose(fileSearch);
		FindNextChangeNotification(dirChangedEvent); //������, ��������� �� ��������� �� ������ ��������� �����
		//�������� ����� �������, �� ��� ���� �� ������ � ����������� ���� =(
	}
	//������ �������� � ��������� ��������� ��-�� �������� ������ fileSearch ������ ���
	//�� �����, ��� ������!
	//��������, ��� ��������

	CloseHandle(dirChangedEvent);

	//FindClose(fileSearch);
	return 0;
}

void chop_file(string filename, string inputDir, string outputDir) //Buy for 4.99$ - SALE!!111oneone
{
	cout << "chopping file " << filename << endl;
	string inputFileName = inputDir + filename;
	HANDLE inputFile = CreateFile(inputFileName.c_str(), GENERIC_READ, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE outputFile = CreateFile((outputDir+"out_"+filename).c_str(), GENERIC_WRITE, 0,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	map<string, int> wordCount;
	string delims = " ,.\n\r:\\\"()='-/;";
	int sizeOfBuffer = 255;
	unique_ptr<char[]>buffer(new char[sizeOfBuffer + 1]);
	DWORD bytesRead;
	unique_ptr<char[]> current(new char[sizeOfBuffer + 1]);
	current[0] = 0;
	unique_ptr<char[]> next(new char[sizeOfBuffer + 1]);
	ReadFile(inputFile, buffer.get(), sizeOfBuffer, &bytesRead, NULL);
	buffer[bytesRead] = '\0';
	bool isTruncated = false;
	while (bytesRead != 0) //���� ���� ����
	{
		buffer[bytesRead] = 0;
		if (bytesRead != 0) //���� ������� ��������
		{
			if (isTruncated)
				strcat(current.get(), strtok(buffer.get(), delims.c_str()));
			else
				strcpy(current.get(), strtok(buffer.get(), delims.c_str()));

			if (strchr(delims.c_str(), buffer[bytesRead - 1]))
				isTruncated = true;
			else
				isTruncated = false;

			char* temp = strtok(NULL, delims.c_str());
			if (temp != NULL)
				strcpy(next.get(), temp);
			while (temp != NULL)
			{
				wordCount[string(current.get())]++;
				strcpy(current.get(), next.get());
				temp = strtok(NULL, delims.c_str());
				if (temp != NULL)
					strcpy(next.get(), temp);
			}
		}

		ReadFile(inputFile, buffer.get(), sizeOfBuffer, &bytesRead, NULL);
		buffer[bytesRead] = '\0';
	}

	for (auto x : wordCount)
	{
		std::string dictionaryLine = x.first + " - " + to_string(x.second) + "\r\n";
		//std::cout << dictionaryLine;
		DWORD bytesWritten;
		WriteFile(outputFile, dictionaryLine.c_str(), strlen(dictionaryLine.c_str()), &bytesWritten, NULL);
	}


	CloseHandle(inputFile);
	CloseHandle(outputFile);
	DeleteFile(inputFileName.c_str());
}

void add_trailing_slash(string &s)
{
	if (s[s.length() - 1] != '\\')
		s += "\\";
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Russian");
	string inputDir,outputDir;
	cout << "������� �����:" << endl;
	cin >> inputDir;
	cout << "�������� �����:" << endl;
	cin >> outputDir;
	cin.ignore();
	inputDir = "D:\\test\\in\\";
	outputDir = "D:\\test\\out\\";
	//////////////////////////////////////
	add_trailing_slash(inputDir);
	add_trailing_slash(outputDir);
	//////////////////////////////////////
	HANDLE stopEvent = CreateEvent(NULL, true, false, NULL);
	//////////////////////////////////////////////////
	s_params* threadParameters = new s_params;
	threadParameters->inputDir = inputDir;
	threadParameters->outputDir = outputDir;
	threadParameters->stopEvent = stopEvent;
	//threadParameters->dirChangedEvent = dirChangedEvent;
	//////////////////////////////////////////////////
	DWORD threadID; //�� ������
	HANDLE dictThread = CreateThread(
						NULL, 
						0, 
						make_dicts, 
						threadParameters, 
						0, 
						&threadID);

	while (WaitForSingleObject(stopEvent, 0) != WAIT_OBJECT_0)
	{
		char c = cin.get();
		if (c == '\n')
			SetEvent(stopEvent);
	}

	WaitForSingleObject(dictThread, INFINITE); //���, ���� ���������� ������ �����

	CloseHandle(dictThread);
	CloseHandle(stopEvent);
	//CloseHandle(dirChangedEvent);
	return 0;
}

