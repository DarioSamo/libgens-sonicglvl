#pragma once
#define SONICGLVL_MSG_SETRECORDING 0x10
#define SONICGLVL_MSG_SAVERECORDING 0x11

#include "PipeClient.h"

struct MsgSetRecording : PipeMessage
{
	bool Enable;

	MsgSetRecording(bool enable)
	{
		ID = SONICGLVL_MSG_SETRECORDING;
		Enable = enable;
	}
};

struct MsgSaveRecording : PipeMessage
{
	char FilePath[MAX_PATH];

	MsgSaveRecording()
	{
		ID = SONICGLVL_MSG_SAVERECORDING;
		ZeroMemory(FilePath, sizeof(FilePath));
	}

	MsgSaveRecording(const char* path)
	{
		ID = SONICGLVL_MSG_SAVERECORDING;
		ZeroMemory(FilePath, sizeof(FilePath));
		strcpy(FilePath, path);
	}
};