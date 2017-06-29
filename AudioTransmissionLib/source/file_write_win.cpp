#include "file_write_win.h"



FileWriteWin::FileWriteWin() : hFile_(NULL)
{
}


FileWriteWin::~FileWriteWin()
{
}

void FileWriteWin::SaveFile(std::vector<char>* data)
{
  SaveFileDialog* save_file_dialog = new SaveFileDialog();
  save_file_dialog->FilterIndex = 1;
  save_file_dialog->Flags |= OFN_SHOWHELP;
  save_file_dialog->InitialDir = _T("C:\\Windows\\");
  save_file_dialog->Title = _T("Save received file");

  if (!save_file_dialog->ShowDialog())
  {
    return;
  }

  if ((hFile_ = CreateFile(
    save_file_dialog->FileName,
    GENERIC_WRITE,
    NULL,
    NULL,
    CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    NULL
  )) == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "unable to open file \n");
    return;
  }


  DWORD lpNumberOfBytesWritten = 0;
  WriteFile(hFile_, data->data(), data->size(), &lpNumberOfBytesWritten, NULL);
  CloseHandle(hFile_);
  delete save_file_dialog;

}
