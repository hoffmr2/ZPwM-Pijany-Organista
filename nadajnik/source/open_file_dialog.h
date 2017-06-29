
#pragma once
#ifndef OPEN_FILE_DIALOG_H_
#define OPEN_FILE_DIALOG_H_

#include <Windows.h>
#include <Commdlg.h>
#include <tchar.h>

class OpenFileDialog
{
public:
  OpenFileDialog(void);

  TCHAR*DefaultExtension;
  TCHAR*FileName;
  TCHAR*Filter;
  int FilterIndex;
  int Flags;
  TCHAR*InitialDir;
  HWND Owner;
  TCHAR*Title;

  bool ShowDialog() const;
};

#endif
