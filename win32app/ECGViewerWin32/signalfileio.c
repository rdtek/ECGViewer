#include "signalfileio.h"

void DoOpenFile(HeartSignal* heartSignal, int maxNum, OPENFILENAME openFileName) {
    WCHAR szFile[100];

    ZeroMemory(&openFileName, sizeof(openFileName));
    openFileName.lStructSize = sizeof(openFileName);
    openFileName.hwndOwner = NULL;
    openFileName.lpstrFile = szFile;
    openFileName.lpstrFile[0] = '\0';
    openFileName.nMaxFile = sizeof(szFile);
    openFileName.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
    openFileName.nFilterIndex = 1;
    openFileName.lpstrFileTitle = NULL;
    openFileName.nMaxFileTitle = 0;
    openFileName.lpstrInitialDir = NULL;
    openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    GetOpenFileName(&openFileName);

    FILE * ptr_file = _wfopen(openFileName.lpstrFile, L"r");
    //int numLines = CountFileLines(openFileName.lpstrFile);

    if (ptr_file != NULL) {

        int indexSamples = 0;
        heartSignal->numberOfSamples = 0;
        char strLine[128];

        while (fgets(strLine, sizeof strLine, ptr_file) != NULL) {
            heartSignal->samples[indexSamples] = strtol(strLine, NULL, 10);
            heartSignal->numberOfSamples++;
            indexSamples++;
            //log_int("\nidxSignalArray: ", idxSignalArray);
            if (heartSignal->numberOfSamples >= maxNum) break;
        }

        fclose(ptr_file);
    }

    return;
}

int CountFileLines(const wchar_t* fileName) {
    int numLines = 0;
    int chr = 0;

    log_wstr(L"ECG Filename: ", fileName);

    FILE * ptr_file = _wfopen(fileName, L"r");
    if (ptr_file != NULL) {
        while (!feof(ptr_file)) {
            chr = fgetc(ptr_file);
            if (chr == '\n') {
                numLines++;
            }
        }
        fclose(ptr_file);
    }

    log_int("\nNumber of lines: ", numLines);
    return numLines;
}