#pragma once
#ifndef FILE_WRITE_WIN_H_
#define FILE_WRITE_WIN_H_

#include <Windows.h>
#include <vector>
#include "save_file_dialog.h"

class FileWriteWin
{
public:
  FileWriteWin();
  ~FileWriteWin();
  void SaveFile(std::vector<char>* data);
private:
  HANDLE hFile_;
};

#endif