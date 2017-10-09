/*++

Copyright (c) Microsoft Corporation

Module Name:
- CommonState.hpp

Abstract:
- This represents common boilerplate state setup required for unit tests to run

Author(s):
- Michael Niksa (miniksa) 18-Jun-2014
- Paul Campbell (paulcam) 18-Jun-2014

Revision History:
- Tranformed to header-only class so it can be included by multiple
unit testing projects in the codebase without a bunch of overhead.
--*/

#pragma once

#include "precomp.h"
#include "../host/globals.h"
#include "../host/newdelete.hpp"
#include "../interactivity/inc/ServiceLocator.hpp"


class CommonState
{
public:

    static const SHORT s_csWindowWidth = 80;
    static const SHORT s_csWindowHeight = 80;
    static const SHORT s_csBufferWidth = 80;
    static const SHORT s_csBufferHeight = 300;

    CommonState()
    {
        m_heap = GetProcessHeap();
    }

    ~CommonState()
    {
        m_heap = nullptr;
    }

    void PrepareGlobalFont()
    {
        COORD coordFontSize;
        coordFontSize.X = 8;
        coordFontSize.Y = 12;
        m_pFontInfo = new FontInfo(L"Consolas", 0, 0, coordFontSize, 0);
    }

    void CleanupGlobalFont()
    {
        if (m_pFontInfo != nullptr)
        {
            delete m_pFontInfo;
        }
    }

    void PrepareGlobalScreenBuffer()
    {
        CONSOLE_INFORMATION* const gci = ServiceLocator::LocateGlobals()->getConsoleInformation();
        COORD coordWindowSize;
        coordWindowSize.X = s_csWindowWidth;
        coordWindowSize.Y = s_csWindowHeight;

        COORD coordScreenBufferSize;
        coordScreenBufferSize.X = s_csBufferWidth;
        coordScreenBufferSize.Y = s_csBufferHeight;

        CHAR_INFO ciFill;
        ciFill.Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;

        CHAR_INFO ciPopupFill;
        ciPopupFill.Attributes = FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_RED;

        UINT uiCursorSize = 12;

        SCREEN_INFORMATION::CreateInstance(coordWindowSize,
                                           m_pFontInfo,
                                           coordScreenBufferSize,
                                           ciFill,
                                           ciPopupFill,
                                           uiCursorSize,
                                           &gci->CurrentScreenBuffer);
    }

    void CleanupGlobalScreenBuffer()
    {
        const CONSOLE_INFORMATION* const gci = ServiceLocator::LocateGlobals()->getConsoleInformation();
        delete gci->CurrentScreenBuffer;
    }

    void PrepareGlobalInputBuffer()
    {
        CONSOLE_INFORMATION* const gci = ServiceLocator::LocateGlobals()->getConsoleInformation();
        gci->pInputBuffer = new InputBuffer();
    }

    void CleanupGlobalInputBuffer()
    {
        const CONSOLE_INFORMATION* const gci = ServiceLocator::LocateGlobals()->getConsoleInformation();
        delete gci->pInputBuffer;
    }

    void PrepareCookedReadData()
    {
        CONSOLE_INFORMATION* const gci = ServiceLocator::LocateGlobals()->getConsoleInformation();
        gci->lpCookedReadData = new COOKED_READ_DATA();
    }

    void CleanupCookedReadData()
    {
        const CONSOLE_INFORMATION* const gci = ServiceLocator::LocateGlobals()->getConsoleInformation();
        delete gci->lpCookedReadData;
    }

    void PrepareNewTextBufferInfo()
    {
        const CONSOLE_INFORMATION* const gci = ServiceLocator::LocateGlobals()->getConsoleInformation();
        COORD coordScreenBufferSize;
        coordScreenBufferSize.X = s_csBufferWidth;
        coordScreenBufferSize.Y = s_csBufferHeight;

        CHAR_INFO ciFill;
        ciFill.Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;

        UINT uiCursorSize = 12;

        m_backupTextBufferInfo = gci->CurrentScreenBuffer->TextInfo;

        m_ntstatusTextBufferInfo = TEXT_BUFFER_INFO::CreateInstance(m_pFontInfo,
                                                                    coordScreenBufferSize,
                                                                    ciFill,
                                                                    uiCursorSize,
                                                                    &gci->CurrentScreenBuffer->TextInfo);
    }

    void CleanupNewTextBufferInfo()
    {
        const CONSOLE_INFORMATION* const gci = ServiceLocator::LocateGlobals()->getConsoleInformation();
        ASSERT(gci->CurrentScreenBuffer != nullptr);
        delete gci->CurrentScreenBuffer->TextInfo;

        gci->CurrentScreenBuffer->TextInfo = m_backupTextBufferInfo;
    }

    void FillTextBuffer()
    {
        const CONSOLE_INFORMATION* const gci = ServiceLocator::LocateGlobals()->getConsoleInformation();
        // fill with some assorted text that doesn't consume the whole row
        const SHORT cRowsToFill = 4;

        ASSERT(gci->CurrentScreenBuffer != nullptr);
        ASSERT(gci->CurrentScreenBuffer->TextInfo != nullptr);

        TEXT_BUFFER_INFO* pTextInfo = gci->CurrentScreenBuffer->TextInfo;

        for (SHORT iRow = 0; iRow < cRowsToFill; iRow++)
        {
            ROW* pRow = &pTextInfo->Rows[iRow];
            FillRow(pRow);
        }

        pTextInfo->GetCursor()->SetYPosition(cRowsToFill);
    }

    void FillTextBufferBisect()
    {
        const CONSOLE_INFORMATION* const gci = ServiceLocator::LocateGlobals()->getConsoleInformation();
        // fill with some text that fills the whole row and has bisecting double byte characters
        const SHORT cRowsToFill = s_csBufferHeight;

        ASSERT(gci->CurrentScreenBuffer != nullptr);
        ASSERT(gci->CurrentScreenBuffer->TextInfo != nullptr);

        TEXT_BUFFER_INFO* pTextInfo = gci->CurrentScreenBuffer->TextInfo;

        for (SHORT iRow = 0; iRow < cRowsToFill; iRow++)
        {
            ROW* pRow = &pTextInfo->Rows[iRow];
            FillBisect(pRow);
        }

        pTextInfo->GetCursor()->SetYPosition(cRowsToFill);
    }

    NTSTATUS GetTextBufferInfoInitResult()
    {
        return m_ntstatusTextBufferInfo;
    }

private:
    HANDLE m_heap;
    NTSTATUS m_ntstatusTextBufferInfo;
    FontInfo* m_pFontInfo;
    TEXT_BUFFER_INFO* m_backupTextBufferInfo;

    void FillRow(ROW* pRow)
    {
        // fill a row
        // 9 characters, 6 spaces. 15 total
        // か = \x304b
        // き = \x304d
        PCWSTR pwszText = L"AB" L"\x304b\x304b" L"C" L"\x304d\x304d" L"DE      ";
        memcpy_s(pRow->CharRow.Chars, CommonState::s_csBufferWidth, pwszText, wcslen(pwszText));
        pRow->CharRow.Left = 0;
        pRow->CharRow.Right = 9; // 1 past the last valid character in the array

        // set double-byte/double-width attributes
        pRow->CharRow.KAttrs[2] = CHAR_ROW::ATTR_LEADING_BYTE;
        pRow->CharRow.KAttrs[3] = CHAR_ROW::ATTR_TRAILING_BYTE;
        pRow->CharRow.KAttrs[5] = CHAR_ROW::ATTR_LEADING_BYTE;
        pRow->CharRow.KAttrs[6] = CHAR_ROW::ATTR_TRAILING_BYTE;

        // set some colors
        TextAttribute Attr = TextAttribute(0);
        pRow->AttrRow.Initialize(15, Attr);
        // A = bright red on dark gray
        // This string starts at index 0
        Attr = TextAttribute(FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY);
        pRow->AttrRow.SetAttrToEnd(0, Attr);

        // BかC = dark gold on bright blue
        // This string starts at index 1
        Attr = TextAttribute(FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
        pRow->AttrRow.SetAttrToEnd(1, Attr);

        // き = bright white on dark purple
        // This string starts at index 5
        Attr = TextAttribute(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE);
        pRow->AttrRow.SetAttrToEnd(5, Attr);

        // DE = black on dark green
        // This string starts at index 7
        Attr = TextAttribute(BACKGROUND_GREEN);
        pRow->AttrRow.SetAttrToEnd(7, Attr);

        // odd rows forced a wrap
        if (pRow->sRowId % 2 != 0)
        {
            pRow->CharRow.SetWrapStatus(true);
        }
        else
        {
            pRow->CharRow.SetWrapStatus(false);
        }
    }

    void FillBisect(ROW *pRow)
    {
        const CONSOLE_INFORMATION* const gci = ServiceLocator::LocateGlobals()->getConsoleInformation();
        // length 80 string of text with bisecting characters at the beginning and end.
        // positions of き(\x304d) are at 0, 27-28, 39-40, 67-68, 79
        PWCHAR pwszText =
            L"\x304d"
            L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            L"\x304d\x304d"
            L"0123456789"
            L"\x304d\x304d"
            L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            L"\x304d\x304d"
            L"0123456789"
            L"\x304d";
        memcpy_s(pRow->CharRow.Chars, CommonState::s_csBufferWidth, pwszText, wcslen(pwszText));
        pRow->CharRow.Left = 0;
        pRow->CharRow.Right = 80; // 1 past the last valid character in the array

        // set double-byte/double-width attributes
        pRow->CharRow.KAttrs[0] = CHAR_ROW::ATTR_TRAILING_BYTE;
        pRow->CharRow.KAttrs[27] = CHAR_ROW::ATTR_LEADING_BYTE;
        pRow->CharRow.KAttrs[28] = CHAR_ROW::ATTR_TRAILING_BYTE;
        pRow->CharRow.KAttrs[39] = CHAR_ROW::ATTR_LEADING_BYTE;
        pRow->CharRow.KAttrs[40] = CHAR_ROW::ATTR_TRAILING_BYTE;
        pRow->CharRow.KAttrs[67] = CHAR_ROW::ATTR_LEADING_BYTE;
        pRow->CharRow.KAttrs[68] = CHAR_ROW::ATTR_TRAILING_BYTE;
        pRow->CharRow.KAttrs[79] = CHAR_ROW::ATTR_LEADING_BYTE;

        // everything gets default attributes
        pRow->AttrRow.Initialize(80, gci->CurrentScreenBuffer->GetAttributes());

        pRow->CharRow.SetWrapStatus(true);
    }
};